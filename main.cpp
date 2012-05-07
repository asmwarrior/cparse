#include "parser.h"
#include "ast.h"
#include "context.h"
#include "dumpvisitor.h"
#include "evalvisitor.h"
#include "includepath.h"
#include <stdio.h>
#include <QCoreApplication>
#include <QFileInfo>
#include <QStringList>

extern int combinedebug;
int main(int argc, char *argv[])
{
    combinedebug = 0;
    QCoreApplication app(argc, argv);
    IncludePath::init();
    Context ctx;
    DumpVisitor dumper(&ctx);
    EvalVisitor evaluator(&ctx);
    QStringList args, files;
    args = app.arguments();
    args.removeFirst();
    foreach (QString arg, args) {
        if (!arg.startsWith("-"))
            files << arg;
    }

    foreach (QString file, files) {
        parseFile(&ctx, file);
        if (ctx.root) {
            puts("==================");
            ctx.root->accept(&dumper);
            puts("==================");
            ctx.root->accept(&evaluator);
        }
    }
}
