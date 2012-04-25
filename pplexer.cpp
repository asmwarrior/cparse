#include "pplexer.h"
#define YY_DECL int pplex(ASTNode *&pplval)
#include "pp_lex.h"
#include "ast.h"
#include <QFile>
#include <QByteArray>

extern int pplex(ASTNode *&pplval);

class PPLexer::Private
{
public:
    Private() : buf(NULL) {}
    void releaseBuffer();
    YY_BUFFER_STATE buf;
};

void PPLexer::Private::releaseBuffer()
{
    if (buf) {
        pp_delete_buffer(buf);
        buf = NULL;
    }
}

PPLexer::PPLexer()
    : Lexer(),
      d(new PPLexer::Private)
{
}

PPLexer::~PPLexer()
{
    d->releaseBuffer();
    delete d;
}

void PPLexer::setFileName(const QString &fname)
{
    QFile f(fname);

    d->releaseBuffer();
    if (!f.open(QIODevice::ReadOnly))
        return;
    setString(f.readAll().constData());
}

void PPLexer::setString(const char *str)
{
    d->releaseBuffer();
    d->buf = pp_scan_string(str);
    pp_switch_to_buffer(d->buf);
}

Token PPLexer::lex()
{
    if (!d->buf)
        return Token();

    Token tok;
    tok.type = pplex((ASTNode*&)tok.value);
    return tok;
}
