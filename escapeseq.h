#ifndef ESCAPESEQ_H
#define ESCAPESEQ_H

#include <QString>
#include <QPair>
#include <QList>
#include <QMap>

class EscapeSequence
{
public:
    static const QMap<QString, QString> &SimpleMap();
    static QString Unescape(const QString &str, bool wide);
};

#endif // ESCAPESEQ_H
