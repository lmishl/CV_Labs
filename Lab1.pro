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
    maskfactory.cpp

HEADERS += \
    image.h \
    mask.h \
    maskfactory.h
