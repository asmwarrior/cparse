#ifndef ASTEXPR_H
#define ASTEXPR_H

#include "ast.h"
#include <QObject>

class ASTNode;
class ASTExprType;
class ASTExpr: public ASTNode
{
    Q_OBJECT
public:
    enum ExprType {
        PrimaryExpr,
        UnaryExpr,
        BinaryExpr,
        TernaryExpr
    };
    ASTExpr(ExprType et);
    ~ASTExpr();
    ExprType exprType() const;
private:
    class Private;
    Private *d;
};

class ASTPrimaryExpr: public ASTExpr
{
    Q_OBJECT
public:
    enum PrimaryType {
        Identifier,
        Constant,
        StringLiteral
    };
    ASTPrimaryExpr(PrimaryType ptype);
    ~ASTPrimaryExpr();
    PrimaryType primaryType() const;
private:
    class Private;
    Private *d;
};

class ASTUnaryExpr: public ASTExpr
{
    Q_OBJECT
public:
    enum UnaryOp {
        LogicalNot,
        AriReverse,
        BitwiseReverse
    };
    ASTUnaryExpr(UnaryOp op, ASTExpr *expr);
    ~ASTUnaryExpr();
    UnaryOp op() const;
    ASTExpr *expr() const;
    ASTExprType *castType() const;
    void setCastType(ASTExprType *t);
private:
    class Private;
    Private *d;
};

class ASTBinaryExpr: public ASTExpr
{
    Q_OBJECT
public:
    enum BinaryOp {
        LogicalAnd,
        LogicalOr,
        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        BitwiseLShift,
        BitwiseRShift,
        RelaEqual,
        RelaInequal,
        RelaLess,
        RelaLessEqual,
        RelaGreater,
        RelaGreaterEqual,
        AriPlus,
        AriMinus,
        AriMult,
        AriDiv,
        AriMod
    };
    ASTBinaryExpr(BinaryOp op, ASTExpr *le, ASTExpr *re);
    ~ASTBinaryExpr();
    BinaryOp op() const;
    ASTExpr *leftExpr() const;
    ASTExpr *rightExpr() const;
private:
    class Private;
    Private *d;
};

class ASTTernaryExpr: public ASTExpr
{
    Q_OBJECT
public:
    enum TernaryOp {
        Conditional
    };
    ASTTernaryExpr(TernaryOp op, ASTExpr *e1, ASTExpr *e2, ASTExpr *e3);
    ~ASTTernaryExpr();
    TernaryOp op() const;
    ASTExpr *firstExpr() const;
    ASTExpr *secondExpr() const;
    ASTExpr *thirdExpr() const;
private:
    class Private;
    Private *d;
};

ASTNode *CreateUnaryExpr(ASTUnaryExpr::UnaryOp op, ASTExpr *expr);
ASTNode *CreateBinaryExpr(ASTBinaryExpr::BinaryOp op, ASTExpr *left, ASTExpr *right);
ASTNode *CreateTernaryExpr(ASTTernaryExpr::TernaryOp op, ASTExpr *first, ASTExpr *second,
                           ASTExpr *third);

#endif // ASTEXPR_H
