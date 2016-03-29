QT += core concurrent

CONFIG += c++11

TARGET = PackageGenerator
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    PackageGenerator.cpp

HEADERS += \
    PackageGenerator.h

