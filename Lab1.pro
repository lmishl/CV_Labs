QT += core
QT += gui

CONFIG += c++14

TARGET = Lab1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    image.cpp \
    mask.cpp \
    maskfactory.cpp \
    pyramid.cpp \
    pyramidlevel.cpp \
    descriptor.cpp \
    descriptorfactory.cpp \
    utils.cpp \
    transformation.cpp

HEADERS += \
    image.h \
    mask.h \
    maskfactory.h \
    pyramid.h \
    pyramidlevel.h \
    descriptor.h \
    descriptorfactory.h \
    utils.h \
    transformation.h




unix|win32: LIBS += -L$$PWD/../../../../../../GnuWin32/lib/ -lgsl

INCLUDEPATH += $$PWD/../../../../../../GnuWin32/include
DEPENDPATH += $$PWD/../../../../../../GnuWin32/include
