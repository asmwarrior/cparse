#include "ppastvisitor.h"
#include "ppcontext.h"

namespace PP {

class ASTVisitor::Private
{
public:
    Context *context;
};

ASTVisitor::ASTVisitor(Context *ctx)
    : QObject(NULL),
      d(new ASTVisitor::Private)
{
    d->context = ctx;
}

Context *ASTVisitor::context()
{
    return d->context;
}

}

#include "moc_ppastvisitor.cpp"
