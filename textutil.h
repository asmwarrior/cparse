#ifndef TEXTUTIL_H
#define TEXTUTIL_H

#include <QByteArray>

QByteArray filterTrigraph(const QByteArray &bytes);
QByteArray filterJoinPartialLine(const QByteArray &bytes);
QByteArray filterAddTrailingNewline(const QByteArray &bytes);

#endif // TEXTUTIL_H
