QT += core concurrent

TARGET = PackageGenerator
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    PackageGenerator.cpp

HEADERS += \
    PackageGenerator.h

