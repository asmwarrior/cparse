#include "parser.h"
#include "ast.h"
#include "context.h"
#include "dumpvisitor.h"
#include "evalvisitor.h"
#include <QtCore/QCoreApplication>
#include <stdio.h>

extern int combinedebug;
int main(int argc, char *argv[])
{
    combinedebug = 1;
    Context ctx;
    DumpVisitor dumper(&ctx);
    EvalVisitor evaluator(&ctx);
    if (argc > 1)
        parseFile(&ctx, argv[1]);
    if (ctx.root) {
        puts("==================");
        ctx.root->accept(&dumper);
        puts("==================");
        ctx.root->accept(&evaluator);
    }
}
