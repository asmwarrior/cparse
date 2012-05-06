#ifndef PARSER_H
#define PARSER_H

#include <QString>

class Context;
class Lexer;
int parseString(Context *ctx, const char *str);
int parseFile(Context *ctx, const QString &fname);
int parse(Context *ctx, Lexer *lexer);

#endif // PARSER_H

