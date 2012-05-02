#include "astconstant.h"
#include "ast.h"
#include "combine_yacc.h"
#include "escapeseq.h"
#include <stdlib.h>
#include <errno.h>
#include <QChar>
#include <QDebug>

class ASTConstant::Private
{
public:
    ASTConstant::ConstantType constantType;
};

ASTConstant::ASTConstant(ASTConstant::ConstantType t)
    : ASTPrimaryExpr(ASTPrimaryExpr::Constant),
      d(new ASTConstant::Private)
{
    d->constantType = t;
}

ASTConstant::~ASTConstant()
{
    delete d;
}

ASTConstant::ConstantType ASTConstant::constantType() const
{
    return d->constantType;
}

class ASTInteger::Private
{
public:
    ASTInteger::IntegerType integerType;
    union {
        int aInt;
        long aLInt;
        long long aLLInt;
        unsigned aUInt;
        unsigned long aULInt;
        unsigned long long aULLInt;
    } value;
};

ASTInteger::ASTInteger(int i)
    : ASTConstant(ASTConstant::Integer),
      d(new ASTInteger::Private)
{
    d->integerType = ASTInteger::Int;
    d->value.aInt = i;
}

ASTInteger::ASTInteger(long li)
    : ASTConstant(ASTConstant::Integer),
      d(new ASTInteger::Private)
{
    d->integerType = ASTInteger::LongInt;
    d->value.aLInt = li;
}

ASTInteger::ASTInteger(long long lli)
    : ASTConstant(ASTConstant::Integer),
      d(new ASTInteger::Private)
{
    d->integerType = ASTInteger::LongLongInt;
    d->value.aLLInt = lli;
}

ASTInteger::ASTInteger(unsigned i)
    : ASTConstant(ASTConstant::Integer),
      d(new ASTInteger::Private)
{
    d->integerType = ASTInteger::UnsignedInt;
    d->value.aUInt = i;
}

ASTInteger::ASTInteger(unsigned long li)
    : ASTConstant(ASTConstant::Integer),
      d(new ASTInteger::Private)
{
    d->integerType = ASTInteger::UnsignedLongInt;
    d->value.aULInt = li;
}

ASTInteger::ASTInteger(unsigned long long lli)
    : ASTConstant(ASTConstant::Integer),
      d(new ASTInteger::Private)
{
    d->integerType = ASTInteger::UnsignedLongLongInt;
    d->value.aULLInt = lli;
}

ASTInteger::~ASTInteger()
{
    delete d;
}

ASTInteger::ASTInteger(const ASTInteger &integer)
    : ASTConstant(ASTConstant::Integer),
      d(new ASTInteger::Private)
{
    d->integerType = integer.d->integerType;
    qMemCopy(&d->value, &integer.d->value, sizeof(d->value));
}

ASTInteger::IntegerType ASTInteger::integerType()
{
    return d->integerType;
}

int ASTInteger::asInt() const
{
    return d->value.aInt;
}

long ASTInteger::asLInt() const
{
    return d->value.aLInt;
}

long long ASTInteger::asLLInt() const
{
    return d->value.aLLInt;
}

unsigned ASTInteger::asUInt() const
{
    return d->value.aUInt;
}

unsigned long ASTInteger::asULInt() const
{
    return d->value.aULInt;
}

unsigned long long ASTInteger::asULLInt() const
{
    return d->value.aULLInt;
}

bool ASTInteger::isZero() const
{
    switch (d->integerType) {
    case ASTInteger::Int:
        return d->value.aInt == 0;
    case ASTInteger::LongInt:
        return d->value.aLInt == 0;
    case ASTInteger::LongLongInt:
        return d->value.aLLInt == 0;
    case ASTInteger::UnsignedInt:
        return d->value.aUInt == 0;
    case ASTInteger::UnsignedLongInt:
        return d->value.aULInt == 0;
    case ASTInteger::UnsignedLongLongInt:
        return d->value.aULLInt == 0;
    }
    return false;
}

