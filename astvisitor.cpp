#include "astvisitor.h"
#include "context.h"

class ASTVisitor::Private
{
public:
    Private() {context=NULL;}
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
