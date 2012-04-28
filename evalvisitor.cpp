#include "evalvisitor.h"
#include "ast.h"
#include "astvisitor.h"
#include "combine_yacc.h"
#include "escapeseq.h"
#include <stdio.h>
#include <QList>
#include <QListIterator>
#include <QSet>
#include <QDebug>

class EvalVisitor::Private
{
public:
    Private(EvalVisitor *qq) : q(qq) {}
    void macroExpand(const QList<QSet<QString> > &setList,
                     QList<QSet<QString> > &exSetList,
                     const QList<ASTPPToken *> &tokens, QList<ASTPPToken *> &exTokens);
    void objMacroExpand(QListIterator<QSet<QString> > &sItor,
                        QList<QSet<QString> > &exSetList,
                        QListIterator<ASTPPToken *> &inItor,
                        QList<ASTPPToken *> &exTokens);
    void funcMacroExpand(QListIterator<QSet<QString> > &sItor,
                         QList<QSet<QString> > &exSetList,
                         QListIterator<ASTPPToken *> &inItor,
                         QList<ASTPPToken *> &exTokens);
    bool canMacroExpand(QListIterator<ASTPPToken *> &i) const;
    bool isObjMacro(ASTPPToken *tok) const;
    bool isFuncMacro(ASTPPToken *tok) const;
    ASTPPToken *concateToken(ASTPPToken *t1, ASTPPToken *t2);
    void filterHashHash(const QSet<QString> &mySet, QList<ASTPPToken*> &tokens,
                        QList<QSet<QString> > &setList);
    void filterPlaceMarker(const QSet<QString> &mySet, QList<ASTPPToken*> &tokens,
                           QList<QSet<QString> > &setList);
    void filterHash(const QMap<QString, QList<ASTPPToken*> > &argMap,
                    const QSet<QString> &mySet,
                    QList<ASTPPToken *> &tokens,
                    QList<QSet<QString> > &setList);
    void filterParam(const QMap<QString, QList<ASTPPToken*> > &exArgMap,
                     const QMap<QString, QList<QSet<QString> > > &exArgSetMap,
                     const QSet<QString> &mySet,
                     QList<ASTPPToken *> &tokens,
                     QList<QSet<QString> > &setList);
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
    QList<QSet<QString> > inSL, outSL;
    for (int i = 0; i < tg->tokenList().size(); i++)
        inSL << QSet<QString>();
    d->macroExpand(inSL, outSL, tg->tokenList(),
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

void EvalVisitor::Private::macroExpand(const QList<QSet<QString> > &setList,
    QList<QSet<QString> > &exSetList,
    const QList<ASTPPToken *> &tokens,
    QList<ASTPPToken *> &exTokens)
{
    QList<ASTPPToken*> inTokens, outTokens, tempTokens;
    QListIterator<ASTPPToken*> itor(inTokens);
    QListIterator<QSet<QString> > sitor(setList);
    QList<QSet<QString> > inSL, outSL, tempSL;
    bool redo;

    inTokens = tokens;
    outTokens.clear();
    inSL = setList;
    outSL.clear();
retry:
    itor = inTokens;
    sitor = inSL;
    redo = false;
    while (itor.hasNext()) {
        if (!canMacroExpand(itor)) {
            outTokens << itor.next();
            outSL << sitor.next();
        } else if (sitor.peekNext().contains(itor.peekNext()->spellName())) {
            outTokens << itor.next();
            outSL << sitor.next();
        } else {
            redo = true;
            tempTokens.clear();
            tempSL.clear();
            if (isObjMacro(itor.peekNext()))
                objMacroExpand(sitor, tempSL, itor, tempTokens);
            else if (isFuncMacro(itor.peekNext()))
                funcMacroExpand(sitor, tempSL, itor, tempTokens);

            outTokens << tempTokens;
            outSL << tempSL;
            break;
        }
    }
    if (redo) {
        while (itor.hasNext())
            outTokens << itor.next();
        while (sitor.hasNext())
            outSL << sitor.next();
        inTokens = outTokens;
        outTokens.clear();
        inSL = outSL;
        outSL.clear();
        goto retry;
    }
    exTokens = outTokens;
    exSetList = outSL;
}

void EvalVisitor::Private::objMacroExpand(QListIterator<QSet<QString> > &sItor,
    QList<QSet<QString> > &exSetList,
    QListIterator<ASTPPToken *> &inItor,
    QList<ASTPPToken *> &exTokens)
{
    QList<ASTPPToken*> tokens;
    QList<QSet<QString> > sl;
    QSet<QString> mySet;
    ASTPPToken *token;
    ASTDefine *def;
    exTokens.clear();
    exSetList.clear();
    token = inItor.next();
    mySet = sItor.next();
    def = static_cast<ASTDefine*>(q->context()->symtab.value(token->spellName()));
    mySet.insert(def->id()->spellName());
    if (!def->body())
        return;
    tokens = def->body()->tokenList();
    sl.clear();
    for (int i = 0; i < tokens.size(); i++)
        sl << mySet;
    filterHashHash(mySet, tokens, sl);
    //macroExpand(sl, exSetList, tokens, exTokens);
    exSetList = sl;
    exTokens = tokens;
}

void EvalVisitor::Private::funcMacroExpand(QListIterator<QSet<QString> > &sItor,
    QList<QSet<QString> > &exSetList,
    QListIterator<ASTPPToken *> &inItor,
    QList<ASTPPToken *> &exTokens)
{
    QList<ASTPPToken*> tokens, argTokens, exArgTokens;
    QList<QSet<QString> > sl, argSetList, exArgSetList;
    QSet<QString> mySet;
    ASTPPToken *token;
    ASTDefine *def;
    int i, brackets;
    QString param;
    QMap<QString, QList<ASTPPToken *> > argMap, exArgMap;
    QMap<QString, QList<QSet<QString> > > argSetMap, exArgSetMap;
    exTokens.clear();
    exSetList.clear();
    token = inItor.next();
    mySet = sItor.next();
    def = static_cast<ASTDefine*>(q->context()->symtab.value(token->spellName()));
    if (!inItor.hasNext() || (inItor.peekNext()->ppTokenType()!='(')) {
        //qWarning() << "Expect '(' to begin macro's argument list.";
        exTokens << token;
        exSetList << mySet;
        return;
    }
    mySet.insert(def->id()->spellName());
    inItor.next();
    sItor.next();
    i = 0;
    while (inItor.hasNext() && (inItor.peekNext()->ppTokenType() != ')')) {
        argTokens.clear();
        argSetList.clear();
        if (i < def->args()->size()) {
            while (inItor.hasNext() && (inItor.peekNext()->ppTokenType() != ',')
                   && (inItor.peekNext()->ppTokenType() != ')')) {
                argTokens << inItor.next();
                argSetList << sItor.next();
                if (inItor.peekPrevious()->ppTokenType() == '(') {
                    brackets = 1;
                    while (inItor.hasNext() && brackets) {
                        argTokens << inItor.next();
                        argSetList << sItor.next();
                        if (inItor.peekPrevious()->ppTokenType() == '(')
                            brackets++;
                        else if (inItor.peekPrevious()->ppTokenType() == ')')
                            brackets--;
                    }
                }
            }
            param = static_cast<ASTPPToken*>(def->args()->nodeList().at(i))->spellName();
            i++;
            if (inItor.hasNext() && (inItor.peekNext()->ppTokenType() == ',')) {
                inItor.next();  // skip ','
                sItor.next();
            }
        } else if ((i == def->args()->size()) && def->isVarArgs()) {
            // Variable Args '...'
            while (inItor.hasNext() && (inItor.peekNext()->ppTokenType() != ')')) {
                argTokens << inItor.next();
                argSetList << sItor.next();
                if (inItor.peekPrevious()->ppTokenType() == '(') {
                    brackets = 1;
                    while (inItor.hasNext() && brackets) {
                        argTokens << inItor.next();
                        argSetList << sItor.next();
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
            exTokens.clear();
            exSetList.clear();
            return;
        }
        if (argTokens.empty()) {
            argTokens << static_cast<ASTPPToken*>(CreatePlaceMarker());
            argSetList << mySet;
        }
        argMap.insert(param, argTokens);
        argSetMap.insert(param, argSetList);
        macroExpand(argSetList, exArgSetList, argTokens, exArgTokens);
        exArgMap.insert(param, exArgTokens);
        exArgSetMap.insert(param, exArgSetList);
    }
    if (!inItor.hasNext() || (inItor.peekNext()->ppTokenType()!=')')) {
        qWarning() << "Expect ')' to close macro's argument list.";
        return;
    }
    inItor.next(); // skip ')'
    sItor.next();

    QList<ASTPPToken *> emptyArg;
    QList<QSet<QString> > emptyArgSet;
    emptyArg << static_cast<ASTPPToken*>(CreatePlaceMarker());
    emptyArgSet << mySet;
    foreach (ASTPPToken *t, def->args()->tokenList())
        if (!argMap.contains(t->spellName())) {
            argMap.insert(t->spellName(), emptyArg);
            argSetMap.insert(t->spellName(), emptyArgSet);
            exArgMap.insert(t->spellName(), emptyArg);
            exArgSetMap.insert(t->spellName(), emptyArgSet);
        }
    if (def->isVarArgs() && !argMap.contains("__VA_ARGS__")) {
        argMap.insert("__VA_ARGS__", emptyArg);
        argSetMap.insert("__VA_ARGS__", emptyArgSet);
        exArgMap.insert("__VA_ARGS__", emptyArg);
        exArgSetMap.insert("__VA_ARGS__", emptyArgSet);
    }

    if (!def->body())
        return;
    tokens = def->body()->tokenList();
    sl.clear();
    for (int i = 0; i < tokens.size(); i++)
        sl << mySet;
    filterHash(argMap, mySet, tokens, sl);
    filterParam(exArgMap, exArgSetMap, mySet, tokens, sl);
    filterHashHash(mySet, tokens, sl);
    filterPlaceMarker(mySet, tokens, sl);
    //macroExpand(sl, exSetList, tokens, exTokens);
    exSetList = sl;
    exTokens = tokens;
}

bool EvalVisitor::Private::canMacroExpand(QListIterator<ASTPPToken *> &i) const
{
    ASTNode *node;
    ASTPPToken *tok;
    tok = i.next();
    if (tok->ppTokenType() == ID) {
        node = q->context()->symtab.value(tok->spellName());
        if (node && (node->type() == ASTNode::Define))
            if (isObjMacro(tok)
                    || (isFuncMacro(tok) && i.hasNext()
                        && static_cast<ASTPPToken*>(i.peekNext())->ppTokenType() == '(')) {
                i.previous();
                return true;
            }
    }
    i.previous();
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

void EvalVisitor::Private::filterHashHash(
        const QSet<QString> &mySet,
        QList<ASTPPToken *> &tokens,
        QList<QSet<QString> > &setList)
{
    ASTPPToken *t1, *t2;
    QMutableListIterator<ASTPPToken *> i(tokens);
    QMutableListIterator<QSet<QString> > si(setList);
    if (tokens.isEmpty())
        return;

    if ((tokens.front()->ppTokenType() == HASH_HASH)
            || (tokens.last()->ppTokenType() == HASH_HASH)) {
        qWarning() << "Macro's replacement list can't start or end with ##";
        tokens.clear();
        setList.clear();
        return;
    }
    while (i.hasNext()) {

        if (i.peekNext()->ppTokenType() == HASH_HASH) {
            t1 = i.peekPrevious();
            i.remove();
            si.remove();
            i.next();
            si.next();
            i.remove();
            si.remove();
            t2 = i.next();
            si.next();
            i.remove();
            si.remove();
            i.insert(concateToken(t1, t2));
            si.insert(mySet);
        } else {
            i.next();
            si.next();
        }
    }
}

void EvalVisitor::Private::filterPlaceMarker(
        const QSet<QString> &mySet,
        QList<ASTPPToken *> &tokens,
        QList<QSet<QString> > &setList)
{
    Q_UNUSED(mySet);
    QMutableListIterator<ASTPPToken *> i(tokens);
    QMutableListIterator<QSet<QString> > si(setList);
    while (i.hasNext()) {
        i.next();
        si.next();
        if (i.peekPrevious()->ppTokenType() == PLACE_MARKER) {
            i.remove();
            si.remove();
        }
    }
}

void EvalVisitor::Private::filterHash(
        const QMap<QString, QList<ASTPPToken *> > &argMap,
        const QSet<QString> &mySet,
        QList<ASTPPToken *> &tokens,
        QList<QSet<QString> > &setList)
{
    QMutableListIterator<ASTPPToken *> i(tokens);
    QMutableListIterator<QSet<QString> > si(setList);
    QString param;
    QString str;
    while (i.hasNext()) {
        i.next();
        si.next();
        if (i.peekPrevious()->ppTokenType() == '#') {
            if (i.hasNext() && (i.peekNext()->ppTokenType() == ID)
                    && argMap.contains(i.peekNext()->spellName())) {
                i.remove();
                si.remove();
                param = i.peekNext()->spellName();
                i.next();
                si.next();
                i.remove();
                si.remove();
                str.clear();
                foreach (ASTPPToken *t, argMap.value(param)) {
                    if (t->isStringLiteral())
                        str += EscapeSequence::Escape(t->spellName());
                    else
                        str += t->spellName();
                    str += " ";
                }
                if (str.endsWith(" "))
                    str.chop(1);
                str.prepend('"');
                str.append('"');
                i.insert(static_cast<ASTPPToken*>(CreateStringLiteral(str)));
                si.insert(mySet);
            } else {
                tokens.clear();
                setList.clear();
                qWarning() << "Expect macro parameter name after '#'";
                break;
            }
        }
    }
}

void EvalVisitor::Private::filterParam(
        const QMap<QString, QList<ASTPPToken *> > &exArgMap,
        const QMap<QString, QList<QSet<QString> > > &exArgSetMap,
        const QSet<QString> &mySet,
        QList<ASTPPToken *> &tokens,
        QList<QSet<QString> > &setList)
{
    QMutableListIterator<ASTPPToken *> i(tokens);
    QMutableListIterator<QSet<QString> > si(setList);
    QString param;
    while (i.hasNext()) {
        i.next();
        si.next();
        if ((i.peekPrevious()->ppTokenType() == ID)
                && exArgMap.contains(i.peekPrevious()->spellName())) {
            param = i.peekPrevious()->spellName();
            i.remove();
            si.remove();
            foreach (ASTPPToken *t, exArgMap.value(param))
                i.insert(t);
            foreach (QSet<QString> sl, exArgSetMap.value(param))
                si.insert(sl.unite(mySet));
        }
    }
}
