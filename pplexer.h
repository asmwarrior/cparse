#ifndef PPLEXER_H
#define PPLEXER_H

#include "lexer.h"
#include <QString>

class PPLexer : public Lexer
{
public:
    PPLexer();
    ~PPLexer();
    void setFileName(const QString &fname);
    void setString(const char *str);
    Token lex();
private:
    class Private;
    Private *d;
};

#endif // PPLEXER_H
