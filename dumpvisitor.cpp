#include "dumpvisitor.h"
#include "astvisitor.h"
#include "ast.h"
#include "context.h"
#include <QDebug>

DumpVisitor::DumpVisitor(Context *ctx)
    : ASTVisitor(ctx)
{
}

void DumpVisitor::visitGroup(ASTNode *node)
{
    ASTGroup *group = static_cast<ASTGroup *>(node);
    for (ASTNodeList::iterator iter = group->begin();
         iter != group->end(); iter++) {

        (*iter)->accept(this);
    }
}

void DumpVisitor::visitTextLine(ASTNode *node)
{
    ASTTextLine *textline = static_cast<ASTTextLine *>(node);
    for (ASTNodeList::iterator iter = textline->begin();
         iter != textline->end(); iter++) {

        (*iter)->accept(this);
    }
    printf("\n");
}

void DumpVisitor::visitTextGroup(ASTNode *node)
{
    ASTTextGroup *tg = static_cast<ASTTextGroup *>(node);
    for (ASTNodeList::iterator iter = tg->begin();
         iter != tg->end(); iter++) {

        (*iter)->accept(this);
    }
}

void DumpVisitor::visitNonDirective(ASTNode *node)
{
    printf("# ");
    ASTNonDirective *nond = static_cast<ASTNonDirective *>(node);
    for (ASTNodeList::iterator iter = nond->begin();
         iter != nond->end(); iter++) {

        (*iter)->accept(this);
    }
    printf("\n");
}

void DumpVisitor::visitInclude(ASTNode *node)
{
    printf("#include ");
    ASTInclude *inc = static_cast<ASTInclude*>(node);
    for (ASTNodeList::iterator iter = inc->begin();
         iter != inc->end(); iter++) {

        (*iter)->accept(this);
    }
    printf("\n");
}

void DumpVisitor::visitPragma(ASTNode *node)
{
    printf("#pragma ");
    ASTPragma *pragma = static_cast<ASTPragma *>(node);
    for (ASTNodeList::iterator iter = pragma->begin();
         iter != pragma->end(); iter++) {

        (*iter)->accept(this);
    }
    printf("\n");
}

void DumpVisitor::visitIfGroup(ASTNode *node)
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

void DumpVisitor::visitDefine(ASTNode *node)
{
    ASTDefine *def = static_cast<ASTDefine *>(node);
    printf("#define ");
    def->id()->accept(this);
    if (def->args()) {
        printf("( ");
        foreach (ASTNode *arg, def->args()->nodeList()) {
            arg->accept(this);
            if (arg != def->args()->nodeList().last())
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

void DumpVisitor::visitUndef(ASTNode *node)
{
    ASTUndef *und = static_cast<ASTUndef*>(node);
    printf("#undef %s\n", und->id()->spellName().toAscii().constData());
}

void DumpVisitor::visitLine(ASTNode *node)
{
    Q_UNUSED(node);
    printf("#line ...\n");
}

void DumpVisitor::visitPPTokens(ASTNode *node)
{
    ASTPPTokens *tokens = static_cast<ASTPPTokens *>(node);
    for (ASTNodeList::iterator iter = tokens->begin();
         iter != tokens->end(); iter++) {

        (*iter)->accept(this);
    }
}

void DumpVisitor::visitPPToken(ASTNode *node)
{
    printf("%s ", node->spellName().toAscii().constData());
}
