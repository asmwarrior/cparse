#include "lexer.h"

Token::Token(int t, void *v)
{
    type = t;
    value = v;
}

Token::Token(const Token &tok)
{
    type = tok.type;
    value = tok.value;
}

Token &Token::operator =(const Token &tok)
{
    type = tok.type;
    value = tok.value;
    return *this;
}

bool Token::operator ==(const Token &tok) const
{
    return (type == tok.type) && (value == tok.value);
}

bool Token::operator !=(const Token &tok) const
{
    return !(*this == tok);
}
