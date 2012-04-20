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
    pp.y

QMAKE_YACCFLAGS = -t -d --report=all

SOURCES += main.cpp \
    ppast.cpp \
    pp.cpp \
    ppcontext.cpp

HEADERS += \
    ppast.h \
    pp.h \
    ppcontext.h


