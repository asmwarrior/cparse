#include "escapeseq.h"
#include <QRegExp>
#include <QString>

static QMap<QString, QString> escapeMap;

const QMap<QString, QString> &EscapeSequence::SimpleMap()
{
    static bool init = false;
    if (!init) {
        escapeMap.clear();
        escapeMap.insert(QString("\\'"), QString("'"));
        escapeMap.insert(QString("\\\""), QString("\""));
        escapeMap.insert(QString("\\\?"), QString("?"));
        escapeMap.insert(QString("\\\\"), QString("\\"));
        escapeMap.insert(QString("\\a"), QString("\a"));
        escapeMap.insert(QString("\\b"), QString("\b"));
        escapeMap.insert(QString("\\f"), QString("\f"));
        escapeMap.insert(QString("\\n"), QString("\n"));
        escapeMap.insert(QString("\\r"), QString("\r"));
        escapeMap.insert(QString("\\t"), QString("\t"));
        escapeMap.insert(QString("\\v"), QString("\v"));
        init = true;
    }
    return escapeMap;
}

QString EscapeSequence::Unescape(const QString &str, bool wide)
{
    QString s = str;
    QChar ch;
    int pos = 0;
    bool ok;
    QRegExp sRE(QRegExp::escape("\\\\('|\\\"|\\\?|\\\\|a|b|f|n|r|t|v"));
    QRegExp oRE(QRegExp::escape("\\\\([0-7]{1,3})"));
    QRegExp hRE(QRegExp::escape("\\\\x([0-9a-fA-f]+)"));
    QRegExp ucnRE(QRegExp::escape("\\\\u([0-9a-fA-f]{4})|\\\\U([0-9a-fA-f]{8}"));
    while ((pos = s.indexOf('\\', pos)) != -1) {
        if (sRE.indexIn(s, pos) == pos) {
            s.replace(pos, sRE.cap().length(), SimpleMap().value(sRE.cap()));
            goto loopNext;
        }
        if (oRE.indexIn(s, pos) == pos) {
            if (wide)
                ch = QChar(oRE.cap(1).toUShort(&ok, 8));
            else
                ch = QChar::fromAscii(oRE.cap(1).toUShort(&ok, 8));
            if (!ok)
                return QString();
            s.replace(pos, oRE.cap().length(), QString(ch));
            ++pos;
            goto loopNext;
        }
        if (hRE.indexIn(s, pos) == pos) {
            if (wide)
                ch = QChar(hRE.cap(1).toUShort(&ok, 16));
            else
                ch = QChar::fromAscii(hRE.cap(1).toUShort(&ok, 16));
            if (!ok)
                return QString();
            s.replace(pos, hRE.cap().length(), QString(ch));
            ++pos;
            goto loopNext;
        }
        if (ucnRE.indexIn(s, pos) == pos) {
            ch = QChar(ucnRE.cap(1).toUInt(&ok, 16));
            if (!ok)
                return QString();
            s.replace(pos, ucnRE.cap().length(), QString(ch));
            ++pos;
            goto loopNext;
        }
loopNext:
        ;
    }
    return s;
}

