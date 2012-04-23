#include "ppastexpr.h"
#include "ppast.h"

namespace PP {

ASTExpr::ASTExpr(ExprType et)
    : ASTNode(EXPR, "Expression")
{
}

}

#include "moc_ppastexpr.cpp"
