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
    assert(decl->getLocation().isValid());
    addFunction(decl);
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
    const clang::NamedDecl* decl = expr->getDecl();
    if (const clang::FunctionDecl* func = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
      Location usage = expr->getLocation();
      assert(usage.isValid());
      addFunction(func, usage);
      if (decls_.find(decl) == decls_.end())
      {
        // FIXME: used but not defined or declared
        // __builtin_memcpy
        // compiler consider declared when first used.
      }
      else
      {
        assert(decls_[decl] == decl->getKind());
      }
    }
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

 public:
  void save(Sink* sink)
  {
    proto::CompilationUnit cu;
    cu.set_main_file(util_.filePathOrDie(sourceManager_.getMainFileID()));
    for (const auto& it : functions_)
    {
      assert(it.first == it.second.filename());
      std::string uri = "functions:" + it.first;
      sink->writeOrDie(uri, it.second.SerializeAsString());
      for (const auto& func : it.second.functions())
      {
        if (func.usage() == proto::kDefine)
        {
          // func.mutable_name_range().set_filename(it.first);
          assert(func.name_range().filename() == it.first);
          *cu.add_functions() = func;
        }
      }
    }
    printf("CompilationUnit %d bytes %d public functions\n", cu.ByteSize(), cu.functions_size());
    // printf("%s\n", cu.DebugString().c_str());
    std::string uri = "main:" + cu.main_file();
    sink->writeOrDie(uri, cu.SerializeAsString());
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

  // is usage is Invalid, it's a define or declare, otherwise a use.
  void addFunction(const clang::FunctionDecl* decl, Location usage = Location())
  {
    assert(decl->getDeclName());
    proto::Function func;
    func.set_name(decl->getName().str());
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
    Util::setStorageClass(decl->getStorageClass(), &func);

    if (usage.isValid())
    {
      func.set_usage(proto::kUse);
    }
    else
    {
      // a declaration or definition
      func.set_usage(decl->isThisDeclarationADefinition() ? proto::kDefine : proto::kDeclare);
      addDecl(decl);
      usage = decl->getLocation();
    }
    if (usage.isMacroID()) func.set_macro(true);
    // if it's a define, record it anyways
    // if it's a usage, record it anyways
    // discard a declaration with no location
    proto::Range range;
    if (util_.setNameRange(func.name(), usage, &range) || func.usage() != proto::kDeclare)
    {
      clang::SourceLocation fileLoc = sourceManager_.getFileLoc(usage);
      string file = util_.filePathOrDie(fileLoc);
      if (func.usage() == proto::kDefine)
        range.set_filename(file);
      if (functions_.find(file) == functions_.end())
        functions_[file].set_filename(file);

      *func.mutable_name_range() = range;
      *functions_[file].add_functions() = func;
    }
  }

  void addDecl(const clang::NamedDecl* decl)
  {
    assert(decls_.find(decl) == decls_.end());
    decls_[decl] = decl->getKind();
  }

  std::unique_ptr<clang::MangleContext> mangle_;
  clang::SourceManager& sourceManager_;
  const clang::LangOptions& langOpts_;
  const Util util_;
  std::unordered_map<const clang::NamedDecl*, clang::Decl::Kind> decls_;
  // map from filename to functions
  std::map<std::string, proto::Functions> functions_;
};

class IndexConsumer : public clang::ASTConsumer
{
 public:
  IndexConsumer(clang::CompilerInstance& compiler, Sink* sink)
    : preprocessor_(compiler.getPreprocessor()),
      sourceManager_(compiler.getSourceManager()),
      sink_(sink)
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
    visitor.save(sink_);
  }

 private:
  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
  Sink* sink_;
};

class IndexAction : public clang::PluginASTAction
{
 protected:
  clang::ASTConsumer *CreateASTConsumer(
      clang::CompilerInstance& compiler,
      clang::StringRef inputFile) override
  {
    LOG_INFO << "IndexAction ctor " << inputFile.str();
    sink_.reset(new Sink(getOutput(inputFile.str()).c_str()));
    auto* pp = new IndexPP(compiler, sink_.get());
    compiler.getPreprocessor().addPPCallbacks(pp);
    return new IndexConsumer(compiler, sink_.get());
    //auto* consumer = new PrintConsumer(CI.getPreprocessor(), CI.getSourceManager(), CI.getLangOpts());
    //pp->setRewriter(consumer->getRewriter());
    //return consumer;
  }

 public:

  IndexAction()
  {
    LOG_INFO << "IndexAction ctor";
  }

  ~IndexAction()
  {
    LOG_INFO << "IndexAction dtor";
  }

  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &arg) override
  {
    printf("ParseArgs %zd\n", arg.size());
    return true;
  }

 private:
  string getOutput(const string& input)
  {
     string out = input + ".cindex";
     std::transform(out.begin(), out.end(), out.begin(),[](char ch)
                    { return ch == '/' ? '_' : ch; });
     return "tmp/" + out;
  }

  std::unique_ptr<Sink> sink_;
};

}
