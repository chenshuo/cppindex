#include "build/record.pb.h"

#include "llvm/Support/Casting.h"
#include "llvm/Support/MD5.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "leveldb/db.h"

#include "muduo/base/Logging.h"

#include <unordered_map>

#include <boost/noncopyable.hpp>

#include <stdio.h>

namespace indexer
{
using std::string;
#include "sink.h"
#include "util.h"
#include "preprocess.h"

class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
  typedef clang::RecursiveASTVisitor<Visitor> base;
  typedef clang::SourceLocation Location;
public:
  explicit Visitor(clang::ASTContext& context)
    : mangle_(context.createMangleContext()),
      sourceManager_(context.getSourceManager()),
      langOpts_(context.getLangOpts()),
      util_(sourceManager_, langOpts_)
  {
  }

  // bool shouldVisitImplicitCode() const { return true; }

  bool VisitDecl(const clang::Decl* decl)
  {
    // const clang::NamedDecl* nd = llvm::dyn_cast<clang::NamedDecl>(decl);
    // printf("decl %p %s %s\n", decl, decl->getDeclKindName(),
    //        nd ? nd->getNameAsString().c_str() : "");
    return true;
  }

  bool VisitFunctionDecl(const clang::FunctionDecl* decl)
  {
    proto::Function func;
    assert(decl->getDeclName());
    func.set_name(decl->getNameAsString());
    {
    string mangled = getMangledName(decl);
    if (!mangled.empty())
      func.set_mangled(mangled);
    }
    {
      clang::QualType type = decl->getType();
      clang::SplitQualType split = type.split();
      func.set_signature(clang::QualType::getAsString(split));
    }
    func.set_define(decl->isThisDeclarationADefinition());
    Util::setStorageClass(decl->getStorageClass(), &func);

    Location start = decl->getLocation();
    if (start.isMacroID()) func.set_macro(true);
    util_.setNameRange(func.name(), start, func.mutable_name_range());
    // FIXME:
    // BUILD_BUG_ON

    // clang::DeclarationNameInfo nameInfo = decl->getNameInfo();
    printf("%s\n", func.DebugString().c_str());
    fflush(stdout);

    return true;
  }

  bool VisitCallExprNOTUSED(clang::CallExpr *expr)
  {
    const clang::FunctionDecl* func = expr->getDirectCallee();
    if (!func)
      return true;
    printf("CallExpr %p callee %p func %p", expr, expr->getCalleeDecl(), func);
    if (expr->getCalleeDecl() != func)
      printf(" =====******");
    printf(" %s %s\n",
           func ? func->getNameAsString().c_str() : NULL,
           getMangledName(func).c_str());
    return true;
  }

  bool VisitStmt(const clang::Stmt* stmt)
  {
    //printf("stmt \n");
    return true;
  }

  bool VisitExpr(const clang::Expr* expr)
  {
    // printf("expr %p\n", expr);
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *expr)
  {
    printf("DeclRefExpr %p: ", expr);
    fflush(stdout);
    expr->getLocation().dump(sourceManager_);
    puts("");
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

 private:
  std::string getMangledName(const clang::FunctionDecl* decl)
  {
    llvm::SmallString<512> buffer;
    llvm::raw_svector_ostream mangled_name(buffer);
    if (mangle_->shouldMangleDeclName(decl))
    {
      mangle_->mangleName(decl, mangled_name);
      return mangled_name.str().str();
    }
    return "";
  }

  std::unique_ptr<clang::MangleContext> mangle_;
  clang::SourceManager& sourceManager_;
  const clang::LangOptions& langOpts_;
  const Util util_;
};

class IndexConsumer : public clang::ASTConsumer
{
 public:
  IndexConsumer(clang::CompilerInstance& compiler, leveldb::DB* db)
    : preprocessor_(compiler.getPreprocessor()),
      sourceManager_(compiler.getSourceManager())
  {
    LOG_DEBUG;
  }

  ~IndexConsumer()
  {
    LOG_DEBUG;
  }

  void HandleTranslationUnit(clang::ASTContext& context) override
  {
    LOG_INFO << "HandleTranslationUnit";
    assert(&sourceManager_ == &context.getSourceManager());
    Visitor visitor(context);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
    LOG_INFO << "HandleTranslationUnit done";
  }

 private:
  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
};

class IndexAction : public clang::PluginASTAction
{
 protected:
  clang::ASTConsumer *CreateASTConsumer(
      clang::CompilerInstance& compiler,
      clang::StringRef inputFile) override
  {
    LOG_INFO << "IndexAction ctor " << inputFile.str();
    auto* pp = new IndexPP(compiler, sink_);
    compiler.getPreprocessor().addPPCallbacks(pp);
    return new IndexConsumer(compiler, nullptr);
    //auto* consumer = new PrintConsumer(CI.getPreprocessor(), CI.getSourceManager(), CI.getLangOpts());
    //pp->setRewriter(consumer->getRewriter());
    //return consumer;
  }

 public:

  IndexAction(Sink* sink, bool save = false)
    : sink_(sink)
  {
    printf("%s\n", __FUNCTION__);

    if (save)
    {
      leveldb::DB* db;
      leveldb::Options options;
      options.create_if_missing = true;
      leveldb::Status status = leveldb::DB::Open(options, "testdb", &db);
      if (status.ok())
      {
        db_.reset(db);
      }
      else
      {
        LOG_ERROR << "Unable to open leveldb";
      }
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
  Sink* sink_;
  std::unique_ptr<leveldb::DB> db_;
};

}
