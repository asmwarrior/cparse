#ifndef PPAST_H
#define PPAST_H

#include <QString>
#include <QList>

namespace PP {

class ASTNode;
class AST
{
public:
    static ASTNode *CreatePunct(const QString &str);
    static ASTNode *CreatePunct(char ch);
    static ASTNode *CreateKeyword(int type, const QString &str);
    static ASTNode *CreateID(const QString &str);
    static ASTNode *CreatePPNumber(const QString &str);
    static ASTNode *CreateCharConstant(const QString &str);
    static ASTNode *CreateStringLiteral(const QString &str);
    static ASTNode *CreateNodeList(const QList<ASTNode*> &list);
    static ASTNode *CreateNodeList(ASTNode* list);
    static ASTNode *CreateNodeList();
private:
    AST();
    AST(const AST &);
};


class ASTNode
{
public:
    ASTNode(int t, const QString &name);
    virtual ~ASTNode();
    int type() const;
    QString spellName() const;
private:
    class Private;
    Private *d;
};

class ASTNodeList: public ASTNode
{
public:
    typedef QList<ASTNode *>::iterator iterator;
    ASTNodeList();
    virtual ~ASTNodeList();

    void append(ASTNode *node);
    void append(const ASTNodeList &nodeList);
    iterator begin() const;
    iterator end() const;
private:
    ASTNodeList(const ASTNodeList &);
    ASTNodeList& operator=(const ASTNodeList &);
    class Private;
    Private *d;
};

}

#endif // PPAST_H
