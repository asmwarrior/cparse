#include <QtCore/QCoreApplication>

extern int ppparse();
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    return ppparse();
}
