#ifndef CONTEXT_H
#define CONTEXT_H

#include "ast.h"
#include <QString>
#include <QMap>

class Context
{
public:
    Context();
    ASTNode *root;
    QMap<QString, ASTNode *> symtab;
};

#endif // CONTEXT_H
