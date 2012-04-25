#ifndef CONTEXT_H
#define CONTEXT_H

#include "ast.h"
#include <QString>
#include <QMap>

class Lexer;
class Context
{
public:
    Context();
    ASTNode *root;
    QMap<QString, ASTNode *> symtab;
    Lexer *lexer;
};

#endif // CONTEXT_H
