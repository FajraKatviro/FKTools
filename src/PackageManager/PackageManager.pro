QT += core
QT -= gui

DESTDIR = $$PWD/../../bin

TARGET = PackageManager
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    PackageManager.cpp

HEADERS += \
    PackageManager.h


include(../deployment.pri)
