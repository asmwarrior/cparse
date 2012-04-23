#ifndef PPAST_H
#define PPAST_H

#include <QString>
#include <QPair>
#include <QList>
#include <QObject>

namespace PP {

class ASTConstantExpr;
class ASTElifGroup;
class ASTGroup;
class ASTIfGroup;
class ASTNode;
class ASTNodeList;
class ASTToken;
class ASTTokens;
class ASTTextLine;
class AST
{
public:
    static ASTNode *CreateGroup(ASTNode *groupPart);
    static ASTNode *CreateIfExpr(ASTConstantExpr *expr);
    static ASTNode *CreateIfdefExpr(ASTToken *id);
    static ASTNode *CreateIfndefExpr(ASTToken *id);
    static ASTNode *CreateIfGroup(ASTConstantExpr *expr, ASTGroup *trueBranch,
                                  ASTGroup *falseBranch);
    static ASTNode *CreateIfGroup(ASTConstantExpr *expr, ASTElifGroup *elifGroup,
                                  ASTGroup *groupAfterElif, ASTGroup *elseBranch);
    static ASTNode *CreateElifGroup(ASTElifGroup *elifGroup, ASTGroup *group,
                                    ASTConstantExpr *expr);
    static ASTNode *CreateElifGroup(ASTGroup *group, ASTConstantExpr *expr);
    static ASTNode *CreateOp(const QString &str);
    static ASTNode *CreateOp(char ch);
    static ASTNode *CreateID(const QString &str);
    static ASTNode *CreatePPNumber(const QString &str);
    static ASTNode *CreateCharConstant(const QString &str);
    static ASTNode *CreateStringLiteral(const QString &str);
    static ASTNode *CreateTokens(ASTToken* token);
    static ASTNode *CreateTokens();
    static ASTNode *CreateInclude(ASTTokens *tokens);
    static ASTNode *CreateDefine(ASTToken *id, ASTTokens *args,
                                 ASTTokens *body);
    static ASTNode *CreateDefineVarArgs(ASTToken *id, ASTTokens *args,
                                 ASTTokens *body);
    static ASTNode *CreateUndef(ASTToken *id);
    static ASTNode *CreateLine(ASTTokens *tokens);
    static ASTNode *CreateError();
    static ASTNode *CreateError(ASTTokens *tokens);
    static ASTNode *CreatePragma();
    static ASTNode *CreatePragma(ASTTokens *tokens);
    static ASTNode *CreateNonDirective();
    static ASTNode *CreateNonDirective(ASTTokens *tokens);
    static ASTNode *CreateTextLine();
    static ASTNode *CreateTextLine(ASTTokens *tokens);
    static ASTNode *CreateConstantExpr(ASTTokens *tokens);
private:
    AST();
    AST(const AST &);
};

class ASTVisitor;
class ASTNode: public QObject
{
    Q_OBJECT
public:
    ASTNode(int t, const QString &name);
    virtual ~ASTNode();
    int type() const;
    QString spellName() const;
    virtual void accept(ASTVisitor *visitor);
private:
    class Private;
    Private *d;
};

class ASTToken : public ASTNode
{
    Q_OBJECT
public:
    ASTToken(int t, const QString &name);
    virtual ~ASTToken();
};

class ASTNodeList: public ASTNode
{
    Q_OBJECT
public:
    typedef QList<ASTNode *>::iterator iterator;
    ASTNodeList(int type, const QString &name);
    virtual ~ASTNodeList();
    bool isEmpty() const;
    void append(ASTNode *node);
    void append(const ASTNodeList &nodeList);
    iterator begin() const;
    iterator end() const;
    QList<ASTNode *> &nodeList();
private:
    ASTNodeList(const ASTNodeList &);
    ASTNodeList& operator=(const ASTNodeList &);
    class Private;
    Private *d;
};

class ASTTokens: public ASTNodeList
{
    Q_OBJECT
public:
    ASTTokens();
    ~ASTTokens();
};

class ASTInclude: public ASTNodeList
{
    Q_OBJECT
public:
    ASTInclude();
    ~ASTInclude();
private:
    class Private;
    Private *d;
};

class ASTDefine: public ASTNode
{
    Q_OBJECT
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
    Q_OBJECT
public:
    ASTUndef(ASTToken *id);
    ~ASTUndef();
    ASTToken *id() const;
private:
    class Private;
    Private *d;
};

class ASTLine: public ASTNodeList
{
    Q_OBJECT
public:
    ASTLine();
    ~ASTLine();
};

class ASTError: public ASTNodeList
{
    Q_OBJECT
public:
    ASTError();
    ~ASTError();
};

class ASTPragma: public ASTNodeList
{
    Q_OBJECT
public:
    ASTPragma();
    ~ASTPragma();
};

class ASTConstantExpr: public ASTNodeList
{
    Q_OBJECT
public:
    ASTConstantExpr();
    ~ASTConstantExpr();
};

class ASTGroup;
class ASTElifElement: public ASTNode
{
    Q_OBJECT
public:
    ASTElifElement(ASTGroup *group, ASTConstantExpr *expr);
    ~ASTElifElement();
    ASTGroup *group() const;
    ASTConstantExpr *expr() const;
private:
    class Private;
    Private *d;
};

class ASTElifGroup: public ASTNodeList
{
    Q_OBJECT
public:
    ASTElifGroup();
    ~ASTElifGroup();
};

class ASTIfGroup: public ASTNode
{
    Q_OBJECT
public:
    ASTIfGroup();
    ~ASTIfGroup();
    void setExpr(ASTConstantExpr *expr);
    void setTrueBranch(ASTGroup *group);
    void setFalseBranch(ASTGroup *group);
    ASTConstantExpr *expr() const;
    ASTGroup *trueBranch() const;
    ASTGroup *falseBranch() const;
private:
    class Private;
    Private *d;
};

class ASTGroup: public ASTNodeList
{
    Q_OBJECT
public:
    ASTGroup();
    ~ASTGroup();
};

class ASTNonDirective: public ASTNodeList
{
    Q_OBJECT
public:
    ASTNonDirective();
    ~ASTNonDirective();
};

class ASTTextLine: public ASTNodeList
{
    Q_OBJECT
public:
    ASTTextLine();
    ~ASTTextLine();
};

}

#endif // PPAST_H
