#include "llvm/Support/Casting.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "leveldb/db.h"

#include "muduo/base/Logging.h"

#include <stdio.h>

#define override

using namespace clang;

const char* kBuiltInFileName = "<built-in>";

const char* reasonString(clang::PPCallbacks::FileChangeReason reason)
{
  switch (reason)
  {
    case clang::PPCallbacks::EnterFile:
      return "Enter";
    case clang::PPCallbacks::ExitFile:
      return "Exit";
    case clang::PPCallbacks::SystemHeaderPragma:
      return "SystemHeaderPragma";
    case clang::PPCallbacks::RenameFile:
      return "Rename";
    default:
      return "Unknown";
  }
}

std::string filePath(const clang::SourceManager& sourceManager, clang::FileID fileId)
{
  if (fileId.isInvalid())
    return "<invalid location>";

  if (const clang::FileEntry* fileEntry = sourceManager.getFileEntryForID(fileId))
  {
    return fileEntry->getName();
  }

  return kBuiltInFileName;
}

std::string filePath(const clang::SourceManager& sourceManager, clang::SourceLocation location)
{
  if (location.isInvalid())
    return "<invalid location>";

  if (location.isFileID())
    return filePath(sourceManager, sourceManager.getFileID(location));

  return "<unknown>";
}

class IndexPP : public clang::PPCallbacks
{
 public:
  IndexPP(clang::CompilerInstance& compiler, leveldb::DB* db)
    : preprocessor_(compiler.getPreprocessor()),
      sourceManager_(compiler.getSourceManager()),
      db_(db)
  {
    // printf("predefines:\n%s\n", preprocessor_.getPredefines().c_str());
    LOG_DEBUG;
  }

  ~IndexPP()
  {
    LOG_DEBUG;
  }

  void FileChanged(clang::SourceLocation location,
                   PPCallbacks::FileChangeReason reason,
                   clang::SrcMgr::CharacteristicKind fileType,
                   clang::FileID previousFileID) override
  {
    LOG_TRACE << reasonString(reason) << " " << filePath(sourceManager_, location);

    const std::string file_changed = filePath(sourceManager_, location);
    if (reason == clang::PPCallbacks::EnterFile)
    {
      std::string content;
      if (getFileContent(location, &content))
      {
        auto it = files_.find(file_changed);
        if (it == files_.end())
        {
          files_[file_changed] = content;
        }
        else if (it->second != content)
        {
          LOG_WARN << "File content changed " << file_changed;
        }
      }
      else
      {
        LOG_ERROR << "unable to get file content for " << file_changed;
      }
    }
  }

  virtual void FileSkipped(const FileEntry &ParentFile,
                           const Token &FilenameTok,
                           SrcMgr::CharacteristicKind FileType) {
    // printf("FileSkipped %s\n", preprocessor_.getSpelling(FilenameTok).c_str());
  }

  virtual void InclusionDirective(SourceLocation hashLoc,
                                  const Token &includeTok,
                                  StringRef filename,
                                  bool isAngled,
                                  CharSourceRange filenameRange,
                                  const FileEntry *file,
                                  StringRef searchPath,
                                  StringRef relativePath,
                                  const Module *imported)
  {
    bool invalid = true;
    unsigned lineno = sourceManager_.getSpellingLineNumber(hashLoc, &invalid);
    if (invalid)
      return;
    std::string currentFile = filePath(sourceManager_, hashLoc);
    std::string includedFile = file->getName();
    LOG_DEBUG << currentFile << ":" << lineno << " -> " << includedFile;
    LOG_INFO << filename.str();
    if (filenameRange.getBegin().isMacroID())
    {
      LOG_WARN << "#include " << filename.str() << " was a macro";
      return;
    }

    // skip '"' or '<'
    SourceLocation filenameStart = filenameRange.getBegin().getLocWithOffset(1);
    SourceLocation filenameEnd = filenameRange.getBegin().getLocWithOffset(-1);
    invalid = true;

    //const char* filename
  }

  virtual void EndOfMainFile()
  {
    LOG_DEBUG;
    if (db_ == nullptr)
      return;
    for (const auto& it : files_)
    {
      // printf("%s\n", it.first.c_str());
      std::string uri = "src:";
      if (it.first[0] != '/')
      {
        uri += '/';
      }
      uri += it.first;
      std::string content;
      leveldb::Status s = db_->Get(leveldb::ReadOptions(), uri, &content);
      if (s.ok())
      {
        if (content != it.second)
        {
          LOG_WARN << "Different content for " << uri;
          // printf("<<<<<\n%s\n=====\n%s\n>>>>>\n", content.c_str(), it.second.c_str());
        }
      }
      // else
      {
        s = db_->Put(leveldb::WriteOptions(), uri, it.second);
        assert(s.ok());
      }
    }
    db_ = nullptr;
  }

