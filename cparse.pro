#-------------------------------------------------
#
# Project created by QtCreator 2012-04-17T10:49:09
#
#-------------------------------------------------

QT       -= gui

TARGET = cparse
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LEXSOURCES += \
    pp.l

YACCSOURCES += \
    combine.y

QMAKE_YACCFLAGS = -t -d --report=all

SOURCES += main.cpp \
    ast.cpp \
    parser.cpp \
    context.cpp \
    astvisitor.cpp \
    dumpvisitor.cpp \
    evalvisitor.cpp \
    astexpr.cpp \
    astconstant.cpp \
    escapeseq.cpp

HEADERS += \
    ast.h \
    parser.h \
    context.h \
    astvisitor.h \
    dumpvisitor.h \
    evalvisitor.h \
    astexpr.h \
    astconstant.h \
    escapeseq.h


