#include "astexpr.h"
#include "ast.h"

class ASTExpr::Private
{
public:
    ASTExpr::ExprType exprType;
};

ASTExpr::ASTExpr(ExprType et)
    : ASTNode(ASTNode::Expr, "Expression"),
      d(new ASTExpr::Private)
{
    d->exprType = et;
}

ASTExpr::~ASTExpr()
{
    delete d;
}

ASTExpr::ExprType ASTExpr::exprType() const
{
    return d->exprType;
}

class ASTPrimaryExpr::Private
{
public:
    ASTPrimaryExpr::PrimaryType primaryType;
};

ASTPrimaryExpr::ASTPrimaryExpr(PrimaryType ptype)
    : ASTExpr(ASTExpr::PrimaryExpr),
      d(new ASTPrimaryExpr::Private)
{
    d->primaryType = ptype;
}

ASTPrimaryExpr::~ASTPrimaryExpr()
{
    delete d;
}

ASTPrimaryExpr::PrimaryType ASTPrimaryExpr::primaryType() const
{
    return d->primaryType;
}

class ASTUnaryExpr::Private
{
public:
    ASTUnaryExpr::UnaryOp op;
    ASTExpr *expr;
};

ASTUnaryExpr::ASTUnaryExpr(UnaryOp op, ASTExpr *expr)
    : ASTExpr(ASTExpr::UnaryExpr),
      d(new ASTUnaryExpr::Private)
{
    d->op = op;
    d->expr = expr;
}

ASTUnaryExpr::~ASTUnaryExpr()
{
    delete d;
}

ASTUnaryExpr::UnaryOp ASTUnaryExpr::op() const
{
    return d->op;
}

ASTExpr *ASTUnaryExpr::expr() const
{
    return d->expr;
}

class ASTBinaryExpr::Private
{
public:
    ASTBinaryExpr::BinaryOp op;
    ASTExpr *leftExpr;
    ASTExpr *rightExpr;
};

ASTBinaryExpr::ASTBinaryExpr(BinaryOp op, ASTExpr *le, ASTExpr *re)
    : ASTExpr(ASTExpr::BinaryExpr),
      d(new ASTBinaryExpr::Private)
{
    d->op = op;
    d->leftExpr = le;
    d->rightExpr = re;
}

ASTBinaryExpr::~ASTBinaryExpr()
{
    delete d;
}

ASTBinaryExpr::BinaryOp ASTBinaryExpr::op() const
{
    return d->op;
}

ASTExpr *ASTBinaryExpr::leftExpr() const
{
    return d->leftExpr;
}

ASTExpr *ASTBinaryExpr::rightExpr() const
{
    return d->rightExpr;
}

class ASTTernaryExpr::Private
{
public:
    ASTTernaryExpr::TernaryOp op;
    ASTExpr *firstExpr;
    ASTExpr *secondExpr;
    ASTExpr *thirdExpr;
};

ASTTernaryExpr::ASTTernaryExpr(TernaryOp op, ASTExpr *e1, ASTExpr *e2, ASTExpr *e3)
    : ASTExpr(ASTExpr::TernaryExpr),
      d(new ASTTernaryExpr::Private)
{
    d->op = op;
    d->firstExpr = e1;
    d->secondExpr = e2;
    d->thirdExpr = e3;
}

ASTTernaryExpr::~ASTTernaryExpr()
{
    delete d;
}

ASTTernaryExpr::TernaryOp ASTTernaryExpr::op() const
{
    return d->op;
}

ASTExpr *ASTTernaryExpr::firstExpr() const
{
    return d->firstExpr;
}

ASTExpr *ASTTernaryExpr::secondExpr() const
{
    return d->secondExpr;
}

ASTExpr *ASTTernaryExpr::thirdExpr() const
{
    return d->thirdExpr;
}
