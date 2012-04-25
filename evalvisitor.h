#ifndef EVALVISITOR_H
#define EVALVISITOR_H

#include "ast.h"
#include "astvisitor.h"
#include "context.h"

class EvalVisitor : public ASTVisitor
{
    Q_OBJECT
public:
    EvalVisitor(Context *ctx);
public slots:
    void visitGroup(ASTNode *node);
    void visitTextLine(ASTNode *node);
    void visitNonDirective(ASTNode *node);
    void visitPragma(ASTNode *node);
    void visitIfGroup(ASTNode *node);
    void visitDefine(ASTNode *node);
    void visitPPTokens(ASTNode *node);
    void visitPPToken(ASTNode *node);
    void visitConstantExpr(ASTNode *node);
};

#endif // EVALVISITOR_H
