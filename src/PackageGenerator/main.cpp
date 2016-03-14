#include <QCoreApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStringList>
#include <QThread>

#include "PackageGenerator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName("Imageset package generator");
    a.setApplicationVersion("1.0");

    const qint32 delay=3000;

    QCommandLineParser parser;
    parser.setApplicationDescription("This application proccess package.json file and build resources");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption removeImages("clean","Clean resource build folder");
    QCommandLineOption addImages("add","Add and process (rescale & crop) missing images");
    QCommandLineOption qrc("qrc","Create package.qrc files");
    QCommandLineOption rcc("rcc","Create binary resources (run Qt resource compiler)");
    QCommandLineOption runningDelay("d",QString("Wait %1 ms before execution").arg(QString::number(delay)));
    parser.addOption(removeImages);
    parser.addOption(addImages);
    parser.addOption(qrc);
    parser.addOption(rcc);
    parser.addOption(runningDelay);

    parser.addPositionalArgument("source","Project folder containing source images");
    parser.addPositionalArgument("target","Build folder containing resource build files");

    parser.process(a);

    if(parser.isSet(runningDelay)){
        QThread::msleep(delay);
    }

    QStringList arguments(parser.positionalArguments());
    if(arguments.size()!=2){
        return 1;
    }

    PackageGenerator generator(arguments.at(0),arguments.at(1));
    if(!generator.readSetting()){
        return 2;
    }

    if(parser.isSet(addImages)){
        if(!generator.cleanImages(!parser.isSet(removeImages))){
            return 3;
        }
        if(!generator.syncImages()){
            return 4;
        }
    }else if(parser.isSet(removeImages)){
        if(!generator.cleanImages(false)){
            return 3;
        }
    }

    if(parser.isSet(qrc)){
        if(!generator.buildQRC()){
            return 5;
        }
    }

    if(parser.isSet(rcc)){
        if(!generator.buildRCC()){
            return 6;
        }
    }

    return 0;
}

