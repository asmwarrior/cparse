#include "parser.h"
#include "ast.h"
#include "context.h"
#include "dumpvisitor.h"
#include <QtCore/QCoreApplication>
#include <stdio.h>

extern int combinedebug;
int main(int argc, char *argv[])
{
    combinedebug = 1;
    Context ctx;
    DumpVisitor visitor(&ctx);
    if (argc > 1)
        parseFile(&ctx, argv[1]);
    if (ctx.root)
        ctx.root->accept(&visitor);
}
