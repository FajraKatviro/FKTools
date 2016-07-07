TEMPLATE = app

QT += qml quick
CONFIG += c++11

DESTDIR = $$PWD/../../bin

SOURCES += main.cpp \
    ImageChecker.cpp

RESOURCES += qml.qrc \
    deployTemplates.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

include(../deployment.pri)

HEADERS += \
    ImageChecker.h

