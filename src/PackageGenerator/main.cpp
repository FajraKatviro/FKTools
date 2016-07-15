#include <QCoreApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStringList>
#include <QThread>

#include "PackageGenerator.h"

bool copyRecursive(const QDir& source,const QDir& target){
    auto list=source.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for(auto i=list.constBegin();i!=list.constEnd();++i){
        QString sourceItem(source.filePath(*i));
        QString targetItem(target.filePath(*i));
        if(QFileInfo(sourceItem).isFile()){
            QFile oldFile(targetItem);
            if(oldFile.exists()){
                oldFile.remove();
            }
            if(!QFile::copy(sourceItem,targetItem)){
                return false;
            }
        }else{
            QDir oldDir(targetItem);
            if(oldDir.exists()){
                oldDir.removeRecursively();
            }
            if(!target.mkdir(*i)){
                return false;
            }
            return copyRecursive(QDir(sourceItem),QDir(targetItem));
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName("Imageset package generator");
    a.setApplicationVersion("1.2");

    const qint32 delay=3000;

    QCommandLineParser parser;
    parser.setApplicationDescription("This application proccess package.json file and build resources");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption removeImages("clean","Clean resource build folder");
    QCommandLineOption addImages("add","Add and process (rescale & crop) missing images");
    QCommandLineOption qrc("qrc","Create package.qrc files");
    QCommandLineOption rcc("rcc","Create binary resources (run Qt resource compiler)");
    QCommandLineOption dir("dir","Run for nested folders (assume provided location of multiple packages)");
    QCommandLineOption runningDelay("d",QString("Wait %1 ms before execution").arg(QString::number(delay)));
    parser.addOption(removeImages);
    parser.addOption(addImages);
    parser.addOption(qrc);
    parser.addOption(rcc);
    parser.addOption(dir);
    parser.addOption(runningDelay);

    parser.addPositionalArgument("source","Project folder containing source images");
    parser.addPositionalArgument("target","Build folder containing resource build files");
    parser.addPositionalArgument("[deployFolder]","Folder to copy target files");

    parser.process(a);

    if(parser.isSet(runningDelay)){
        QThread::msleep(delay);
    }

    QStringList arguments(parser.positionalArguments());

    if(arguments.size()<2 || arguments.size()>3){
        return 1;
    }

    QString source(arguments.at(0));
    QString target(arguments.at(1));

    QString deployFolder;
    if(arguments.size()>2){
        deployFolder=arguments.at(2);
    }

    QStringList subfolders;
    if(parser.isSet(dir)){
        QDir sourceDir(source);
        subfolders=sourceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    }else{
        subfolders<<".";
    }

    foreach(QString subfolder,subfolders){
        PackageGenerator generator(source+"/"+subfolder,target+"/img/"+subfolder);
        if(!generator.readSetting()){
            return 2;
        }

        if(parser.isSet(addImages)){
            if(!generator.cleanImages(!parser.isSet(removeImages))){
                return 3;
            }
            if(!generator.addImages()){
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
    }

    if(!deployFolder.isEmpty()){
        QDir deployDir(deployFolder);
        if(!deployDir.exists()){
            if(!deployDir.mkpath(".")){
                return 7;
            }
        }
        if(!copyRecursive(QDir(target+"/bin/"),QDir(deployFolder))){

            return 8;
        }
    }

    return 0;
}

