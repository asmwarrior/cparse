#include "parser.h"
#include "context.h"
#include "pplexer.h"
#include "combine_yacc.h"
#include <stdio.h>

extern int combineparse(Context *ctx);

int parseString(Context *ctx, const char *str)
{
    PPLexer pplexer;
    pplexer.setString(str);
    return parse(ctx, &pplexer);
}

int parseFile(Context *ctx, const QString &fname)
{
    PPLexer pplexer;
    pplexer.setFileName(fname);
    return parse(ctx, &pplexer);
}

int parse(Context *ctx, Lexer *lexer)
{
    ctx->lexer = lexer;
    ctx->parseStart = true;
    return combineparse(ctx);
}
