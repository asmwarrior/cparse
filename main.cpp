#include "preprocess.h"
#include <QtCore/QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    return PP::parseFile(stdin);
}
