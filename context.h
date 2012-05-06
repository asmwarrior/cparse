#ifndef CONTEXT_H
#define CONTEXT_H

#include "ast.h"
#include <QString>
#include <QMap>

typedef QMap<QString, ASTNode *> SymbolTable;
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
    QMap<QString, ASTNode *> rootCollection;
    SymbolTable symtab;
    Lexer *lexer;
    LangDialect langDialect;
    int includeDepth;
    bool parseStart;
};

#endif // CONTEXT_H

