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
    QList<ASTPPToken*> filterHashHash(const QList<ASTPPToken*> &inTokens);
    QList<ASTPPToken*> filterPlaceMarker(const QList<ASTPPToken*> &inTokens);
    QList<ASTPPToken*> filterHash(const QMap<QString, QList<ASTPPToken*> > &argMap,
                                  const QList<ASTPPToken *> &inTokens);
    QList<ASTPPToken*> filterParam(const QMap<QString, QList<ASTPPToken*> > &exArgMap,
                                   const QList<ASTPPToken *> &inTokens);
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
    Q_UNUSED(node);
}

void EvalVisitor::visitPragma(ASTNode *node)
{
    Q_UNUSED(node);
}

void EvalVisitor::visitIfGroup(ASTNode *node)
{
    Q_UNUSED(node);
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
    ASTPPToken *token;
    ASTDefine *def;
    expandedTokens.clear();
    token = inItor.next();
    def = static_cast<ASTDefine*>(q->context()->symtab.value(token->spellName()));
    newbl.insert(def->id()->spellName());
    expandedMacros.insert(def->id()->spellName());
    if (!def->body())
        return;
    tokens = filterHashHash(def->body()->tokenList());
    macroExpand(newbl, expandedMacros, tokens, expandedTokens);
}

void EvalVisitor::Private::funcMacroExpand(const QSet<QString> &blacklist,
    QSet<QString> &expandedMacros,
    QListIterator<ASTPPToken *> &inItor,
    QList<ASTPPToken *> &expandedTokens)
{
    QList<ASTPPToken*> tokens, argTokens, expandedArgTokens;
    QSet<QString> newbl = blacklist;
    ASTPPToken *token;
    ASTDefine *def;
    int i, brackets;
    QString param;
    QMap<QString, QList<ASTPPToken *> > argMap, expandedArgMap;
    QSet<QString> argExpandedMacros;
    expandedTokens.clear();
    token = inItor.next();
    def = static_cast<ASTDefine*>(q->context()->symtab.value(token->spellName()));
    if (!inItor.hasNext() || (inItor.peekNext()->ppTokenType()!='(')) {
        //qWarning() << "Expect '(' to begin macro's argument list.";
        expandedTokens << token;
        return;
    }
    inItor.next();
    i = 0;
    while (inItor.hasNext() && (inItor.peekNext()->ppTokenType() != ')')) {
        argTokens.clear();
        if (i < def->args()->size()) {
            while (inItor.hasNext() && (inItor.peekNext()->ppTokenType() != ',')
                   && (inItor.peekNext()->ppTokenType() != ')')) {
                argTokens << inItor.next();
                if (inItor.peekPrevious()->ppTokenType() == '(') {
                    brackets = 1;
                    while (inItor.hasNext() && brackets) {
                        argTokens << inItor.next();
                        if (inItor.peekPrevious()->ppTokenType() == '(')
                            brackets++;
                        else if (inItor.peekPrevious()->ppTokenType() == ')')
                            brackets--;
                    }
                }
            }
            param = static_cast<ASTPPToken*>(def->args()->nodeList().at(i))->spellName();
            i++;
            if (inItor.hasNext() && (inItor.peekNext()->ppTokenType() == ','))
                inItor.next();  // skip ','
        } else if ((i == def->args()->size()) && def->isVarArgs()) {
            // Variable Args '...'
            while (inItor.hasNext() && (inItor.peekNext()->ppTokenType() != ')')) {
                argTokens << inItor.next();
                if (inItor.peekPrevious()->ppTokenType() == '(') {
                    brackets = 1;
                    while (inItor.hasNext() && brackets) {
                        argTokens << inItor.next();
                        if (inItor.peekPrevious()->ppTokenType() == '(')
                            brackets++;
                        else if (inItor.peekPrevious()->ppTokenType() == '(')
                            brackets--;
                    }
                }
            }
            param = "__VA_ARGS__";
        } else {
            qWarning() << "Too much macro arguments";
            expandedTokens.clear();
            return;
        }
        if (argTokens.empty())
            argTokens << static_cast<ASTPPToken*>(CreatePlaceMarker());
        argMap.insert(param, argTokens);
        argExpandedMacros.clear();
        macroExpand(QSet<QString>(), argExpandedMacros, argTokens, expandedArgTokens);
        expandedArgMap.insert(param, expandedArgTokens);
    }
    if (!inItor.hasNext() || (inItor.peekNext()->ppTokenType()!=')')) {
        qWarning() << "Expect ')' to close macro's argument list.";
        return;
    }
    inItor.next();
    newbl.insert(def->id()->spellName());
    expandedMacros.insert(def->id()->spellName());
    if (!def->body())
        return;
    tokens = def->body()->tokenList();
    tokens = filterHash(argMap, tokens);
    tokens = filterParam(expandedArgMap, tokens);
    tokens = filterHashHash(tokens);
    tokens = filterPlaceMarker(tokens);
    macroExpand(newbl, expandedMacros, tokens, expandedTokens);
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
    ASTNode *node = NULL;
    if (t1->isPlaceMarker())
        return t2;
    else if (t2->isPlaceMarker())
        return t1;
    if (t1->isID() && (t2->isID() || t2->isPPNumber())) {
        node = CreateID(t1->spellName() + t2->spellName());
    } else if (t1->isPPNumber() && (t2->isID() || t2->isPPNumber()
                                    || (t2->ppTokenType() == '+')
                                    || (t2->ppTokenType() == '-'))) {
        node = CreatePPNumber(t1->spellName() + t2->spellName());
    } else if (t1->isOp()  && t2->isOp()) {
        if (GOpMap.contains(t1->spellName() + t2->spellName()))
            node = CreateOp(t1->spellName() + t2->spellName());
    }
    return static_cast<ASTPPToken*>(node);
}

