#include "ppast.h"
#include "pp_yacc.h"
#include <QString>
#include <QList>
#include <QMap>
#include <QDebug>

extern char *pptext;
extern int ppleng;
extern int pplineno;

namespace PP {

class ASTNode::Private
{
public:
    int type;
    QString name;
};

ASTNode::ASTNode(int t, const QString &name)
    : d(new ASTNode::Private)
{
    d->type = t;
    d->name = name;
}

ASTNode::~ASTNode()
{
    delete d;
}

int ASTNode::type() const
{
    return d->type;
}

QString ASTNode::spellName() const
{
    return d->name;
}

ASTNode *AST::CreateOp(const QString &str)
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
    return new ASTToken(opMap.value(str), pptext);
}

ASTNode *AST::CreateOp(char ch)
{
    char str[10];
    sprintf(str, "%c", ch);
    return CreateOp(str);
}

ASTNode *AST::CreateID(const QString &str)
{
    return new ASTToken(ID, str);
}

ASTNode *AST::CreatePPNumber(const QString &str)
{
    return new ASTToken(PP_NUMBER, str);
}

ASTNode *AST::CreateCharConstant(const QString &str)
{
    return new ASTToken(CHAR_CONSTANT, str);
}

ASTNode *AST::CreateStringLiteral(const QString &str)
{
    return new ASTToken(STRING_LITERAL, str);
}

ASTNode *AST::CreateNodeList(ASTNode *node)
{
    ASTNodeList *nodeList = new ASTNodeList;
    nodeList->append(node);
    return nodeList;
}

ASTNode *AST::CreateNodeList()
{
    return new ASTNodeList;
}

ASTNode *AST::CreateInclude(ASTNodeList *list)
{
    return new ASTInclude(list);
}

ASTNode *AST::CreateDefine(ASTToken *id, ASTNodeList *args,
                           ASTNodeList *body)
{
    return new ASTDefine(id, args, body);
}

ASTNode *AST::CreateDefineVarArgs(ASTToken *id, ASTNodeList *args,
                           ASTNodeList *body)
{
    ASTDefine *def = new ASTDefine(id, args, body);
    def->setVarArgs(true);
    return def;
}

ASTNode *AST::CreateUndef(ASTToken *id)
{
    return new ASTUndef(id);
}

ASTNode *AST::CreateLine(ASTNodeList *list)
{
    return new ASTLine(list);
}

ASTNode *AST::CreateError()
{
    return new ASTError(static_cast<ASTNodeList*>(CreateNodeList()));
}

ASTNode *AST::CreateError(ASTNodeList *list)
{
    return new ASTError(list);
}

ASTNode *AST::CreatePragma(ASTNodeList *list)
{
    return new ASTPragma(list);
}

ASTNode *AST::CreateConstantExpr(ASTNodeList *list)
{
    return new ASTConstantExpr(list);
}

//=============================================================================

class ASTNodeList::Private
{
public:
    QList<ASTNode*> list;
};

ASTNodeList::ASTNodeList()
    : ASTNode(NODE_LIST, "NodeList"),
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

class ASTDefine::Private
{
public:
    ASTToken *id;
    ASTNodeList *args;
    ASTNodeList *body;
    bool vargs;
};

ASTDefine::ASTDefine(ASTToken *id, ASTNodeList *args, ASTNodeList *body)
    : ASTNode(DEFINE, "Define"),
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

class ASTInclude::Private
{
public:
    ASTNodeList *nodeList;
};

ASTInclude::ASTInclude(ASTNodeList *list)
    : ASTNode(INCLUDE, "Include"),
      d(new ASTInclude::Private)
{
    d->nodeList = list;
}

ASTInclude::~ASTInclude()
{
    delete d;
}

ASTNodeList *ASTInclude::nodeList() const
{
    return d->nodeList;
}

class ASTLine::Private
{
public:
    ASTNodeList *nodeList;
};

ASTLine::ASTLine(ASTNodeList *list)
    : ASTNode(LINE, "Line"),
      d(new ASTLine::Private)
{
    d->nodeList = list;
}

ASTLine::~ASTLine()
{
    delete d;
}

ASTNodeList *ASTLine::nodeList() const
{
    return d->nodeList;
}

class ASTError::Private
{
public:
    ASTNodeList *nodeList;
};

ASTError::ASTError(ASTNodeList *list)
    : ASTNode(ERROR, "Error"),
      d(new ASTError::Private)
{
    d->nodeList = list;
}

ASTError::~ASTError()
{
    delete d;
}

ASTNodeList *ASTError::nodeList() const
{
    return d->nodeList;
}

class ASTPragma::Private
{
public:
    ASTNodeList *nodeList;
};

ASTPragma::ASTPragma(ASTNodeList *list)
    : ASTNode(PRAGMA, "Pragma"),
      d(new ASTPragma::Private)
{
    d->nodeList = list;
}

ASTPragma::~ASTPragma()
{
    delete d;
}

ASTNodeList *ASTPragma::nodeList() const
{
    return d->nodeList;
}

class ASTUndef::Private
{
public:
    ASTToken *id;
};

ASTUndef::ASTUndef(ASTToken *id)
    : ASTNode(UNDEF, "Undef"),
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

ASTToken::ASTToken(int t, const QString &name)
    : ASTNode(t, name)
{
}

ASTToken::~ASTToken()
{
}

class ASTConstantExpr::Private
{
public:
    ASTNodeList *nodeList;
};

ASTConstantExpr::ASTConstantExpr(ASTNodeList *list)
    : ASTNode(CONSTANT_EXPR, "ConstantExpression"),
      d(new ASTConstantExpr::Private)
{
    d->nodeList = list;
}

ASTConstantExpr::~ASTConstantExpr()
{
    delete d;
}

ASTNodeList *ASTConstantExpr::nodeList() const
{
    return d->nodeList;
}

}
