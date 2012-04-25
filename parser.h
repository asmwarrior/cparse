#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

class ASTNode;
int parseString(ASTNode **proot, const char *str);
int parseFile(ASTNode **proot, FILE *f);

#endif // PARSER_H
