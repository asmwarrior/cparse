#ifndef PPCONTEXT_H
#define PPCONTEXT_H

#include "ppast.h"
#include <QString>
#include <QMap>
namespace PP {
class Context
{
public:
    Context();
    ASTNode *root;
    QMap<QString, ASTNode *> symtab;
};
}

#endif // PPCONTEXT_H
