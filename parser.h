#ifndef PARSER_H
#define PARSER_H

#include <QString>

class Context;
int parseString(Context *ctx, const char *str);
int parseFile(Context *ctx, const QString &fname);

#endif // PARSER_H
