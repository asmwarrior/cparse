#ifndef PP_H
#define PP_H

#include <stdio.h>

namespace PP {
int parseString(const char *str);
int parseFile(FILE *f);
}

#endif // PP_H
