#include "textutil.h"
#include <QByteArray>
#include <QDebug>

QByteArray filterTrigraph(const QByteArray &bytes)
{
    QByteArray buf = bytes;
    buf.replace("\?\?=", "#");
    buf.replace("\?\?)", "]");
    buf.replace("\?\?!", "|");
    buf.replace("\?\?(", "[");
    buf.replace("\?\?'", "^");
    buf.replace("\?\?>", "}");
    buf.replace("\?\?/", "\\");
    buf.replace("\?\?<", "{");
    buf.replace("\?\?-", "~");
    return buf;
}

QByteArray filterJoinPartialLine(const QByteArray &bytes)
{
    QByteArray buf = bytes;
    buf.replace("\\\n", "");
    return buf;
}

QByteArray filterAddTrailingNewline(const QByteArray &bytes)
{
    QByteArray buf = bytes;
    if (!buf.endsWith("\n")) {
        buf.append("\n");
    }
    return buf;
}
