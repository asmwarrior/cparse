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
    ctx->lexer = &pplexer;
    return combineparse(ctx);
}

int parseFile(Context *ctx, const QString &fname)
{
    PPLexer pplexer;
    pplexer.setFileName(fname);
    ctx->lexer = &pplexer;
    return combineparse(ctx);
}
