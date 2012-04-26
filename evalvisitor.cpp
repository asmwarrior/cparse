#include "evalvisitor.h"
#include "astvisitor.h"
#include "combine_yacc.h"
#include <stdio.h>
#include <QList>
#include <QDebug>

class EvalVisitor::Private
{
public:
    Private(EvalVisitor *qq) : q(qq) {}
    EvalVisitor::MacroArgList getMacroArg(ASTTextLines *tls, int *idx, bool *ok);
    EvalVisitor::MacroArgList getMacroArgs(ASTTextLines *tls, int *idx, bool *ok);
    EvalVisitor::PPTokenList expandFunctionMacro(
            ASTDefine *def, const EvalVisitor::MacroArgList &args);
    EvalVisitor::PPTokenList expandObjectMacro(
            ASTDefine *def);
    EvalVisitor *q;
    QList<ASTPPToken*> ppTokens;
};

EvalVisitor::EvalVisitor(Context *ctx)
    : ASTVisitor(ctx),
      d(new EvalVisitor::Private(this))
{
}

EvalVisitor::~EvalVisitor()
{
    delete d;
}

void EvalVisitor::visitGroup(ASTNode *node)
{
    ASTGroup *group = static_cast<ASTGroup *>(node);
    for (ASTNodeList::iterator iter = group->begin();
         iter != group->end(); iter++) {

        (*iter)->accept(this);
    }
}

void EvalVisitor::visitTextLines(ASTNode *node)
{

}

void EvalVisitor::visitNonDirective(ASTNode *node)
{
}

void EvalVisitor::visitPragma(ASTNode *node)
{
}

void EvalVisitor::visitIfGroup(ASTNode *node)
{
}

void EvalVisitor::visitDefine(ASTNode *node)
{
    ASTDefine *def = static_cast<ASTDefine*>(node);
    context()->symtab.insert(def->id()->spellName(), def);
}

void EvalVisitor::visitUndef(ASTNode *node)
{
    ASTUndef *und = static_cast<ASTUndef*>(node);
    context()->symtab.remove(und->id()->spellName());
}

EvalVisitor::MacroArgList EvalVisitor::Private::getMacroArg(ASTTextLines *tls, int *idx, bool *ok)
{
}

EvalVisitor::MacroArgList EvalVisitor::Private::getMacroArgs(ASTTextLines *tls, int *idx, bool *ok)
{
}

EvalVisitor::PPTokenList EvalVisitor::Private::expandFunctionMacro(ASTDefine *def,
    const EvalVisitor::MacroArgList &args)
{
}

EvalVisitor::PPTokenList EvalVisitor::Private::expandObjectMacro(ASTDefine *def)
{
}
