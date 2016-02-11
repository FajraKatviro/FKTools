#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QtQml>

#include "ImageChecker.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<ImageChecker>("imageManager",1,0,"ImageChecker");

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

