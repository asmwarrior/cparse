#ifndef PPTOKENLISTLEXER_H
#define PPTOKENLISTLEXER_H

#include "lexer.h"
#include "ast.h"
#include <QList>

class PPTokenListLexer : public Lexer
{
public:
    PPTokenListLexer();
    ~PPTokenListLexer();
    void setPPTokenList(const QList<ASTPPToken*> &tokenList);
    Token lex();
private:
    class Private;
    Private *d;
};

#endif // PPTOKENLISTLEXER_H
