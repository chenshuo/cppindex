#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <stdio.h>

#define override

class Visitor : public clang::RecursiveASTVisitor<Visitor> {
  typedef clang::RecursiveASTVisitor<Visitor> base;
public:
  bool VisitDecl(const clang::Decl* decl)
  {
    printf("%s\n", decl->getDeclKindName());
    return true;
  }
};

class IndexConsumer : public clang::ASTConsumer {
public:
  void HandleTranslationUnit(clang::ASTContext& context) override
  {
    printf("HandleTranslationUnit\n");
    Visitor visitor;
    visitor.TraverseDecl(context.getTranslationUnitDecl());
  }
};
