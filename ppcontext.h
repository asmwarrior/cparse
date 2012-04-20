#ifndef PPCONTEXT_H
#define PPCONTEXT_H

#include "ppast.h"
namespace PP {
class Context
{
public:
    Context();
    ASTNode *root;
};
}

#endif // PPCONTEXT_H
