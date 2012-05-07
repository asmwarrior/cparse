#ifndef INCLUDEPATH_H
#define INCLUDEPATH_H

#include <QString>

class IncludePath
{
public:
    ~IncludePath();
    static void init();
    static IncludePath *instance();
    void add(const QString &dirName);
    void remove(const QString &dirName);
    bool contains(const QString &dirName) const;
    QString find(const QString &baseName) const;
    void save();
    void restore();
private:
    IncludePath();
    IncludePath(const IncludePath &);
    class Private;
    Private *d;
};

#endif // INCLUDEPATH_H
