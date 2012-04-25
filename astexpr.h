#ifndef ASTEXPR_H
#define ASTEXPR_H

#include "ast.h"
#include <QObject>

class ASTNode;

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
    ASTPrimaryExpr();
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
        PreInc,
        PreDec,
        PostInc,
        PostDec,
        LogicalNot,
        SizeOf,
        Defined,
        Hash,
        FuncInvoke
    };
    ASTUnaryExpr(UnaryOp op, ASTExpr *expr);
    ~ASTUnaryExpr();
    UnaryOp op() const;
    ASTExpr *expr() const;
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
        BitwiseXOr,
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
        AriMod,
        Comma,
        HashHash
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

#endif // ASTEXPR_H
