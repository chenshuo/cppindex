#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include <stdio.h>

class IndexConsumer : public clang::ASTConsumer {
 public:
};

class IndexAction : public clang::PluginASTAction {
 protected:
  virtual clang::ASTConsumer *CreateASTConsumer(clang::CompilerInstance &CI,
                                         clang::StringRef InFile)
  {
    printf("CreateAST\n");
    return new IndexConsumer();
  }

 public:
  virtual bool ParseArgs(const clang::CompilerInstance &CI,
                         const std::vector<std::string> &arg)
  {
    printf("ParseArgs %zd\n", arg.size());
    return true;
  }
};

static clang::FrontendPluginRegistry::Add<IndexAction>
X("index", "index");

