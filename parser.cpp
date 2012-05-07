#include "parser.h"
#include "context.h"
#include "pplexer.h"
#include "combine_yacc.h"
#include "textutil.h"
#include <stdio.h>
#include <QFile>
#include <QByteArray>
#include <QDebug>

extern int combineparse(Context *ctx);

int parseString(Context *ctx, const char *str)
{
    PPLexer pplexer;
    QByteArray buf(str);
    buf = filterTrigraph(buf);
    buf = filterJoinPartialLine(buf);
    buf = filterAddTrailingNewline(buf);
    pplexer.setString(buf.constData());
    return parse(ctx, &pplexer);
}

int parseFile(Context *ctx, const QString &fname)
{
    PPLexer pplexer;
    QFile f;
    QByteArray buf;
    int ret;
    if (ctx->rootCollection.contains(fname)) {
        ctx->root = ctx->rootCollection.value(fname);
        return 0;
    }
    f.setFileName(fname);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Unable to open file " << fname;
        return -1;
    }
    buf = f.readAll();
    f.close();
    buf = filterTrigraph(buf);
    buf = filterJoinPartialLine(buf);
    buf = filterAddTrailingNewline(buf);
    pplexer.setString(buf.constData());
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
