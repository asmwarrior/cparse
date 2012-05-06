#ifndef LEXER_H
#define LEXER_H
#include <stddef.h>

class Token
{
public:
    Token(int t = 0, void *v = NULL);
    Token(const Token &tok);
    Token &operator =(const Token &tok);
    bool operator ==(const Token &tok) const;
    bool operator !=(const Token &tok) const;
    int type;
    void *value;
};

class Lexer
{
public:
    virtual Token lex() = 0;
};

#endif // LEXER_H

