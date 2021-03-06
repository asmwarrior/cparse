#include "evalvisitor.h"
#include "ast.h"
#include "astexpr.h"
#include "astconstant.h"
#include "astvisitor.h"
#include "combine_yacc.h"
#include "escapeseq.h"
#include "pptokenlistlexer.h"
#include "parser.h"
#include "eval.h"
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
    void filterDefined(QList<ASTPPToken *> &tokens);
    void filterUndefined(QList<ASTPPToken *> &tokens);
    QString buildIncludeString(const QList<ASTPPToken*> &tokens);
    QString removeDoubleQuote(const QString &str);
    bool bracketMatched(const QList<ASTPPToken*> &tokens);
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

void EvalVisitor::visitInclude(ASTNode *node)
{
    ASTInclude *inc = static_cast<ASTInclude*>(node);
    QString str = d->buildIncludeString(inc->tokenList());
    ASTNode *saveRoot;
    parseFile(context(), str);
    if (!context()->root) {
        qWarning() << "Bad include: " << str;
        return;
    }
    if (context()->includeDepth >= 20) {
        qWarning() << "Recursive include depth exceeded";
        return;
    }
    ++context()->includeDepth;
    saveRoot = context()->root;
    context()->root->accept(this);
    context()->root = saveRoot;
    --context()->includeDepth;
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
    ASTTextLine *tl;
    QList<ASTPPToken*> tokens, outTokens;
    QList<QSet<QString> > inSL, outSL;
    int i, j;
    for (i = 0; i < tg->nodeList().size(); i++) {
        tl = static_cast<ASTTextLine*>(tg->nodeList().at(i));
        tokens << tl->tokenList();
        if (!d->bracketMatched(tokens))
            continue;
        for (j = 0; j < tokens.size(); j++)
            inSL << QSet<QString>();
        d->macroExpand(inSL, outSL, tokens,
                       outTokens);
        foreach (ASTPPToken *tok, outTokens)
            printf("%s ", tok->spellName().toUtf8().constData());
        printf("\n");
        d->ppTokens << outTokens;
        tokens.clear();
    }
    if (!tokens.isEmpty()) {
        qWarning() << "bracket not matched";
    }
}

void EvalVisitor::visitNonDirective(ASTNode *node)
{
    Q_UNUSED(node);
    printf("\n");
}

void EvalVisitor::visitPragma(ASTNode *node)
{
    Q_UNUSED(node);
    printf("\n");
}

void EvalVisitor::visitIfGroup(ASTNode *node)
{
    ASTIfGroup *ifg = static_cast<ASTIfGroup*>(node);
    Context ctx;
    PPTokenListLexer lexer;
    ASTInteger *integer;
    QList<ASTPPToken*> inTokens, outTokens;
    QList<QSet<QString> > inSL, outSL;
    inTokens = ifg->expr()->tokenList();
    d->filterDefined(inTokens);
    for (int i = 0; i < inTokens.size(); i++)
        inSL << QSet<QString>();
    d->macroExpand(inSL, outSL, inTokens, outTokens);
    d->filterUndefined(outTokens);
    lexer.setPPTokenList(outTokens);
    ctx.langDialect = Context::PPExpression;
    ctx.symtab = context()->symtab;
    parse(&ctx, &lexer);
    if (ctx.root && (integer = evalExpr(static_cast<ASTExpr*>(ctx.root)))) {
        if (!integer->isZero()) {
            if (ifg->trueBranch())
                visitGroup(ifg->trueBranch());
        } else {
            if (ifg->falseBranch())
                visitGroup(ifg->falseBranch());
        }
    } else {
        qDebug() << "Error parse #if ";
    }
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

void EvalVisitor::Private::filterDefined(QList<ASTPPToken *> &tokens)
{
    QMutableListIterator<ASTPPToken *> i(tokens);
    bool macroDefined;
    while (i.hasNext()) {
        i.next();
        if (i.peekPrevious()->ppTokenType() == DEFINED) {
            macroDefined = false;
            i.remove();
            i.next();
            if (i.peekPrevious()->ppTokenType() == ID) {
                if (q->context()->symtab.contains(i.peekPrevious()->spellName()))
                    macroDefined = true;
                else
                    macroDefined = false;
                i.remove();
            } else if (i.peekPrevious()->ppTokenType() == '(') {
                i.remove();
                i.next();
                if (i.peekPrevious()->ppTokenType() == ID) {
                    if (q->context()->symtab.contains(i.peekPrevious()->spellName()))
                        macroDefined = true;
                    else
                        macroDefined = false;
                } else {
                    qWarning() << "Expect ID after 'defined ('";
                    continue;
                }
                i.remove();
                i.next();
                if (i.peekPrevious()->ppTokenType() != ')') {
                    qWarning() << "Expect ')' after 'defined ( ID'";
                    continue;
                } else {
                    i.remove();
                }
            }
            ASTPPToken *ppToken;
            if (macroDefined)
                ppToken = static_cast<ASTPPToken*>(CreatePPNumber("1"));
            else
                ppToken = static_cast<ASTPPToken*>(CreatePPNumber("0"));
            i.insert(ppToken);
        }
    }
}

void EvalVisitor::Private::filterUndefined(QList<ASTPPToken *> &tokens)
{
    QMutableListIterator<ASTPPToken *> i(tokens);
    while (i.hasNext()) {
        i.next();
        if (i.peekPrevious()->isID()) {
            i.remove();
            i.insert(static_cast<ASTPPToken*>(CreatePPNumber("0")));
        }
    }
}

QString EvalVisitor::Private::buildIncludeString(const QList<ASTPPToken *> &inTokens)
{
    QString fname;
    int i;
    QList<ASTPPToken*> tokens, outTokens;
    QList<QSet<QString> > inSL, outSL;
    bool retried = false;
    tokens = inTokens;
retry:
    if (tokens.isEmpty())
        return QString();
    if ((tokens.size() == 1) && ((tokens.first()->ppTokenType() == STRING_LITERAL)
                                 || (tokens.first()->ppTokenType() == HEADER_NAME))) {
        fname = removeDoubleQuote(EscapeSequence::Unescape(tokens.at(0)->spellName()));
        goto out;
    }
    if ((tokens.size() > 2) && (tokens.first()->ppTokenType() == '<')
            && (tokens.last()->ppTokenType() == '>')) {
        for (i=1; i<tokens.size()-1; i++)
            fname.append(removeDoubleQuote(EscapeSequence::Unescape(tokens.at(i)->spellName())));
       goto out;
    }
    if (retried)
        goto out;
    for (i = 0; i < tokens.size(); i++)
        inSL << QSet<QString>();
    macroExpand(inSL, outSL, tokens,
                   outTokens);
    retried = true;
    goto retry;
out:
    return fname;
}

QString EvalVisitor::Private::removeDoubleQuote(const QString &str)
{
    if (str.startsWith("\"") && str.endsWith("\""))
        return str.mid(1, str.size()-2);
    return str;
}

bool EvalVisitor::Private::bracketMatched(const QList<ASTPPToken *> &tokens)
{
    int bdepth = 0;
    foreach (ASTPPToken *tok, tokens) {
        if (tok->ppTokenType() == '(')
            ++bdepth;
        if (tok->ppTokenType() == ')')
            --bdepth;
    }
    return bdepth == 0;
}
