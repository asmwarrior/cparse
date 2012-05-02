#ifndef ASTCONSTANT_H
#define ASTCONSTANT_H

#include "astexpr.h"
#include <QChar>
#include <QObject>
#include <QMap>

class ASTConstant: public ASTPrimaryExpr
{
    Q_OBJECT
public:
    enum ConstantType {
        Integer,
        Float,
        Char,
        Enum
    };
    ASTConstant(ConstantType t);
    ~ASTConstant();
    ConstantType constantType() const;
private:
    class Private;
    Private *d;
};

class ASTInteger: public ASTConstant
{
    Q_OBJECT
public:
    enum IntegerType {
        Int,
        LongInt,
        LongLongInt,
        UnsignedInt,
        UnsignedLongInt,
        UnsignedLongLongInt
    };
    explicit ASTInteger(int i);
    explicit ASTInteger(long li);
    explicit ASTInteger(long long lli);
    explicit ASTInteger(unsigned i);
    explicit ASTInteger(unsigned long li);
    explicit ASTInteger(unsigned long long lli);
    ~ASTInteger();
    ASTInteger(const ASTInteger &integer);
    IntegerType integerType();
    int asInt() const;
    long asLInt() const;
    long long asLLInt() const;
    unsigned asUInt() const;
    unsigned long asULInt() const;
    unsigned long long asULLInt() const;
    bool isZero() const;
    void promoteTo(ASTInteger::IntegerType itype);
private:
    class Private;
    Private *d;
};

class ASTChar: public ASTConstant
{
    Q_OBJECT
public:
    ASTChar(const QChar &ch);
    ~ASTChar();
    QChar qchar() const;
    bool isWide() const;
    void setWide(bool w);
    int asInt() const;
private:
    class Private;
    Private *d;
};

ASTNode *CreateConstant(ASTPPToken *token);
ASTNode *CreateIntegerOrFloat(const QString &str);
ASTNode *CreateInteger(const QString &str);
ASTNode *CreateFloat(const QString &str);
ASTNode *CreateChar(const QString &str);

#endif // ASTCONSTANT_H
