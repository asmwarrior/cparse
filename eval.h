#ifndef EVAL_H
#define EVAL_H

class ASTInteger;
class ASTExpr;
class ASTPrimaryExpr;
class ASTUnaryExpr;
class ASTBinaryExpr;
class ASTTernaryExpr;

ASTInteger *evalPrimaryExpr(ASTPrimaryExpr *expr);
ASTInteger *evalUnaryExpr(ASTUnaryExpr *expr);
ASTInteger *evalBinaryExpr(ASTBinaryExpr *expr);
ASTInteger *evalTernaryExpr(ASTTernaryExpr *expr);
ASTInteger *evalExpr(ASTExpr *expr);

#endif // EVAL_H
