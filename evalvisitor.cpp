#include "evalvisitor.h"
#include "astvisitor.h"
#include <stdio.h>

EvalVisitor::EvalVisitor(Context *ctx)
    : ASTVisitor(ctx)
{
}

void EvalVisitor::visitGroup(ASTNode *node)
{
    ASTGroup *group = static_cast<ASTGroup *>(node);
    for (ASTNodeList::iterator iter = group->begin();
         iter != group->end(); iter++) {

        (*iter)->accept(this);
    }
}

void EvalVisitor::visitTextLine(ASTNode *node)
{
    ASTTextLine *textline = static_cast<ASTTextLine *>(node);
    for (ASTNodeList::iterator iter = textline->begin();
         iter != textline->end(); iter++) {

        (*iter)->accept(this);
    }
}

void EvalVisitor::visitNonDirective(ASTNode *node)
{
    printf("# ");
    ASTNonDirective *nond = static_cast<ASTNonDirective *>(node);
    for (ASTNodeList::iterator iter = nond->begin();
         iter != nond->end(); iter++) {

        (*iter)->accept(this);
    }
}

void EvalVisitor::visitPragma(ASTNode *node)
{
    printf("#pragma ");
    ASTPragma *pragma = static_cast<ASTPragma *>(node);
    for (ASTNodeList::iterator iter = pragma->begin();
         iter != pragma->end(); iter++) {

        (*iter)->accept(this);
    }
    printf("\n");
}

void EvalVisitor::visitIfGroup(ASTNode *node)
{
    printf("#if ");
    ASTIfGroup *ifGroup = static_cast<ASTIfGroup *>(node);
    ifGroup->expr()->accept(this);
    printf("\n");
    if (ifGroup->trueBranch())
        ifGroup->trueBranch()->accept(this);
    printf("#else\n");
    if (ifGroup->falseBranch())
        ifGroup->falseBranch()->accept(this);
    printf("#endif\n");
}

void EvalVisitor::visitDefine(ASTNode *node)
{
    ASTDefine *def = static_cast<ASTDefine *>(node);
    printf("#define ");
    def->id()->accept(this);
    if (def->args()) {
        printf("( ");
        foreach (ASTNode *arg, def->args()->nodeList()) {
            arg->accept(this);
            printf(", ");
        }
        if (def->isVarArgs())
            printf("... ");
        printf(") ");
    }
    if (def->body())
        def->body()->accept(this);
    printf("\n");
}

void EvalVisitor::visitPPTokens(ASTNode *node)
{
    ASTPPTokens *tokens = static_cast<ASTPPTokens *>(node);
    for (ASTNodeList::iterator iter = tokens->begin();
         iter != tokens->end(); iter++) {

        (*iter)->accept(this);
    }
}

void EvalVisitor::visitPPToken(ASTNode *node)
{
    printf("%s ", node->spellName().toAscii().constData());
}

void EvalVisitor::visitConstantExpr(ASTNode *node)
{
    ASTConstantExpr *expr = static_cast<ASTConstantExpr *>(node);
    foreach (ASTNode *subexpr, expr->nodeList()) {
        subexpr->accept(this);
    }
}
