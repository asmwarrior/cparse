#ifndef AST_H
#define AST_H

#include <QString>
#include <QPair>
#include <QList>
#include <QObject>

class ASTConstantExpr;
class ASTElifGroup;
class ASTGroup;
class ASTIfGroup;
class ASTNode;
class ASTNodeList;
class ASTPPToken;
class ASTPPTokens;
class ASTTextLine;

ASTNode *CreateGroup(ASTNode *groupPart);
ASTNode *CreateIfExpr(ASTConstantExpr *expr);
ASTNode *CreateIfdefExpr(ASTPPToken *id);
ASTNode *CreateIfndefExpr(ASTPPToken *id);
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
ASTNode *CreatePPTokens(ASTPPToken* PPToken);
ASTNode *CreatePPTokens();
ASTNode *CreateInclude(ASTPPTokens *PPTokens);
ASTNode *CreateDefine(ASTPPToken *id, ASTPPTokens *args,
                      ASTPPTokens *body);
ASTNode *CreateDefineVarArgs(ASTPPToken *id, ASTPPTokens *args,
                      ASTPPTokens *body);
ASTNode *CreateUndef(ASTPPToken *id);
ASTNode *CreateLine(ASTPPTokens *PPTokens);
ASTNode *CreateError();
ASTNode *CreateError(ASTPPTokens *PPTokens);
ASTNode *CreatePragma();
ASTNode *CreatePragma(ASTPPTokens *PPTokens);
ASTNode *CreateNonDirective();
ASTNode *CreateNonDirective(ASTPPTokens *PPTokens);
ASTNode *CreateTextLine();
ASTNode *CreateTextLine(ASTPPTokens *PPTokens);
ASTNode *CreateConstantExpr(ASTPPTokens *PPTokens);

class ASTVisitor;
class ASTNode: public QObject
{
    Q_OBJECT
public:
    enum Type {
        PPToken,
        PPTokens,
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
        ElifGroup,
        Constant
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

class ASTPPToken : public ASTNode
{
    Q_OBJECT
public:
    ASTPPToken(int t, const QString &name);
    virtual ~ASTPPToken();
    int ppTokenType() const;
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

class ASTPPTokens: public ASTNodeList
{
    Q_OBJECT
public:
    ASTPPTokens();
    ~ASTPPTokens();
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
    ASTDefine(ASTPPToken *id, ASTNodeList *args, ASTNodeList *body);
    ~ASTDefine();
    ASTPPToken *id() const;
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
    ASTUndef(ASTPPToken *id);
    ~ASTUndef();
    ASTPPToken *id() const;
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

#endif // AST_H
