#ifndef EVALVISITOR_H
#define EVALVISITOR_H

#include "ast.h"
#include "astvisitor.h"
#include "context.h"
#include <QList>

class EvalVisitor : public ASTVisitor
{
    Q_OBJECT
public:
    typedef QList<ASTPPToken*> PPTokenList;
    typedef PPTokenList MacroArg;
    typedef QList<MacroArg> MacroArgList;
    EvalVisitor(Context *ctx);
    ~EvalVisitor();
public slots:
    void visitGroup(ASTNode *node);
    void visitTextGroup(ASTNode *node);
    void visitNonDirective(ASTNode *node);
    void visitPragma(ASTNode *node);
    void visitIfGroup(ASTNode *node);
    void visitDefine(ASTNode *node);
    void visitUndef(ASTNode *node);
    PPTokenList ppTokens() const;
private:
    class Private;
    Private *d;
};

#endif // EVALVISITOR_H
