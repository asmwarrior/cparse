#include "ppast.h"
#include "ppastvisitor.h"
#include "pp_yacc.h"
#include <QString>
#include <QList>
#include <QMap>
#include <QObject>
#include <QMetaMethod>
#include <QDebug>

namespace PP {

class ASTNode::Private
{
public:
    ASTNode::Type type;
    QString name;
};

ASTNode::ASTNode(ASTNode::Type t, const QString &name)
    : d(new ASTNode::Private)
{
    d->type = t;
    d->name = name;
}

ASTNode::~ASTNode()
{
    delete d;
}

ASTNode::Type ASTNode::type() const
{
    return d->type;
}

QString ASTNode::spellName() const
{
    return d->name;
}

void ASTNode::accept(ASTVisitor *visitor)
{
    QString cName = metaObject()->className();
    QString mName;
    int idx;
    idx = cName.lastIndexOf("::");
    if (idx != -1)
        cName.remove(0, idx+2);    // remove XX:: Namespace prefix
    mName = cName;
    mName.remove(0, 3);    // remove AST prefix
    mName.prepend("visit");
    visitor->metaObject()->invokeMethod(visitor, mName.toAscii().constData(),
                                        Q_ARG(ASTNode *, this));
}

ASTNode *CreateOp(const QString &str)
{
    static bool mapInit = false;
    static QMap<QString, int> opMap;
    if (!mapInit) {
        mapInit = true;
        opMap.insert("defined", DEFINED);
        opMap.insert("[", '[');
        opMap.insert("]", ']');
        opMap.insert("(", '(');
        opMap.insert(")", ')');
        opMap.insert("{", '{');
        opMap.insert("}", '}');
        opMap.insert(".", '.');
        opMap.insert("&", '&');
        opMap.insert("*", '*');
        opMap.insert("+", '+');
        opMap.insert("-", '-');
        opMap.insert("~", '~');
        opMap.insert("/", '/');
        opMap.insert("%", '%');
        opMap.insert("^", '^');
        opMap.insert("(", ')');
        opMap.insert("|", '|');
        opMap.insert("<", '<');
        opMap.insert(">", '>');
        opMap.insert("?", '?');
        opMap.insert(":", ':');
        opMap.insert(";", ';');
        opMap.insert("=", '=');
        opMap.insert(",", ',');
        opMap.insert("#", '#');
        opMap.insert("->", MEMBER_REF);
        opMap.insert("++", PLUS_PLUS);
        opMap.insert("--", MINUS_MINUS);
        opMap.insert("<<", LSHIFT);
        opMap.insert(">>", RSHIFT);
        opMap.insert("<=", LE);
        opMap.insert(">=", GE);
        opMap.insert("==", EQ);
        opMap.insert("!=", NE);
        opMap.insert("&&", AND_AND);
        opMap.insert("*=", MULT_ASSIGN);
        opMap.insert("/=", DIV_ASSIGN);
        opMap.insert("%=", MOD_ASSIGN);
        opMap.insert("+=", PLUS_ASSIGN);
        opMap.insert("-=", MINUS_ASSIGN);
        opMap.insert("<<=", LSHIFT_ASSIGN);
        opMap.insert(">>=", RSHIFT_ASSIGN);
        opMap.insert("&=", AND_ASSIGN);
        opMap.insert("^=", XOR_ASSIGN);
        opMap.insert("|=", OR_ASSIGN);
        opMap.insert("##", HASH_HASH);
        opMap.insert("<:", '[');
        opMap.insert(":>", ']');
        opMap.insert("<%", '{');
        opMap.insert("%>", '}');
        opMap.insert("%:", '#');
        opMap.insert("%:%:", HASH_HASH);
        opMap.insert("...", DOT_DOT_DOT);
    }
    if (!opMap.contains(str)) {
        qFatal("Unknown punct: %s", str.toAscii().constData());
        return 0;
    }
    return new ASTToken(opMap.value(str), str);
}

ASTNode *CreateOp(char ch)
{
    char str[10];
    sprintf(str, "%c", ch);
    return CreateOp(str);
}

ASTNode *CreateID(const QString &str)
{
    return new ASTToken(ID, str);
}

ASTNode *CreatePPNumber(const QString &str)
{
    return new ASTToken(PP_NUMBER, str);
}

ASTNode *CreateCharConstant(const QString &str)
{
    return new ASTToken(CHAR_CONSTANT, str);
}

ASTNode *CreateStringLiteral(const QString &str)
{
    return new ASTToken(STRING_LITERAL, str);
}

ASTNode *CreateTokens(ASTToken *token)
{
    ASTTokens *tokens = new ASTTokens;
    tokens->append(token);
    return tokens;
}

ASTNode *CreateTokens()
{
    return new ASTTokens();
}

ASTNode *CreateInclude(ASTTokens *tokens)
{
    ASTInclude *inc;
    inc = new ASTInclude();
    inc->append(*tokens);
    delete tokens;
    return inc;
}

ASTNode *CreateDefine(ASTToken *id, ASTTokens *args,
                           ASTTokens *body)
{
    return new ASTDefine(id, args, body);
}

ASTNode *CreateDefineVarArgs(ASTToken *id, ASTTokens *args,
                           ASTTokens *body)
{
    ASTDefine *def = new ASTDefine(id, args, body);
    def->setVarArgs(true);
    return def;
}

ASTNode *CreateUndef(ASTToken *id)
{
    return new ASTUndef(id);
}

ASTNode *CreateLine(ASTTokens *tokens)
{
    ASTLine *line = new ASTLine();
    line->append(*tokens);
    delete tokens;
    return line;
}

ASTNode *CreateError()
{
    ASTError *err = new ASTError();
    return err;
}

ASTNode *CreateError(ASTTokens *tokens)
{
    ASTError *err = new ASTError();
    err->append(*tokens);
    delete tokens;
    return err;
}

ASTNode *CreatePragma()
{
    return new ASTPragma();
}

ASTNode *CreatePragma(ASTTokens *tokens)
{
    ASTPragma *pragma = new ASTPragma();
    pragma->append(*tokens);
    delete tokens;
    return pragma;
}

ASTNode *CreateNonDirective()
{
    return new ASTNonDirective();
}

ASTNode *CreateNonDirective(ASTTokens *tokens)
{
    ASTNonDirective *nond = new ASTNonDirective();
    nond->append(*tokens);
    delete tokens;
    return nond;
}

ASTNode *CreateTextLine()
{
    return new ASTTextLine();
}

ASTNode *CreateTextLine(ASTTokens *tokens)
{
    ASTTextLine *tl;
    tl = new ASTTextLine();
    tl->append(*tokens);
    delete tokens;
    return tl;
}

ASTNode *CreateConstantExpr(ASTTokens *tokens)
{
    ASTConstantExpr *constExpr = new ASTConstantExpr();
    constExpr->append(*tokens);
    delete tokens;
    return constExpr;
}

//=============================================================================

class ASTNodeList::Private
{
public:
    Private() {list.clear();}
    QList<ASTNode*> list;
};

ASTNodeList::ASTNodeList(Type type, const QString &name)
    : ASTNode(type, name),
      d(new ASTNodeList::Private)
{
}

ASTNodeList::~ASTNodeList()
{
    delete d;
}

bool ASTNodeList::isEmpty() const
{
    return d->list.isEmpty();
}

void ASTNodeList::append(ASTNode *node)
{
    if (node)
        d->list.append(node);
}

void ASTNodeList::append(const ASTNodeList &nodeList)
{
    d->list.append(nodeList.d->list);
}

ASTNodeList::iterator ASTNodeList::begin() const
{
    return d->list.begin();
}

ASTNodeList::iterator ASTNodeList::end() const
{
    return d->list.end();
}

QList<ASTNode *> &ASTNodeList::nodeList()
{
    return d->list;
}

class ASTDefine::Private
{
public:
    ASTToken *id;
    ASTNodeList *args;
    ASTNodeList *body;
    bool vargs;
};

ASTDefine::ASTDefine(ASTToken *id, ASTNodeList *args, ASTNodeList *body)
    : ASTNode(ASTNode::Define, "Define"),
      d(new ASTDefine::Private)
{
    d->id = id;
    d->args = args;
    d->body = body;
    d->vargs = false;
}

ASTDefine::~ASTDefine()
{
    delete d;
}

ASTToken* ASTDefine::id() const
{
    return d->id;
}

ASTNodeList *ASTDefine::args() const
{
    return d->args;
}

ASTNodeList *ASTDefine::body() const
{
    return d->body;
}

bool ASTDefine::isVarArgs() const
{
    return d->vargs;
}

void ASTDefine::setVarArgs(bool vargs)
{
    d->vargs = vargs;
}

ASTInclude::ASTInclude()
    : ASTNodeList(ASTNode::Include, "Include")
{
}

ASTInclude::~ASTInclude()
{
}

ASTLine::ASTLine()
    : ASTNodeList(ASTNode::Line, "Line")
{
}

ASTLine::~ASTLine()
{
}

ASTError::ASTError()
    : ASTNodeList(ASTNode::Error, "Error")
{
}

ASTError::~ASTError()
{
}

ASTPragma::ASTPragma()
    : ASTNodeList(ASTNode::Pragma, "Pragma")
{
}

ASTPragma::~ASTPragma()
{
}

class ASTUndef::Private
{
public:
    ASTToken *id;
};

ASTUndef::ASTUndef(ASTToken *id)
    : ASTNode(ASTNode::Undef, "Undef"),
      d(new ASTUndef::Private)
{
    d->id = id;
}

ASTUndef::~ASTUndef()
{
    delete d;
}

ASTToken* ASTUndef::id() const
{
    return d->id;
}

class ASTToken::Private
{
public:
    int tokenType;
};

ASTToken::ASTToken(int t, const QString &name)
    : ASTNode(ASTNode::Token, name),
      d(new ASTToken::Private)
{
    d->tokenType = t;
}

ASTToken::~ASTToken()
{
}

int ASTToken::tokenType() const
{
    return d->tokenType;
}

bool ASTToken::isOp() const
{
    return !isID() && !isPPNumber() & !isCharConstant() && !isStringLiteral();
}

bool ASTToken::isID() const
{
    return tokenType() == ID;
}

bool ASTToken::isPPNumber() const
{
    return tokenType() == PP_NUMBER;
}

bool ASTToken::isCharConstant() const
{
    return tokenType() == CHAR_CONSTANT;
}

bool ASTToken::isStringLiteral() const
{
    return tokenType() == STRING_LITERAL;
}

ASTConstantExpr::ASTConstantExpr()
    : ASTNodeList(ASTNode::Expr, "ConstantExpression")
{
}

ASTConstantExpr::~ASTConstantExpr()
{
}

ASTTokens::ASTTokens()
    : ASTNodeList(ASTNode::Tokens, "Tokens")
{
}

ASTTokens::~ASTTokens()
{
}

class ASTElifElement::Private
{
public:
    ASTGroup *group;
    ASTConstantExpr *expr;
};

ASTElifElement::ASTElifElement(ASTGroup *group, ASTConstantExpr *expr)
    : ASTNode(ASTNode::ElifElem, "Elif element"),
      d(new ASTElifElement::Private)
{
    d->group = group;
    d->expr = expr;
}

ASTElifElement::~ASTElifElement()
{
    delete d;
}

ASTGroup *ASTElifElement::group() const
{
    return d->group;
}

ASTConstantExpr *ASTElifElement::expr() const
{
    return d->expr;
}

ASTElifGroup::ASTElifGroup()
    : ASTNodeList(ASTNode::ElifGroup, "Elif group")
{
}

ASTElifGroup::~ASTElifGroup()
{
}

class ASTIfGroup::Private
{
public:
    ASTConstantExpr *expr;
    ASTGroup *trueBranch;
    ASTGroup *falseBranch;
};

ASTIfGroup::ASTIfGroup()
    : ASTNode(IfGroup, "If group"),
      d(new ASTIfGroup::Private)
{
}

ASTIfGroup::~ASTIfGroup()
{
    delete d;
}

void ASTIfGroup::setExpr(ASTConstantExpr *expr)
{
    d->expr = expr;
}

void ASTIfGroup::setTrueBranch(ASTGroup *group)
{
    d->trueBranch = group;
}

void ASTIfGroup::setFalseBranch(ASTGroup *group)
{
    d->falseBranch = group;
}

ASTConstantExpr *ASTIfGroup::expr() const
{
    return d->expr;
}

ASTGroup *ASTIfGroup::trueBranch() const
{
    return d->trueBranch;
}

ASTGroup *ASTIfGroup::falseBranch() const
{
    return d->falseBranch;
}

ASTGroup::ASTGroup()
    : ASTNodeList(ASTNode::Group, "Group")
{
}

ASTGroup::~ASTGroup()
{
}

ASTNonDirective::ASTNonDirective()
    : ASTNodeList(ASTNode::NonDirective, "Non directive")
{
}

ASTNonDirective::~ASTNonDirective()
{
}

ASTNode *CreateGroup(ASTNode *groupPart)
{
    ASTGroup *group = new ASTGroup();
    group->append(groupPart);
    return group;
}

ASTNode *CreateIfExpr(ASTConstantExpr *expr)
{
    return expr;
}

ASTNode *CreateIfdefExpr(ASTToken *id)
{
    ASTConstantExpr *expr = new ASTConstantExpr();
    expr->append(CreateOp("defined"));
    expr->append(CreateOp('('));
    expr->append(id);
    expr->append(CreateOp(')'));
    return expr;
}

ASTNode *CreateIfndefExpr(ASTToken *id)
{
    ASTConstantExpr *expr = new ASTConstantExpr();
    expr->append(CreateOp('!'));
    expr->append(CreateOp("defined"));
    expr->append(CreateOp('('));
    expr->append(id);
    expr->append(CreateOp(')'));
    return expr;
}

ASTNode *CreateIfGroup(ASTConstantExpr *expr, ASTGroup *trueBranch, ASTGroup *falseBranch)
{
    return CreateIfGroup(expr, NULL, trueBranch, falseBranch);
}

ASTNode *CreateIfGroup(ASTConstantExpr *expr, ASTElifGroup *elifGroup, ASTGroup *groupAfterElif, ASTGroup *elseBranch)
{
    ASTIfGroup *ifGroup, *aif, *bif;
    ASTElifElement *elem;

    ifGroup = new ASTIfGroup();
    ifGroup->setExpr(expr);
    aif = ifGroup;
    if (elifGroup)
        for (ASTNodeList::iterator iter = elifGroup->begin();
             iter != elifGroup->end(); iter++) {

            elem = static_cast<ASTElifElement*>(*iter);
            aif->setTrueBranch(elem->group());
            bif = new ASTIfGroup();
            bif->setExpr(elem->expr());
            aif->setFalseBranch(static_cast<ASTGroup*>(CreateGroup(bif)));
            aif = bif;
        }
    aif->setTrueBranch(groupAfterElif);
    aif->setFalseBranch(elseBranch);
    return ifGroup;
}

ASTNode *CreateElifGroup(ASTElifGroup *elifGroup, ASTGroup *group, ASTConstantExpr *expr)
{
    elifGroup->append(new ASTElifElement(group, expr));
    return elifGroup;
}

ASTNode *CreateElifGroup(ASTGroup *group, ASTConstantExpr *expr)
{
    ASTElifGroup *elifGroup = new ASTElifGroup;
    ASTElifElement *elifElem = new ASTElifElement(group, expr);
    elifGroup->append(elifElem);
    return elifGroup;
}

ASTTextLine::ASTTextLine()
    : ASTNodeList(ASTNode::TextLine, "TextLine")
{
}

ASTTextLine::~ASTTextLine()
{
}

}

#include "moc_ppast.cpp"
