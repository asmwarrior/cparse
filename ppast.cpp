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

ASTNode *AST::CreatePunct(const QString &str)
{
    static bool mapInit = false;
    static QMap<QString, int> punctMap;
    if (!mapInit) {
        mapInit = true;
        punctMap.insert("[", '[');
        punctMap.insert("]", ']');
        punctMap.insert("(", '(');
        punctMap.insert(")", ')');
        punctMap.insert("{", '{');
        punctMap.insert("}", '}');
        punctMap.insert(".", '.');
        punctMap.insert("&", '&');
        punctMap.insert("*", '*');
        punctMap.insert("+", '+');
        punctMap.insert("-", '-');
        punctMap.insert("~", '~');
        punctMap.insert("/", '/');
        punctMap.insert("%", '%');
        punctMap.insert("^", '^');
        punctMap.insert("(", ')');
        punctMap.insert("|", '|');
        punctMap.insert("<", '<');
        punctMap.insert(">", '>');
        punctMap.insert("?", '?');
        punctMap.insert(":", ':');
        punctMap.insert(";", ';');
        punctMap.insert("=", '=');
        punctMap.insert(",", ',');
        punctMap.insert("#", '|');
        punctMap.insert("->", MEMBER_REF);
        punctMap.insert("++", PLUS_PLUS);
        punctMap.insert("--", MINUS_MINUS);
        punctMap.insert("<<", LSHIFT);
        punctMap.insert(">>", RSHIFT);
        punctMap.insert("<=", LE);
        punctMap.insert(">=", GE);
        punctMap.insert("==", EQ);
        punctMap.insert("!=", NE);
        punctMap.insert("&&", AND_AND);
        punctMap.insert("*=", MULT_ASSIGN);
        punctMap.insert("/=", DIV_ASSIGN);
        punctMap.insert("%=", MOD_ASSIGN);
        punctMap.insert("+=", PLUS_ASSIGN);
        punctMap.insert("-=", MINUS_ASSIGN);
        punctMap.insert("<<=", LSHIFT_ASSIGN);
        punctMap.insert(">>=", RSHIFT_ASSIGN);
        punctMap.insert("&=", AND_ASSIGN);
        punctMap.insert("^=", XOR_ASSIGN);
        punctMap.insert("|=", OR_ASSIGN);
        punctMap.insert("##", HASH_HASH);
        punctMap.insert("<:", '[');
        punctMap.insert(":>", ']');
        punctMap.insert("<%", '{');
        punctMap.insert("%>", '}');
        punctMap.insert("%:", '#');
        punctMap.insert("%:%:", HASH_HASH);
        punctMap.insert("...", DOT_DOT_DOT);
    }
    if (!punctMap.contains(str)) {
        qFatal("Unknown punct: %s", str.toAscii().constData());
        return 0;
    }
    return new ASTNode(punctMap.value(str), pptext);
}

ASTNode *AST::CreatePunct(char ch)
{
    char str[10];
    sprintf(str, "%c", ch);
    return CreatePunct(str);
}

ASTNode *AST::CreateKeyword(int type, const QString &str)
{
    return new ASTNode(type, str);
}

ASTNode *AST::CreateID(const QString &str)
{
    return new ASTNode(ID, str);
}

ASTNode *AST::CreatePPNumber(const QString &str)
{
    return new ASTNode(PP_NUMBER, str);
}

ASTNode *AST::CreateCharConstant(const QString &str)
{
    return new ASTNode(CHAR_CONSTANT, str);
}

ASTNode *AST::CreateStringLiteral(const QString &str)
{
    return new ASTNode(STRING_LITERAL, str);
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

void ASTNodeList::append(ASTNode *node)
{
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

}
