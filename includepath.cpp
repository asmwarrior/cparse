#include "includepath.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QStack>
#include <QString>

class IncludePath::Private
{
public:
    QSet<QString> dirSet;
    QStack<QSet<QString> > saved;
};

IncludePath::IncludePath()
    : d(new IncludePath::Private)
{
    d->dirSet.insert(QDir::currentPath());
    QDir::setSearchPaths("File", d->dirSet.values());
}

IncludePath::~IncludePath()
{
    delete d;
}

void IncludePath::init()
{
    volatile IncludePath *incPath = instance();
    Q_UNUSED(incPath);
}

IncludePath *IncludePath::instance()
{
    static IncludePath *incPath = NULL;
    if (!incPath) {
        incPath = new IncludePath;
        foreach (QString arg, QCoreApplication::arguments()) {
            if (arg.startsWith("-I")) {
                arg.remove(0, 2);
                if (!arg.startsWith("/"))
                    arg.prepend(QDir::currentPath() + QLatin1Char('/'));
                if (!arg.endsWith("/"))
                    arg.append("/");
                incPath->add(arg);
            }
        }
    }
    return incPath;
}

void IncludePath::add(const QString &dirName)
{
    if (d->dirSet.contains(dirName))
        return;
    QDir dir(dirName);
    if (dir.exists()) {
        d->dirSet.insert(dirName);
        QDir::setSearchPaths("File", d->dirSet.values());
    }
}

void IncludePath::remove(const QString &dirName)
{
    if (!d->dirSet.contains(dirName))
        return;
    d->dirSet.remove(dirName);
    QDir::setSearchPaths("File", d->dirSet.values());
}

bool IncludePath::contains(const QString &dirName) const
{
    return d->dirSet.contains(dirName);
}

QString IncludePath::find(const QString &baseName) const
{
    QFileInfo fi(baseName);
    if (fi.exists())
        return fi.absoluteFilePath();
    else
        return QString();
}

void IncludePath::save()
{
    d->saved.push(d->dirSet);
}

void IncludePath::restore()
{
    if (!d->saved.isEmpty())
        d->dirSet = d->saved.pop();
}
