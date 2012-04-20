#ifndef PPAST_H
#define PPAST_H

#include <QString>
#include <QList>

namespace PP {

class ASTNode;
class ASTToken;
class ASTNodeList;
class AST
{
public:
    static ASTNode *CreateOp(const QString &str);
    static ASTNode *CreateOp(char ch);
    static ASTNode *CreateID(const QString &str);
    static ASTNode *CreatePPNumber(const QString &str);
    static ASTNode *CreateCharConstant(const QString &str);
    static ASTNode *CreateStringLiteral(const QString &str);
    static ASTNode *CreateNodeList(const QList<ASTNode*> &list);
    static ASTNode *CreateNodeList(ASTNode* list);
    static ASTNode *CreateNodeList();
    static ASTNode *CreateInclude(ASTNodeList *list);
    static ASTNode *CreateDefine(ASTToken *id, ASTNodeList *args,
                                 ASTNodeList *body);
    static ASTNode *CreateDefineVarArgs(ASTToken *id, ASTNodeList *args,
                                 ASTNodeList *body);
    static ASTNode *CreateUndef(ASTToken *id);
    static ASTNode *CreateLine(ASTNodeList *list);
    static ASTNode *CreateError();
    static ASTNode *CreateError(ASTNodeList *list);
    static ASTNode *CreatePragma(ASTNodeList *list);
    static ASTNode *CreateConstantExpr(ASTNodeList *list);
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

class ASTToken : public ASTNode
{
public:
    ASTToken(int t, const QString &name);
    virtual ~ASTToken();
};

class ASTNodeList: public ASTNode
{
public:
    typedef QList<ASTNode *>::iterator iterator;
    ASTNodeList();
    virtual ~ASTNodeList();
    bool isEmpty() const;
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

class ASTInclude: public ASTNode
{
public:
    ASTInclude(ASTNodeList *list);
    ~ASTInclude();
    ASTNodeList *nodeList() const;
private:
    class Private;
    Private *d;
};

class ASTDefine: public ASTNode
{
public:
    ASTDefine(ASTToken *id, ASTNodeList *args, ASTNodeList *body);
    ~ASTDefine();
    ASTToken *id() const;
    ASTNodeList *args() const;
    ASTNodeList *body() const;
    bool isVarArgs() const;
    void setVarArgs(bool vargs);
private:
    class Private;
    Private *d;
};

class ASTUndef: public ASTNode
{
public:
    ASTUndef(ASTToken *id);
    ~ASTUndef();
    ASTToken *id() const;
private:
    class Private;
    Private *d;
};

class ASTLine: public ASTNode
{
public:
    ASTLine(ASTNodeList *list);
    ~ASTLine();
    ASTNodeList *nodeList() const;
private:
    class Private;
    Private *d;
};

class ASTError: public ASTNode
{
public:
    ASTError(ASTNodeList *list);
    ~ASTError();
    ASTNodeList *nodeList() const;
private:
    class Private;
    Private *d;
};

class ASTPragma: public ASTNode
{
public:
    ASTPragma(ASTNodeList *list);
    ~ASTPragma();
    ASTNodeList *nodeList() const;
private:
    class Private;
    Private *d;
};

class ASTConstantExpr: public ASTNode
{
public:
    ASTConstantExpr(ASTNodeList *list);
    ~ASTConstantExpr();
    ASTNodeList *nodeList() const;
private:
    class Private;
    Private *d;
};

}

#endif // PPAST_H
