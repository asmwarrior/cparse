#include "ast.h"
#include "astvisitor.h"
#include "combine_yacc.h"
#include <QString>
#include <QList>
#include <QMap>
#include <QObject>
#include <QMetaMethod>
#include <QDebug>

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
        opMap.insert("!", '!');
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
    return new ASTPPToken(opMap.value(str), str);
}

ASTNode *CreateOp(char ch)
{
    char str[10];
    sprintf(str, "%c", ch);
    return CreateOp(str);
}

ASTNode *CreateID(const QString &str)
{
    return new ASTPPToken(ID, str);
}

ASTNode *CreatePPNumber(const QString &str)
{
    return new ASTPPToken(PP_NUMBER, str);
}

ASTNode *CreateCharConstant(const QString &str)
{
    return new ASTPPToken(CHAR_CONSTANT, str);
}

ASTNode *CreateStringLiteral(const QString &str)
{
    return new ASTPPToken(STRING_LITERAL, str);
}

ASTNode *CreatePPTokens(ASTPPToken *PPToken)
{
    ASTPPTokens *PPTokens = new ASTPPTokens;
    PPTokens->append(PPToken);
    return PPTokens;
}

ASTNode *CreatePPTokens()
{
    return new ASTPPTokens();
}

ASTNode *CreateInclude(ASTPPTokens *PPTokens)
{
    ASTInclude *inc;
    inc = new ASTInclude();
    inc->append(*PPTokens);
    delete PPTokens;
    return inc;
}

ASTNode *CreateDefine(ASTPPToken *id, ASTPPTokens *args,
                           ASTPPTokens *body)
{
    return new ASTDefine(id, args, body);
}

ASTNode *CreateDefineVarArgs(ASTPPToken *id, ASTPPTokens *args,
                           ASTPPTokens *body)
{
    ASTDefine *def = new ASTDefine(id, args, body);
    def->setVarArgs(true);
    return def;
}

ASTNode *CreateUndef(ASTPPToken *id)
{
    return new ASTUndef(id);
}

ASTNode *CreateLine(ASTPPTokens *PPTokens)
{
    ASTLine *line = new ASTLine();
    line->append(*PPTokens);
    delete PPTokens;
    return line;
}

ASTNode *CreateError()
{
    ASTError *err = new ASTError();
    return err;
}

ASTNode *CreateError(ASTPPTokens *PPTokens)
{
    ASTError *err = new ASTError();
    err->append(*PPTokens);
    delete PPTokens;
    return err;
}

ASTNode *CreatePragma()
{
    return new ASTPragma();
}

ASTNode *CreatePragma(ASTPPTokens *PPTokens)
{
    ASTPragma *pragma = new ASTPragma();
    pragma->append(*PPTokens);
    delete PPTokens;
    return pragma;
}

ASTNode *CreateNonDirective()
{
    return new ASTNonDirective();
}

ASTNode *CreateNonDirective(ASTPPTokens *PPTokens)
{
    ASTNonDirective *nond = new ASTNonDirective();
    nond->append(*PPTokens);
    delete PPTokens;
    return nond;
}

ASTNode *CreateTextLine()
{
    return new ASTTextLine();
}

ASTNode *CreateTextLine(ASTPPTokens *PPTokens)
{
    ASTTextLine *tl;
    tl = new ASTTextLine();
    tl->append(*PPTokens);
    delete PPTokens;
    return tl;
}

ASTNode *CreateConstantExpr(ASTPPTokens *PPTokens)
{
    ASTConstantExpr *constExpr = new ASTConstantExpr();
    constExpr->append(*PPTokens);
    delete PPTokens;
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
    ASTPPToken *id;
    ASTNodeList *args;
    ASTNodeList *body;
    bool vargs;
};

ASTDefine::ASTDefine(ASTPPToken *id, ASTNodeList *args, ASTNodeList *body)
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

ASTPPToken* ASTDefine::id() const
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
    ASTPPToken *id;
};

ASTUndef::ASTUndef(ASTPPToken *id)
    : ASTNode(ASTNode::Undef, "Undef"),
      d(new ASTUndef::Private)
{
    d->id = id;
}

ASTUndef::~ASTUndef()
{
    delete d;
}

ASTPPToken* ASTUndef::id() const
{
    return d->id;
}

class ASTPPToken::Private
{
public:
    int ppTokenType;
};

ASTPPToken::ASTPPToken(int t, const QString &name)
    : ASTNode(ASTNode::PPToken, name),
      d(new ASTPPToken::Private)
{
    d->ppTokenType = t;
}

ASTPPToken::~ASTPPToken()
{
}

int ASTPPToken::ppTokenType() const
{
    return d->ppTokenType;
}

bool ASTPPToken::isOp() const
{
    return !isID() && !isPPNumber() & !isCharConstant() && !isStringLiteral();
}

bool ASTPPToken::isID() const
{
    return ppTokenType() == ID;
}

bool ASTPPToken::isPPNumber() const
{
    return ppTokenType() == PP_NUMBER;
}

bool ASTPPToken::isCharConstant() const
{
    return ppTokenType() == CHAR_CONSTANT;
}

bool ASTPPToken::isStringLiteral() const
{
    return ppTokenType() == STRING_LITERAL;
}

ASTConstantExpr::ASTConstantExpr()
    : ASTNodeList(ASTNode::Expr, "ConstantExpression")
{
}

ASTConstantExpr::~ASTConstantExpr()
{
}

ASTPPTokens::ASTPPTokens()
    : ASTNodeList(ASTNode::PPTokens, "PPTokens")
{
}

ASTPPTokens::~ASTPPTokens()
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

ASTNode *CreateIfdefExpr(ASTPPToken *id)
{
    ASTConstantExpr *expr = new ASTConstantExpr();
    expr->append(CreateOp("defined"));
    expr->append(CreateOp('('));
    expr->append(id);
    expr->append(CreateOp(')'));
    return expr;
}

ASTNode *CreateIfndefExpr(ASTPPToken *id)
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
