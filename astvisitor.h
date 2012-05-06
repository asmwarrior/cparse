#ifndef ASTVISITOR_H
#define ASTVISITOR_H

#include <QObject>

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


#endif // ASTVISITOR_H

