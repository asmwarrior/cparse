#ifndef PPEVALVISITOR_H
#define PPEVALVISITOR_H

#include "ppast.h"
#include "ppastvisitor.h"
#include "ppcontext.h"

namespace PP {
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
    void visitTokens(ASTNode *node);
    void visitToken(ASTNode *node);
    void visitConstantExpr(ASTNode *node);
};
}

#endif // PPEVALVISITOR_H
