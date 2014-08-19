#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <stdio.h>

#define override

class Visitor : public clang::RecursiveASTVisitor<Visitor> {
  typedef clang::RecursiveASTVisitor<Visitor> base;
public:
  // bool shouldVisitImplicitCode() const { return true; }

  bool VisitDecl(const clang::Decl* decl)
  {
    printf("decl %s\n", decl->getDeclKindName());
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *expr)
  {
    printf("CallExpr %p\n", expr->getCalleeDecl());
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
