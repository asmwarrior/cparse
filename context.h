#ifndef CONTEXT_H
#define CONTEXT_H

#include "ast.h"
#include <QString>
#include <QMap>

class Lexer;
class Context
{
public:
    enum LangDialect {
        None,
        PP,
        PPExpression
    };
    Context();
    ASTNode *root;
    QMap<QString, ASTNode *> symtab;
    Lexer *lexer;
    LangDialect langDialect;
    bool parseStart;

};

#endif // CONTEXT_H
