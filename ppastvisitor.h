#ifndef PPASTVISITOR_H
#define PPASTVISITOR_H

#include <QObject>

namespace PP {
class Context;
class ASTVisitor: public QObject
{
    Q_OBJECT
public:
    ASTVisitor(Context *ctx);

    Context *context();

private:
    class Private;
    Private *d;
};
}


#endif // PPASTVISITOR_H
