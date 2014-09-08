#include "llvm/Support/Casting.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <stdio.h>

#define override

using namespace clang;

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

  return "<built-in>";
}

std::string filePath(const clang::SourceManager& sourceManager, clang::SourceLocation location)
{
  if (location.isInvalid())
    return "<invalid location>";

  if (location.isFileID())
    return filePath(sourceManager, sourceManager.getFileID(location));

  return "<unknown>";
}

class PPConsumer : public clang::PPCallbacks {
 public:
  PPConsumer(const clang::Preprocessor& preprocessor,
             clang::SourceManager& sourceManager)
    : preprocessor_(preprocessor),
      sourceManager_(sourceManager)
  {
    printf("predefines:\n%s\n", preprocessor.getPredefines().c_str());
  }

  void FileChanged(clang::SourceLocation location,
                   PPCallbacks::FileChangeReason reason,
                   clang::SrcMgr::CharacteristicKind file_type,
                   clang::FileID previousFileID) override
  {
    const std::string file_changed = filePath(sourceManager_, location);
    printf("FileChanged %s reason %s ", filePath(sourceManager_, location).c_str(), reasonString(reason));
    if (reason == clang::PPCallbacks::EnterFile)
    {
      if (files_.find(file_changed) == files_.end())
      {
        std::string content;
        if (getFileContent(location, &content))
        {
          files_[file_changed] = content;
          printf("%zd\n", content.size());
        }
        else
        {
          printf("unable to get file content\n");
        }
      }
      else
      {
        printf("seen!!!\n");
      }
    }
    else if (reason == clang::PPCallbacks::ExitFile)
    {
      printf(" from: %s\n", filePath(sourceManager_, previousFileID).c_str());
    }
    else
      printf("\n");
  }

  virtual void FileSkipped(const FileEntry &ParentFile,
                           const Token &FilenameTok,
                           SrcMgr::CharacteristicKind FileType) {
    printf("FileSkipped %s\n", preprocessor_.getSpelling(FilenameTok).c_str());
  }

  virtual void InclusionDirective(SourceLocation HashLoc,
                                  const Token &IncludeTok,
                                  StringRef FileName,
                                  bool IsAngled,
                                  CharSourceRange FilenameRange,
                                  const FileEntry *File,
                                  StringRef SearchPath,
                                  StringRef RelativePath,
                                  const Module *Imported) {
    printf("InclusionDirective\n");
  }

  virtual void EndOfMainFile() {
    printf("EndOfMainFile\n");
  }