class ASTChar::Private
{
public:
    QChar qchar;
    bool wide;
};

ASTChar::ASTChar(const QChar &ch)
    : ASTConstant(ASTConstant::Char),
      d(new ASTChar::Private)
{
    d->qchar = ch;
    d->wide = false;
}

ASTChar::~ASTChar()
{
    delete d;
}

QChar ASTChar::qchar() const
{
    return d->qchar;
}

bool ASTChar::isWide() const
{
    return d->wide;
}

void ASTChar::setWide(bool w)
{
    d->wide = w;
}

int ASTChar::asInt() const
{
    if (d->wide)
        return d->qchar.unicode();
    else
        return (int)QString(d->qchar).toUtf8().at(0);
}

ASTNode *CreateConstant(ASTPPToken *token)
{
    switch (token->ppTokenType()) {
    case CHAR_CONSTANT:
        return CreateChar(token->spellName());
        break;
    case PP_NUMBER:
        return CreateIntegerOrFloat(token->spellName());
        break;
    case ID:
        qDebug() << "CreateConstant enum constant not implemented";
        break;
    }
    return NULL;
}

ASTNode *CreateIntegerOrFloat(const QString &str)
{
    if (str.contains('.') || str.contains('e') || str.contains('E')
            || str.contains('p') || str.contains('P') || str.contains('+')
            || str.contains('-') || str.contains('f') || str.contains('F'))
        return CreateFloat(str);
    else
        return CreateInteger(str);
}

ASTNode *CreateChar(const QString &str)
{
    bool wide = false;
    QString s = str;
    ASTChar *astChar;
    if (s.startsWith('L')) {
        s.remove(0,1);
        wide = true;
    }
    s.remove(0,1);
    s.chop(1);
    s = EscapeSequence::Unescape(s, wide);
    if (s.isNull())
        return NULL;
    if (s.size() > 1) {
        return NULL;
    } else {
        astChar = new ASTChar(s.at(0));
        astChar->setWide(wide);
        return astChar;
    }
    return NULL;
}

ASTNode *CreateInteger(const QString &str)
{
    QByteArray s = str.toLower().toAscii();
    ASTInteger::IntegerType itype = ASTInteger::Int;
    int base = 10;
    bool ok = true;
    if (s.endsWith("ull") || s.endsWith("llu")) {
        itype = ASTInteger::UnsignedLongLongInt;
        s.chop(3);
    } else if (s.endsWith("ul") || s.endsWith("lu")) {
        itype = ASTInteger::UnsignedLongInt;
        s.chop(2);
    } else if (s.endsWith("ll")) {
        itype = ASTInteger::LongLongInt;
        s.chop(2);
    } else if (s.endsWith("l")) {
        itype = ASTInteger::LongInt;
        s.chop(1);
    } else if (s.endsWith("u")) {
        itype = ASTInteger::UnsignedInt;
        s.chop(1);
    }
    if (s.startsWith("0x"))
        base = 16;
    else if (s.startsWith("0"))
        base = 8;
    ASTInteger *astInt;
    switch (itype) {
    case ASTInteger::Int:
        astInt = new ASTInteger(s.toInt(&ok, base));
        break;
    case ASTInteger::LongInt:
        astInt = new ASTInteger(s.toLong(&ok, base));
        break;
    case ASTInteger::LongLongInt:
        astInt = new ASTInteger(s.toLongLong(&ok, base));
        break;
    case ASTInteger::UnsignedInt:
        astInt = new ASTInteger(s.toUInt(&ok, base));
        break;
    case ASTInteger::UnsignedLongInt:
        astInt = new ASTInteger(s.toULong(&ok, base));
        break;
    case ASTInteger::UnsignedLongLongInt:
        astInt = new ASTInteger(s.toULongLong(&ok, base));
        break;
    }
    if (!ok) {
        delete astInt;
        return NULL;
    } else
        return astInt;
}

ASTNode *CreateFloat(const QString &str)
{
    qDebug() << __func__ << "not implemented.";
    Q_UNUSED(str);
    return NULL;
}
