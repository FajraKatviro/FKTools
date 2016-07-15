#include <QCoreApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QThread>

#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFileInfo>
#include <QDir>

bool copyRecursive(const QString& base, const QString& path, const QString& target, const QString& plugin){
    QDir baseDir(base);
    QDir targetDir(target);
    QDir sourceDir(path);
    QDir destDir(targetDir.absoluteFilePath(baseDir.relativeFilePath(path)));
    destDir.mkpath(".");
    QFileInfoList list(sourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot));
    foreach(QFileInfo info,list){
        if(info.isFile()){
            if(info.suffix()!="so" || info.baseName()==plugin){
                QFile f(info.absoluteFilePath());
                QString targetPath(destDir.absoluteFilePath(info.fileName()));
                QFile t(targetPath);
                if(t.exists()){
                    if(!t.remove()){
                        return false;
                    }
                }
                qDebug("copy %s to %s",f.fileName().toLatin1().constData(),targetPath.toLatin1().constData());
                if(!f.copy(targetPath)){
                    return false;
                }
            }
        }else{
            if(!copyRecursive(base,info.absoluteFilePath(),target,plugin)){
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName("Deploy-QML tool");
    a.setApplicationVersion("1.0");

    const qint32 delay=3000;

    QCommandLineParser parser;
    parser.setApplicationDescription("This application designed to copy qml dependencies instead of missing qt deploy tool for x11 platforms");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption qmlArg("qml","Path to qt qml folder","path");
    QCommandLineOption jsonArg("json","File with qmlimportscanner output","file");
    QCommandLineOption runningDelay("d",QString("Wait %1 ms before execution").arg(QString::number(delay)));
    parser.addOption(qmlArg);
    parser.addOption(jsonArg);
    parser.addOption(runningDelay);

    parser.addPositionalArgument("target","Target deploy folder");

    parser.process(a);

    if(parser.isSet(runningDelay)){
        QThread::msleep(delay);
    }

    QStringList arguments(parser.positionalArguments());

    if(arguments.size()!=1){
        return 1;
    }

    if(!parser.isSet(qmlArg) || !parser.isSet(jsonArg)){
        return 2;
    }

    QFile jsonFile(parser.value(jsonArg));
    if(!jsonFile.open(QIODevice::ReadOnly)){
        return 3;
    }

    QString target(arguments.at(0)), base(parser.value(qmlArg));

    QByteArray data(jsonFile.readAll());

    QJsonArray array(QJsonDocument::fromJson(data).array());
    qint32 size=array.size();
    qDebug("copy QML dependencies");
    for(qint32 i=0;i<size;++i){
        QJsonObject obj(array.at(i).toObject());
        QString path(obj.value("path").toString());
        if(!path.isEmpty() && obj.value("type").toString()=="module"){
            QString plugin(obj.value("plugin").toString());
            if(!copyRecursive(base,path,target,"lib"+plugin)){
                qDebug("Unable copy required files");
                return 4;
            }
        }
    }

    return 0;
}