QList<ASTPPToken *> EvalVisitor::Private::filterHashHash(const QList<ASTPPToken*> &inTokens)
{
    ASTPPToken *t1, *t2;
    QList<ASTPPToken*> tokens;
    tokens = inTokens;
    QMutableListIterator<ASTPPToken *> i(tokens);
    if (tokens.isEmpty())
        return tokens;

    if ((tokens.front()->ppTokenType() == HASH_HASH)
            || (tokens.last()->ppTokenType() == HASH_HASH)) {
        qWarning() << "Macro's replacement list can't start or end with ##";
        tokens.clear();
        return tokens;
    }
    while (i.hasNext()) {
        if (i.peekNext()->ppTokenType() == HASH_HASH) {
            t1 = i.peekPrevious();
            i.remove();
            i.next();
            i.remove();
            t2 = i.next();
            i.remove();
            i.insert(concateToken(t1, t2));
        } else {
            i.next();
        }
    }
    return tokens;
}

QList<ASTPPToken *> EvalVisitor::Private::filterPlaceMarker(
        const QList<ASTPPToken*> &inTokens)
{
    QList<ASTPPToken*> tokens;
    tokens = inTokens;
    QMutableListIterator<ASTPPToken *> i(tokens);
    while (i.hasNext()) {
        if (i.next()->ppTokenType() == PLACE_MARKER)
            i.remove();
    }
    return tokens;
}

QList<ASTPPToken *> EvalVisitor::Private::filterHash(
        const QMap<QString, QList<ASTPPToken *> > &argMap,
        const QList<ASTPPToken *> &inTokens)
{
    QList<ASTPPToken*> tokens;
    tokens = inTokens;
    QMutableListIterator<ASTPPToken *> i(tokens);
    QString param;
    while (i.hasNext()) {
        if (i.next()->ppTokenType() == '#') {
            if (i.hasNext() && (i.peekNext()->ppTokenType() == ID)
                    && argMap.contains(i.peekNext()->spellName())) {
                i.remove();
                param = i.peekNext()->spellName();
                i.next();
                i.remove();
                foreach (ASTPPToken *t, argMap.value(param))
                    i.insert(t);
            } else {
                tokens.clear();
                qWarning() << "Expect macro parameter name after '#'";
                break;
            }
        }
    }
    return tokens;
}

QList<ASTPPToken *> EvalVisitor::Private::filterParam(
        const QMap<QString, QList<ASTPPToken *> > &exArgMap,
        const QList<ASTPPToken *> &inTokens)
{
    QList<ASTPPToken*> tokens;
    tokens = inTokens;
    QMutableListIterator<ASTPPToken *> i(tokens);
    QString param;
    while (i.hasNext()) {
        if ((i.next()->ppTokenType() == ID)
                && exArgMap.contains(i.peekPrevious()->spellName())) {

            param = i.peekPrevious()->spellName();
            i.remove();
            foreach (ASTPPToken *t, exArgMap.value(param))
                i.insert(t);
        }
    }
    return tokens;
}
