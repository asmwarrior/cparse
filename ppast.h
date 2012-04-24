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

ASTNode *CreateGroup(ASTNode *groupPart);
ASTNode *CreateIfExpr(ASTConstantExpr *expr);
ASTNode *CreateIfdefExpr(ASTToken *id);
ASTNode *CreateIfndefExpr(ASTToken *id);
ASTNode *CreateIfGroup(ASTConstantExpr *expr, ASTGroup *trueBranch,
                       ASTGroup *falseBranch);
ASTNode *CreateIfGroup(ASTConstantExpr *expr, ASTElifGroup *elifGroup,
                       ASTGroup *groupAfterElif, ASTGroup *elseBranch);
ASTNode *CreateElifGroup(ASTElifGroup *elifGroup, ASTGroup *group,
                         ASTConstantExpr *expr);
ASTNode *CreateElifGroup(ASTGroup *group, ASTConstantExpr *expr);
ASTNode *CreateOp(const QString &str);
ASTNode *CreateOp(char ch);
ASTNode *CreateID(const QString &str);
ASTNode *CreatePPNumber(const QString &str);
ASTNode *CreateCharConstant(const QString &str);
ASTNode *CreateStringLiteral(const QString &str);
ASTNode *CreateTokens(ASTToken* token);
ASTNode *CreateTokens();
ASTNode *CreateInclude(ASTTokens *tokens);
ASTNode *CreateDefine(ASTToken *id, ASTTokens *args,
                      ASTTokens *body);
ASTNode *CreateDefineVarArgs(ASTToken *id, ASTTokens *args,
                      ASTTokens *body);
ASTNode *CreateUndef(ASTToken *id);
ASTNode *CreateLine(ASTTokens *tokens);
ASTNode *CreateError();
ASTNode *CreateError(ASTTokens *tokens);
ASTNode *CreatePragma();
ASTNode *CreatePragma(ASTTokens *tokens);
ASTNode *CreateNonDirective();
ASTNode *CreateNonDirective(ASTTokens *tokens);
ASTNode *CreateTextLine();
ASTNode *CreateTextLine(ASTTokens *tokens);
ASTNode *CreateConstantExpr(ASTTokens *tokens);

class ASTVisitor;
class ASTNode: public QObject
{
    Q_OBJECT
public:
    enum Type {
        Token,
        Tokens,
        Include,
        Define,
        Error,
        Line,
        Pragma,
        Expr,
        Undef,
        IfGroup,
        TextLine,
        NonDirective,
        Group,
        ElifElem,
        ElifGroup
    };
    ASTNode(Type t, const QString &name);
    virtual ~ASTNode();
    Type type() const;
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
    int tokenType() const;
    bool isOp() const;
    bool isID() const;
    bool isPPNumber() const;
    bool isCharConstant() const;
    bool isStringLiteral() const;
private:
    class Private;
    Private *d;
};

class ASTNodeList: public ASTNode
{
    Q_OBJECT
public:
    typedef QList<ASTNode *>::iterator iterator;
    ASTNodeList(ASTNode::Type type, const QString &name);
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
