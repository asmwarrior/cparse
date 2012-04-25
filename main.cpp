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
    QCoreApplication a(argc, argv);
    FILE *f;
    Context ctx;
    DumpVisitor visitor(&ctx);
    if (argc > 1)
        f = fopen(argv[1], "r");
    else
        f = stdin;
    parseFile(&ctx.root, f);
    if (ctx.root)
        ctx.root->accept(&visitor);
}
