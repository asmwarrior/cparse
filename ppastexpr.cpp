#include "ppastexpr.h"
#include "ppast.h"

namespace PP {

ASTExpr::ASTExpr(ExprType et)
    : ASTNode(ASTNode::Expr, "Expression")
{
}

}

#include "moc_ppastexpr.cpp"
