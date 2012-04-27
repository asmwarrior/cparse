#include "evalvisitor.h"
#include "ast.h"
#include "astvisitor.h"
#include "combine_yacc.h"
#include <stdio.h>
#include <QList>
#include <QListIterator>
#include <QSet>
#include <QDebug>

class EvalVisitor::Private
{
public:
    Private(EvalVisitor *qq) : q(qq) {}
    void macroExpand(const QSet<QString> &blacklist,
                     QSet<QString> &expandedMacros,
                     const QList<ASTPPToken *> &tokens, QList<ASTPPToken *> &expandedTokens);
    void objMacroExpand(const QSet<QString> &blacklist,
                        QSet<QString> &expandedMacros,
                        QListIterator<ASTPPToken *> &inItor,
                        QList<ASTPPToken *> &expandedTokens);
    void funcMacroExpand(const QSet<QString> &blacklist,
                         QSet<QString> &expandedMacros,
                         QListIterator<ASTPPToken *> &inItor,
                         QList<ASTPPToken *> &expandedTokens);
    bool canMacroExpand(ASTPPToken *tok) const;
    bool isObjMacro(ASTPPToken *tok) const;
    bool isFuncMacro(ASTPPToken *tok) const;
    ASTPPToken *concateToken(ASTPPToken *t1, ASTPPToken *t2);
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

void EvalVisitor::visitTextGroup(ASTNode *node)
{
    ASTTextGroup *tg = static_cast<ASTTextGroup*>(node);
    QList<ASTPPToken*> outTokens;
    QSet<QString> eset;
    d->macroExpand(QSet<QString>(), eset, tg->tokenList(),
                   outTokens);
    foreach (ASTPPToken *tok, outTokens)
        printf("%s ", tok->spellName().toUtf8().constData());
    printf("\n");
    d->ppTokens << outTokens;
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

EvalVisitor::PPTokenList EvalVisitor::ppTokens() const
{
    return d->ppTokens;
}

void EvalVisitor::Private::macroExpand(const QSet<QString> &blacklist,
    QSet<QString> &expandedMacros,
    const QList<ASTPPToken *> &tokens,
    QList<ASTPPToken *> &expandedTokens)
{
    QList<ASTPPToken*> inTokens, outTokens, tempTokens;
    QListIterator<ASTPPToken*> itor(inTokens);
    QSet<QString> bl;
    ASTPPToken *tok;
    bool redo;

    inTokens = tokens;
    outTokens.clear();
    bl = blacklist;
retry:
    tempTokens.clear();
    itor = inTokens;
    redo = false;
    while (itor.hasNext()) {
        tok = itor.peekNext();
        if (!canMacroExpand(tok))
            outTokens << itor.next();
        else if (bl.contains(tok->spellName()))
            outTokens << itor.next();
        else {
            redo = true;
            if (isObjMacro(tok))
                objMacroExpand(bl, expandedMacros, itor, tempTokens);
            else if (isFuncMacro(tok))
                funcMacroExpand(bl, expandedMacros, itor, tempTokens);

            outTokens << tempTokens;
        }
    }
    if (redo) {
        inTokens = outTokens;
        outTokens.clear();
        bl.unite(expandedMacros);
        goto retry;
    }
    expandedTokens = outTokens;
}

void EvalVisitor::Private::objMacroExpand(const QSet<QString> &blacklist,
    QSet<QString> &expandedMacros,
    QListIterator<ASTPPToken *> &inItor,
    QList<ASTPPToken *> &expandedTokens)
{
    QList<ASTPPToken*> tokens, outTokens;
    QSet<QString> newbl = blacklist;
    ASTPPToken *token, *nextToken;
    ASTDefine *def;
    expandedTokens.clear();
    token = inItor.next();
    def = static_cast<ASTDefine*>(q->context()->symtab.value(token->spellName()));
    newbl.insert(def->id()->spellName());
    expandedMacros.insert(def->id()->spellName());
    if (!def->body() || def->body()->isEmpty())
        return;
    if ((def->body()->tokenList().front()->ppTokenType() == HASH_HASH)
            || (def->body()->tokenList().last()->ppTokenType() == HASH_HASH)) {
        qWarning() << "Macro's replacement list can't start or end with ##";
        expandedTokens.clear();
        return;
    }
    tokens.clear();
    QListIterator<ASTPPToken *> i(def->body()->tokenList());
    while (i.hasNext()) {
        token = i.next();
        if (i.hasNext() && (i.peekNext()->ppTokenType() == HASH_HASH)) {
            do {
                i.next();   // skip '##'
                nextToken = i.next(); // can't fail
                ASTPPToken *newToken = concateToken(token, nextToken);
                if (newToken) {
                    tokens << newToken;
                    token = newToken;
                }
            } while (i.hasNext() && (i.peekNext()->ppTokenType() == HASH_HASH));
        } else
            tokens << token;
    }
    macroExpand(newbl, expandedMacros, tokens, outTokens);
    expandedTokens = outTokens;
}

void EvalVisitor::Private::funcMacroExpand(const QSet<QString> &blacklist,
    QSet<QString> &expandedMacros,
    QListIterator<ASTPPToken *> &inItor,
    QList<ASTPPToken *> &expandedTokens)
{
    QList<ASTPPToken*> tokens, outTokens;
    QSet<QString> newbl = blacklist;
    ASTPPToken *token, *nextToken;
    ASTDefine *def;
    expandedTokens.clear();
    token = inItor.next();
    if (!inItor.hasNext() || (inItor.peekNext()->ppTokenType()!='(')) {
        expandedTokens << token;
        return;
    }
    def = static_cast<ASTDefine*>(q->context()->symtab.value(token->spellName()));
    newbl.insert(def->id()->spellName());
    expandedMacros.insert(def->id()->spellName());
    if (!def->body() || def->body()->isEmpty())
        return;
    if ((def->body()->tokenList().front()->ppTokenType() == HASH_HASH)
            || (def->body()->tokenList().last()->ppTokenType() == HASH_HASH)) {
        qWarning() << "Macro's replacement list can't start or end with ##";
        expandedTokens.clear();
        return;
    }
    tokens.clear();
    QListIterator<ASTPPToken *> i(def->body()->tokenList());
    while (i.hasNext()) {
        token = i.next();
        if (i.hasNext() && (i.peekNext()->ppTokenType() == HASH_HASH)) {
            do {
                i.next();   // skip '##'
                nextToken = i.next(); // can't fail
                ASTPPToken *newToken = concateToken(token, nextToken);
                if (newToken) {
                    tokens << newToken;
                    token = newToken;
                } else
                    break;
            } while (i.hasNext() && (i.peekNext()->ppTokenType() == HASH_HASH));
        } else if (token->ppTokenType() == '#') {
            if (!i.hasNext()) {
                qWarning() << "Need macro paramenter after '#'";
                expandedTokens.clear();
                return;
            }
            token = i.next();
            // TODO: fix it
            tokens << CreateStringLiteral(token->spellName());
        } else {
            tokens << token;
        }

    }
    macroExpand(newbl, expandedMacros, tokens, outTokens);
    expandedTokens = outTokens;
    return;
}

bool EvalVisitor::Private::canMacroExpand(ASTPPToken *tok) const
{
    ASTNode *node;
    if (tok->ppTokenType() == ID) {
        node = q->context()->symtab.value(tok->spellName());
        if (node && (node->type() == ASTNode::Define))
            return true;
    }
    return false;
}

bool EvalVisitor::Private::isObjMacro(ASTPPToken *tok) const
{
    ASTDefine *def;
    def = static_cast<ASTDefine*>(q->context()->symtab.value(tok->spellName()));
    return def && !def->args();
}

bool EvalVisitor::Private::isFuncMacro(ASTPPToken *tok) const
{
    ASTDefine *def;
    def = static_cast<ASTDefine*>(q->context()->symtab.value(tok->spellName()));
    return def && def->args();
}

ASTPPToken *EvalVisitor::Private::concateToken(ASTPPToken *t1, ASTPPToken *t2)
{
    return NULL;
}