  /// \brief Called by Preprocessor::HandleMacroExpandedIdentifier when a
  /// macro invocation is found.
  virtual void MacroExpands(const Token &MacroNameTok, const MacroDirective *MD,
                            SourceRange Range, const MacroArgs *Args) {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  /// \brief Hook called whenever a macro definition is seen.
  virtual void MacroDefined(const Token &MacroNameTok,
                            const MacroDirective *MD) {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
    auto start = MacroNameTok.getLocation();
    clang::SourceRange range(start, start.getLocWithOffset(MacroNameTok.getLength()));
    if (range.getBegin().isMacroID())
    {
      printf("don't know\n");
    }
    else
    {
      //const auto first = sourceManager_.getDecomposedSpellingLoc(start);
      //const auto last = sourceManager_.getDecomposedSpellingLoc(range.getEnd());
      //printf("%u %u\n", first, last);

    }
  }

  /// \brief Hook called whenever a macro \#undef is seen.
  ///
  /// MD is released immediately following this callback.
  virtual void MacroUndefined(const Token &MacroNameTok,
                              const MacroDirective *MD) {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  /// \brief Hook called whenever the 'defined' operator is seen.
  /// \param MD The MacroDirective if the name was a macro, null otherwise.
  virtual void Defined(const Token &MacroNameTok, const MacroDirective *MD,
                       SourceRange Range) {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());

  }

  // Called for #ifdef, checks for use of an existing macro definition.
  // Implements PPCallbacks::Ifdef.
  void Ifdef(clang::SourceLocation location,
             const clang::Token& MacroNameTok,
             const clang::MacroDirective* macro) override {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  // Called for #ifndef, checks for use of an existing macro definition.
  // Implements PPCallbacks::Ifndef.
  void Ifndef(clang::SourceLocation location,
              const clang::Token& MacroNameTok,
              const clang::MacroDirective* macro) override {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }


  /// \brief Hook called when a source range is skipped.
  /// \param Range The SourceRange that was skipped. The range begins at the
  /// \#if/\#else directive and ends after the \#endif/\#else directive.
  virtual void SourceRangeSkipped(SourceRange Range) {
    // printf("%s\n", __FUNCTION__);
  }

 private:
  bool getFileContent(clang::SourceLocation location, std::string* content)
  {
    assert(location.isFileID());
    clang::FileID fileId = sourceManager_.getFileID(location);
    if (const clang::FileEntry* fileEntry = sourceManager_.getFileEntryForID(fileId))
    {
      bool isInvalid = false;
      const llvm::MemoryBuffer* buffer = sourceManager_.getMemoryBufferForFile(fileEntry, &isInvalid);
      if (buffer && ! isInvalid)
      {
        *content = buffer->getBuffer().str();
        return true;
      }
    }
    else if (fileId == preprocessor_.getPredefinesFileID())
    {
      *content = preprocessor_.getPredefines();
      return true;
    }
    return false;
  }

  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
  std::map<std::string, std::string> files_;
  leveldb::DB* db_;
};

class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
  typedef clang::RecursiveASTVisitor<Visitor> base;
public:
  // bool shouldVisitImplicitCode() const { return true; }

  bool VisitDecl(const clang::Decl* decl)
  {
    const clang::NamedDecl* nd = llvm::dyn_cast<clang::NamedDecl>(decl);
    printf("decl %p %s %s\n", decl, decl->getDeclKindName(),
           nd ? nd->getNameAsString().c_str() : "");
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *expr)
  {
    const clang::FunctionDecl* func = expr->getDirectCallee();
    if (!func)
      return true;
    printf("CallExpr %p %p", expr->getCalleeDecl(), func);
    if (expr->getCalleeDecl() != func)
      printf(" =====******");
    printf(" %s\n", func ? func->getNameAsString().c_str() : NULL);
    return true;
  }

  bool VisitStmt(const clang::Stmt* stmt)
  {
    //printf("stmt \n");
    return true;
  }
  bool VisitExpr(const clang::Expr* expr)
  {
    //printf("expr \n");
    return true;
  }

  bool VisitType(const clang::Type* type)
  {
    //printf("type \n");
    return true;
  }
  bool VisitTypeLoc(clang::TypeLoc typeloc)
  {
    //printf("typeloc \n");
    return true;
  }
};

class IndexConsumer : public clang::ASTConsumer
{
 public:
  IndexConsumer()
  {
    LOG_DEBUG;
  }

  ~IndexConsumer()
  {
    LOG_DEBUG;
  }

  void HandleTranslationUnit(clang::ASTContext& context) override
  {
    LOG_DEBUG;
    //Visitor visitor;
    //visitor.TraverseDecl(context.getTranslationUnitDecl());
    printf("HandleTranslationUnit done\n");
  }

 private:
};

class IndexAction : public clang::PluginASTAction
{
 protected:
  virtual clang::ASTConsumer *CreateASTConsumer(
      clang::CompilerInstance& compiler,
      clang::StringRef InFile) override
  {
    printf("CreateAST %s\n", InFile.str().c_str());
    auto* pp = new IndexPP(compiler, db_.get());
    compiler.getPreprocessor().addPPCallbacks(pp);
    return new IndexConsumer();
    //auto* consumer = new PrintConsumer(CI.getPreprocessor(), CI.getSourceManager(), CI.getLangOpts());
    //pp->setRewriter(consumer->getRewriter());
    //return consumer;
  }

 public:

  IndexAction(bool save = false)
  {
    printf("%s\n", __FUNCTION__);

    if (save)
    {
      leveldb::DB* db;
      leveldb::Options options;
      options.create_if_missing = true;
      leveldb::Status status = leveldb::DB::Open(options, "testdb", &db);
      assert(status.ok());
      db_.reset(db);
    }
  }

  ~IndexAction()
  {
    LOG_DEBUG;
  }

  virtual bool ParseArgs(const clang::CompilerInstance &CI,
                         const std::vector<std::string> &arg)
  {
    printf("ParseArgs %zd\n", arg.size());
    return true;
  }

 private:
  std::unique_ptr<leveldb::DB> db_;
};