  /// \brief Called by Preprocessor::HandleMacroExpandedIdentifier when a
  /// macro invocation is found.
  virtual void MacroExpands(const Token &MacroNameTok, const MacroDirective *MD,
                            SourceRange Range, const MacroArgs *Args) {
    printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  /// \brief Hook called whenever a macro definition is seen.
  virtual void MacroDefined(const Token &MacroNameTok,
                            const MacroDirective *MD) {
    printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
    auto start = MacroNameTok.getLocation();
    clang::SourceRange range(start, start.getLocWithOffset(MacroNameTok.getLength()));
    if (range.getBegin().isMacroID())
    {
      printf("don't know\n");
    }
    else
    {
      const auto first = sourceManager_.getDecomposedSpellingLoc(start).second;
      const auto last = sourceManager_.getDecomposedSpellingLoc(range.getEnd()).second;
      printf("%u %u\n", first, last);
    }
  }

  /// \brief Hook called whenever a macro \#undef is seen.
  ///
  /// MD is released immediately following this callback.
  virtual void MacroUndefined(const Token &MacroNameTok,
                              const MacroDirective *MD) {
    printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  /// \brief Hook called whenever the 'defined' operator is seen.
  /// \param MD The MacroDirective if the name was a macro, null otherwise.
  virtual void Defined(const Token &MacroNameTok, const MacroDirective *MD,
                       SourceRange Range) {
    printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());

  }

  // Called for #ifdef, checks for use of an existing macro definition.
  // Implements PPCallbacks::Ifdef.
  void Ifdef(clang::SourceLocation location,
             const clang::Token& MacroNameTok,
             const clang::MacroDirective* macro) override {
    printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  // Called for #ifndef, checks for use of an existing macro definition.
  // Implements PPCallbacks::Ifndef.
  void Ifndef(clang::SourceLocation location,
              const clang::Token& MacroNameTok,
              const clang::MacroDirective* macro) override {
    printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }


  /// \brief Hook called when a source range is skipped.
  /// \param Range The SourceRange that was skipped. The range begins at the
  /// \#if/\#else directive and ends after the \#endif/\#else directive.
  virtual void SourceRangeSkipped(SourceRange Range) {
    printf("%s\n", __FUNCTION__);
  }

 private:
  bool getFileContent(clang::SourceLocation location, std::string* content)
  {
    if (const clang::FileEntry* fileEntry = sourceManager_.getFileEntryForID(sourceManager_.getFileID(location)))
    {
      bool isInvalid = false;
      const llvm::MemoryBuffer* buffer = sourceManager_.getMemoryBufferForFile(fileEntry, &isInvalid);
      if (buffer && ! isInvalid)
      {
        *content = buffer->getBuffer().str();
        return true;
      }
    }
    return false;
  }

  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
  std::map<std::string, std::string> files_;
};

class Visitor : public clang::RecursiveASTVisitor<Visitor> {
  typedef clang::RecursiveASTVisitor<Visitor> base;
public:
  // bool shouldVisitImplicitCode() const { return true; }

  bool VisitDecl(const clang::Decl* decl)
  {
    const clang::NamedDecl* nd = llvm::dyn_cast<clang::NamedDecl>(decl);
    printf("decl %s %s\n", decl->getDeclKindName(),
           nd ? nd->getNameAsString().c_str() : "");
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *expr)
  {
    printf("CallExpr %p\n", expr->getCalleeDecl());
    return true;
  }

  bool VisitStmt(const clang::Stmt* stmt)
  {
    printf("stmt \n");
    return true;
  }
  bool VisitExpr(const clang::Expr* expr)
  {
    printf("expr \n");
    return true;
  }

  bool VisitType(const clang::Type* type)
  {
    printf("type \n");
    return true;
  }
  bool VisitTypeLoc(clang::TypeLoc typeloc)
  {
    printf("typeloc \n");
    return true;
  }
};

class IndexConsumer : public clang::ASTConsumer {
public:
  void HandleTranslationUnit(clang::ASTContext& context) override
  {
    printf("HandleTranslationUnit\n");
    Visitor visitor;
    //visitor.TraverseDecl(context.getTranslationUnitDecl());
    printf("HandleTranslationUnit done\n");
  }
};

class PrintPP : public clang::PPCallbacks {
 public:
  PrintPP(const clang::Preprocessor& preprocessor,
          clang::SourceManager& sourceManager)
    : preprocessor_(preprocessor),
      sourceManager_(sourceManager),
      rewriter_(nullptr)
  {
    //printf("predefines:\n%s\n", preprocessor.getPredefines().c_str());
  }

  void setRewriter(clang::Rewriter* r) { rewriter_ = r; }

  /// \brief Called by Preprocessor::HandleMacroExpandedIdentifier when a
  /// macro invocation is found.
  virtual void MacroExpands(const Token &MacroNameTok, const MacroDirective *MD,
                            SourceRange Range, const MacroArgs *Args) {
    auto start = MacroNameTok.getLocation();
    if (start.isMacroID() || sourceManager_.getFileID(start) != sourceManager_.getMainFileID())
      return;
    printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
    clang::SourceRange range(start, start.getLocWithOffset(MacroNameTok.getLength()));
    rewriter_->InsertTextBefore(start, "<span class=\"macro\">");
    rewriter_->InsertTextAfter(range.getEnd(), "</span>");
  }

  /// \brief Hook called whenever a macro definition is seen.
  virtual void MacroDefined(const Token &MacroNameTok,
                            const MacroDirective *MD) {
    auto start = MacroNameTok.getLocation();
    if (start.isMacroID() || sourceManager_.getFileID(start) != sourceManager_.getMainFileID())
      return;
    printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
    clang::SourceRange range(start, start.getLocWithOffset(MacroNameTok.getLength()));
    if (range.getBegin().isMacroID())
    {
      printf("don't know\n");
    }
    else
    {
      const auto first = sourceManager_.getDecomposedSpellingLoc(MD->getLocation()).second;
      const auto last = sourceManager_.getDecomposedSpellingLoc(range.getEnd()).second;
      //printf("%u %u\n", first, last);
      rewriter_->InsertTextBefore(start, "<span class=\"macro\">");
      rewriter_->InsertTextAfter(range.getEnd(), "</span>");
    }
  }

 private:
  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
  std::map<std::string, std::string> files_;
  clang::Rewriter* rewriter_;
};

class PrintConsumer : public clang::ASTConsumer {
 public:
  explicit PrintConsumer(clang::Preprocessor& pp,
                         clang::SourceManager& smgr,
                         const clang::LangOptions& opts)
    : preprocessor_(pp),
      sourceManager_(smgr),
      // langOptions_(opts),
      rewriter_(smgr, opts)
  { }

  clang::Rewriter* getRewriter() { return &rewriter_; }

  void HandleTranslationUnit(clang::ASTContext& context) override
  {
    printf("PrintConsumer::HandleTranslationUnit\n");
    FileID fid = context.getSourceManager().getMainFileID();
    clang::RewriteBuffer& rewriteBuffer = rewriter_.getEditBuffer(fid);

    escapeHtml(fid);
    //const llvm::MemoryBuffer *FromFile = sourceManager_.getBuffer(fid);
    //clang::Lexer L(fid, FromFile, sourceManager_, langOptions_);
    //L.SetCommentRetentionState(true);

    clang::Preprocessor& PP = preprocessor_;
    PP.EnterMainSourceFile();
    clang::Token Tok;

    PP.SetMacroExpansionOnlyInDirectives();
    do {
      PP.Lex(Tok);
      unsigned TokOffs = sourceManager_.getFileOffset(Tok.getLocation());
      unsigned TokLen = Tok.getLength();
      printf("off=%d len=%d kind=%s\n", TokOffs, TokLen, tok::getTokenName(Tok.getKind()));
    } while (Tok.isNot(tok::eof));

    printf("PrintConsumer::HandleTranslationUnit done\n%s", rewriteBuffer.ToString().c_str());
  }

private:
  void escapeHtml(FileID fid)
  {
    const llvm::MemoryBuffer* buf = sourceManager_.getBuffer(fid);
    const char* curr = buf->getBufferStart();
    const char* FileEnd = buf->getBufferEnd();

    assert (curr <= FileEnd);

    RewriteBuffer& rb = rewriter_.getEditBuffer(fid);

    for (unsigned FilePos = 0; curr != FileEnd ; ++curr, ++FilePos) {
      switch (*curr) {
        case '<':
          rb.ReplaceText(FilePos, 1, "&lt;");
          break;

        case '>':
          rb.ReplaceText(FilePos, 1, "&gt;");
          break;

        case '&':
          rb.ReplaceText(FilePos, 1, "&amp;");
          break;
      }
    }
  }

  clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
  // const clang::LangOptions& langOptions_;
  clang::Rewriter rewriter_;
};

class IndexAction : public clang::PluginASTAction {
 protected:
  virtual clang::ASTConsumer *CreateASTConsumer(
      clang::CompilerInstance &CI,
      clang::StringRef InFile) override
  {
    printf("CreateAST\n");
    auto* pp = new PrintPP(CI.getPreprocessor(), CI.getSourceManager());
    CI.getPreprocessor().addPPCallbacks(pp);
    //return new IndexConsumer();
    auto* consumer = new PrintConsumer(CI.getPreprocessor(), CI.getSourceManager(), CI.getLangOpts());
    pp->setRewriter(consumer->getRewriter());
    return consumer;
  }

 public:
  virtual bool ParseArgs(const clang::CompilerInstance &CI,
                         const std::vector<std::string> &arg)
  {
    printf("ParseArgs %zd\n", arg.size());
    return true;
  }
};
