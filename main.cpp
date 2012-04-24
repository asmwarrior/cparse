#include "pp.h"
#include "ppast.h"
#include "ppcontext.h"
#include "ppdumpvisitor.h"
#include <QtCore/QCoreApplication>
#include <stdio.h>

extern int ppdebug;
int main(int argc, char *argv[])
{
    ppdebug = 1;
    QCoreApplication a(argc, argv);
    FILE *f;
    PP::Context ctx;
    PP::DumpVisitor visitor(&ctx);
    if (argc > 1)
        f = fopen(argv[1], "r");
    else
        f = stdin;
    PP::parseFile(&ctx.root, f);
    if (ctx.root)
        ctx.root->accept(&visitor);
}
