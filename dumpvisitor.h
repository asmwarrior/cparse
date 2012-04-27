#ifndef DUMPVISITOR_H
#define DUMPVISITOR_H

#include "astvisitor.h"
#include "ast.h"

class DumpVisitor : public ASTVisitor
{
    Q_OBJECT
public:
    DumpVisitor(Context *ctx);
public slots:
    void visitGroup(ASTNode *node);
    void visitTextLine(ASTNode *node);
    void visitTextGroup(ASTNode *node);
    void visitNonDirective(ASTNode *node);
    void visitInclude(ASTNode *node);
    void visitPragma(ASTNode *node);
    void visitIfGroup(ASTNode *node);
    void visitDefine(ASTNode *node);
    void visitPPTokens(ASTNode *node);
    void visitPPToken(ASTNode *node);
    void visitConstantExpr(ASTNode *node);
};

#endif // DUMPVISITOR_H
