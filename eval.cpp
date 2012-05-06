#include "eval.h"
#include "ast.h"
#include "astexpr.h"
#include "astconstant.h"

ASTInteger *evalInteger(ASTInteger *expr)
{
    ASTInteger *i = new ASTInteger(*expr);
    return i;
}

ASTInteger *evalChar(ASTChar *expr)
{
    ASTInteger *i = new ASTInteger(expr->asInt());
    return i;
}

ASTInteger *evalConstant(ASTConstant *expr)
{
    switch (expr->constantType()) {
    case ASTConstant::Char:
        return evalChar(static_cast<ASTChar*>(expr));
    case ASTConstant::Integer:
        return evalInteger(static_cast<ASTInteger*>(expr));
    case ASTConstant::Float:
    case ASTConstant::Enum:
        break;
    }
    return NULL;
}

ASTInteger *evalPrimaryExpr(ASTPrimaryExpr *expr)
{
    switch (expr->primaryType()) {
    case ASTPrimaryExpr::Identifier:
        break;
    case ASTPrimaryExpr::Constant:
        return evalConstant(static_cast<ASTConstant*>(expr));
    case ASTPrimaryExpr::StringLiteral:
        break;
    }
    return NULL;
}

ASTInteger *evalUnaryExpr(ASTUnaryExpr *expr)
{
    ASTInteger *i = NULL, *base;
    base = evalExpr(expr->expr());
    if (!base)
        return NULL;
    switch (expr->op()) {
    case ASTUnaryExpr::LogicalNot:
        i = new ASTInteger(base->isZero() ? 1 : 0);
        break;
    case ASTUnaryExpr::AriReverse:
        switch (base->integerType()) {
        case ASTInteger::Int:
            i = new ASTInteger(-base->asInt());
            break;
        case ASTInteger::LongInt:
            i = new ASTInteger(-base->asLInt());
            break;
        case ASTInteger::LongLongInt:
            i = new ASTInteger(-base->asLLInt());
            break;
        case ASTInteger::UnsignedInt:
            i = new ASTInteger(-base->asUInt());
            break;
        case ASTInteger::UnsignedLongInt:
            i = new ASTInteger(-base->asULInt());
            break;
        case ASTInteger::UnsignedLongLongInt:
            i = new ASTInteger(-base->asULLInt());
            break;
        }
        break;
    case ASTUnaryExpr::BitwiseReverse:
        switch (base->integerType()) {
        case ASTInteger::Int:
            i = new ASTInteger(~base->asInt());
            break;
        case ASTInteger::LongInt:
            i = new ASTInteger(~base->asLInt());
            break;
        case ASTInteger::LongLongInt:
            i = new ASTInteger(~base->asLLInt());
            break;
        case ASTInteger::UnsignedInt:
            i = new ASTInteger(~base->asUInt());
            break;
        case ASTInteger::UnsignedLongInt:
            i = new ASTInteger(~base->asULInt());
            break;
        case ASTInteger::UnsignedLongLongInt:
            i = new ASTInteger(~base->asULLInt());
            break;
        }
        break;
    }
    delete base;
    return i;
}

template <typename T>
T evalBinaryNum(ASTBinaryExpr::BinaryOp op, T t1, T t2)
{
    switch (op) {
    case ASTBinaryExpr::AriPlus:
        return t1 + t2;
    case ASTBinaryExpr::AriMinus:
        return t1 - t2;
    case ASTBinaryExpr::AriMult:
        return t1 * t2;
    case ASTBinaryExpr::AriDiv:
        return t1 / t2;
    case ASTBinaryExpr::AriMod:
        return t1 % t2;
    case ASTBinaryExpr::LogicalAnd:
        return t1 && t2;
    case ASTBinaryExpr::LogicalOr:
        return t1 || t2;
    case ASTBinaryExpr::RelaEqual:
        return t1 == t2;
    case ASTBinaryExpr::RelaGreater:
        return t1 > t2;
    case ASTBinaryExpr::RelaGreaterEqual:
        return t1 >= t2;
    case ASTBinaryExpr::RelaInequal:
        return t1 != t2;
    case ASTBinaryExpr::RelaLess:
        return t1 < t2;
    case ASTBinaryExpr::RelaLessEqual:
        return t1 <= t2;
    case ASTBinaryExpr::BitwiseAnd:
        return t1 & t2;
    case ASTBinaryExpr::BitwiseOr:
        return t1 | t2;
    case ASTBinaryExpr::BitwiseXor:
        return t1 ^ t2;
    case ASTBinaryExpr::BitwiseLShift:
        return t1 << t2;
    case ASTBinaryExpr::BitwiseRShift:
        return t1 >> t2;
    }
    return T(0);
}

ASTInteger *evalBinaryExpr(ASTBinaryExpr *expr)
{
    ASTInteger *i1, *i2, *i = NULL;
    ASTInteger::IntegerType itype;
    i1 = evalExpr(expr->leftExpr());
    i2 = evalExpr(expr->rightExpr());
    if (!i1 || !i2) {
        i = NULL;
        goto out;
    }
    itype = qMax(i1->integerType(), i2->integerType());
    i1->promoteTo(itype);
    i2->promoteTo(itype);
    switch (itype) {
    case ASTInteger::Int:
        i = new ASTInteger(evalBinaryNum(expr->op(), i1->asInt(), i2->asInt()));
        break;
    case ASTInteger::LongInt:
        i = new ASTInteger(evalBinaryNum(expr->op(), i1->asInt(), i2->asInt()));
        break;
    case ASTInteger::LongLongInt:
        i = new ASTInteger(evalBinaryNum(expr->op(), i1->asInt(), i2->asInt()));
        break;
    case ASTInteger::UnsignedInt:
        i = new ASTInteger(evalBinaryNum(expr->op(), i1->asInt(), i2->asInt()));
        break;
    case ASTInteger::UnsignedLongInt:
        i = new ASTInteger(evalBinaryNum(expr->op(), i1->asInt(), i2->asInt()));
        break;
    case ASTInteger::UnsignedLongLongInt:
        i = new ASTInteger(evalBinaryNum(expr->op(), i1->asInt(), i2->asInt()));
        break;
    }
out:
    delete i1;
    delete i2;
    return i;
}

ASTInteger *evalTernaryExpr(ASTTernaryExpr *expr)
{
    ASTInteger *i1, *i2, *i3, *i;
    i1 = evalExpr(expr->firstExpr());
    i2 = evalExpr(expr->secondExpr());
    i3 = evalExpr(expr->thirdExpr());
    if (!i1 || !i2 || !i3) {
        i = NULL;
        goto out;
    }
    if (!i1->isZero())
        i = new ASTInteger(*i2);
    else
        i = new ASTInteger(*i3);
out:
    delete i1;
    delete i2;
    delete i3;
    return i;
}

ASTInteger *evalExpr(ASTExpr *expr)
{
    switch (expr->exprType()) {
    case ASTExpr::PrimaryExpr:
        return evalPrimaryExpr(static_cast<ASTPrimaryExpr*>(expr));
    case ASTExpr::UnaryExpr:
        return evalUnaryExpr(static_cast<ASTUnaryExpr*>(expr));
    case ASTExpr::BinaryExpr:
        return evalBinaryExpr(static_cast<ASTBinaryExpr*>(expr));
    case ASTExpr::TernaryExpr:
        return evalTernaryExpr(static_cast<ASTTernaryExpr*>(expr));
    }
    return NULL;
}

