#ifndef PPASTEXPR_H
#define PPASTEXPR_H

namespace PP {
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
    ExprType exprType() const;
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
        FuncInvoke
    };
    ASTUnaryExpr(UnaryOp op, ASTExpr *expr);
    UnaryOp op() const;
    ASTExpr *expr() const;
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
        Comma
    };
    ASTBinaryExpr(BinaryOp op, ASTExpr *le, ASTExpr *re);
    BinaryOp op() const;
    ASTExpr *leftExpr() const;
    ASTExpr *rightExpr() const;
};

class ASTTernaryExpr: public ASTExpr
{
    Q_OBJECT
public:
    enum TernaryOp {
        Conditional
    };
    ASTTernaryExpr(TernaryOp op, ASTExpr *e1, ASTExpr *e2, ASTExpr *e3);
    TernaryOp op() const;
    ASTExpr *firstExpr() const;
    ASTExpr *secondExpr() const;
    ASTExpr *thirdExpr() const;
};

}

#endif // PPASTEXPR_H
