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
    int ret;
    if (ctx->rootCollection.contains(fname)) {
        ctx->root = ctx->rootCollection.value(fname);
        return 0;
    }
    pplexer.setFileName(fname);
    ctx->langDialect = Context::PP;
    ret = parse(ctx, &pplexer);
    if (ret == 0) {
        ctx->rootCollection.insert(fname, ctx->root);
    }
    return ret;
}

int parse(Context *ctx, Lexer *lexer)
{
    ctx->lexer = lexer;
    ctx->parseStart = true;
    ctx->root = NULL;
    return combineparse(ctx);
}
