#ifndef PP_H
#define PP_H

#include <stdio.h>

namespace PP {
class ASTNode;
int parseString(ASTNode **proot, const char *str);
int parseFile(ASTNode **proot, FILE *f);
}

#endif // PP_H
