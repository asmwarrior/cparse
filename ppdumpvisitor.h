#ifndef PPDUMPVISITOR_H
#define PPDUMPVISITOR_H

#include "ppastvisitor.h"
#include "ppast.h"

namespace PP {
class DumpVisitor : public ASTVisitor
{
    Q_OBJECT
public:
    DumpVisitor(Context *ctx);
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
}

#endif // PPDUMPVISITOR_H
