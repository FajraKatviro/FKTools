#include <QCoreApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStringList>
#include <QThread>

#include "PackageManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName("Imageset map generator");
    a.setApplicationVersion("1.0");

    const qint32 delay=3000;

    QCommandLineParser parser;
    parser.setApplicationDescription("This application look throw target folder and creates or refreshes .json file for package builder");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption addSizeset("a","Add sizeset","size");
    QCommandLineOption removeSizeset("r","Remove sizeset","size");
    QCommandLineOption runningDelay("d",QString("Wait %1 ms before execution").arg(QString::number(delay)));
    parser.addOption(addSizeset);
    parser.addOption(removeSizeset);
    parser.addOption(runningDelay);

    parser.addPositionalArgument("target","Target folder containing source images");

    parser.process(a);

    if(parser.isSet(runningDelay)){
        QThread::msleep(delay);
    }

    QStringList arguments(parser.positionalArguments());
    if(arguments.size()!=1){
        return 1;
    }

    PackageManager package(arguments.at(0));
    if(!package.readData()){
        return 2;
    }

    QStringList sizesToRemove = parser.values(removeSizeset);
    if(!sizesToRemove.isEmpty()){
        foreach(QString size,sizesToRemove)package.removeSizeset(size);
    }
    QStringList sizesToAdd    = parser.values(addSizeset);
    if(!sizesToAdd.isEmpty()){
        foreach(QString size,sizesToAdd)package.addSizeset(size);
    }

    package.refreshPackage();

    if(!package.writeData()){
        return 3;
    }

    return 0;
}

