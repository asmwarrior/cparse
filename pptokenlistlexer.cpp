#include "pptokenlistlexer.h"
#include "astconstant.h"
#include "combine_yacc.h"

class PPTokenListLexer::Private
{
public:
    Private() : i(0) {}
    QList<ASTPPToken*> tokenList;
    int i;
};

PPTokenListLexer::PPTokenListLexer()
    : Lexer(),
      d(new PPTokenListLexer::Private)
{
}

PPTokenListLexer::~PPTokenListLexer()
{
    delete d;
}

void PPTokenListLexer::setPPTokenList(const QList<ASTPPToken *> &tokenList)
{
    d->tokenList = tokenList;
    d->i = 0;
}

Token PPTokenListLexer::lex()
{
    ASTPPToken *ppToken;
    Token tok;
    if (d->i < d->tokenList.size())
         ppToken = d->tokenList.at(d->i++);
    else
        ppToken = NULL;
    if (ppToken) {
        switch (ppToken->ppTokenType()) {
        case PP_NUMBER:
        case CHAR_CONSTANT:
            tok.value = CreateConstant(ppToken);
            if (tok.value)
                tok.type = NUM;
            else
                tok.type = 0;
            break;
        default:
            tok.value = ppToken;
            tok.type = ppToken->ppTokenType();
        }
    }
    return tok;
}
