#include "PackageGenerator.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QXmlStreamWriter>

#include <QProcess>
#include <QtConcurrent/QtConcurrentMap>

#include <QImage>

#include <QProcessEnvironment>

#include "sizeString.h"
#include "selectBestSizeset.h"
#include <algorithm>
#include <limits>
#include <cmath>

PackageGenerator::PackageGenerator(const QString& sourcePath, const QString& buildPath)
    :_sourceFolder(QDir::cleanPath(sourcePath)),_buildFolder(buildPath),_output(stdout){

}

bool PackageGenerator::readSetting(){
    output("Read package.json");
    QFile pkg(_sourceFolder.filePath("package.json"));
    if(pkg.open(QIODevice::ReadOnly)){
        QByteArray data(pkg.readAll());
        QJsonDocument doc=QJsonDocument::fromJson(data);
        QJsonObject json=doc.object();
        if(!json.isEmpty()){
            QJsonArray sizes=json.value("sizes").toArray();
            QJsonArray images=json.value("images").toArray();
            for(qint32 i=0;i<sizes.size();++i){
                QSize size=FKUtility::stringToSize(sizes.at(i).toString());
                _targetSizes.append(size);
            }
            for(qint32 i=0;i<images.size();++i){
                QJsonObject image=images.at(i).toObject();
                ImageSetting& setting=_imageSettings[image.value("path").toString()];
                QJsonArray imageSources=image.value("sourceSizes").toArray();
                QJsonArray usedImages=image.value("usedSizes").toArray();
                for(qint32 s=0;s<imageSources.size();++s){
                    setting.sourceSizes.append(FKUtility::stringToSize(imageSources.at(s).toString()));
                }
                for(qint32 s=0;s<usedImages.size();++s){
                    QSize usedSize=FKUtility::stringToSize(usedImages.at(s).toString());
                    if(!usedSize.isValid()){
                        usedSize=FKUtility::selectBestSizeset(setting.sourceSizes,_targetSizes.at(s));
                    }
                    setting.usedSizes.append(usedSize);
                }

            }
            output("Success");
            return true;
        }
    }
    output(QString("Unable read package.json for %1 package").arg(_sourceFolder.dirName()));
    return false;
}

bool PackageGenerator::addImages(){
    output("Proccess images");
    for(qint32 s=0;s<_targetSizes.size();++s){
        for(QMap<QString,ImageSetting>::ConstIterator i=_imageSettings.constBegin();i!=_imageSettings.constEnd();++i){
            if(!processImage(i.key(),i.value().usedSizes.at(s),_targetSizes.at(s),i.value().crop)){
                return false;
            }
        }
    }
    output("Success");
    return true;
}

bool PackageGenerator::buildQRC(){
    output("Build qrc");
    for(qint32 s=0;s<_targetSizes.size();++s){
        QString sizeset=FKUtility::sizeToString(_targetSizes.at(s));
        QFile qrc(QString("%1/package.qrc").arg(_buildFolder.filePath(sizeset)));
        if(qrc.open(QIODevice::WriteOnly)){
            QXmlStreamWriter xml(&qrc);
            xml.writeStartElement("RCC");
                xml.writeStartElement("qresource");
                    xml.writeAttribute("prefix","/"+_sourceFolder.dirName());
                    for(QMap<QString,ImageSetting>::ConstIterator i=_imageSettings.constBegin();i!=_imageSettings.constEnd();++i){
                        xml.writeTextElement("file",i.key());
                    }
                xml.writeEndElement();
            xml.writeEndElement();
        }else{
            output(QString("Unable write qrc file for %1 package %2 sizeset").arg(_sourceFolder.dirName()).arg(sizeset));
            return false;
        }
    }
    output("Success");
    return true;
}

bool processResource(QProcess* process){
    process->start();
    bool success=process->waitForStarted()
              && process->waitForFinished(60000*10) //10 minutes
              && process->exitCode()==0;
    process->deleteLater();
    return success;
}

bool PackageGenerator::buildRCC(){
    output("Build rcc");

    QProcessEnvironment e(QProcessEnvironment::systemEnvironment());
    QString qtdir(e.value("QTDIR"));
    QString command;
    if(!qtdir.isEmpty()){
        command=qtdir+"/bin/rcc";
    }else{
        output("QTDIR environment variable is not set, trying run rcc from PATH");
        command="rcc";
    }

    QList<QProcess*> processPool;
    for(qint32 s=0;s<_targetSizes.size();++s){
        QString path=QString("%1/%2").arg(_buildFolder.path()).arg(FKUtility::sizeToString(_targetSizes.at(s)));
        QString targetPath=QString("%1/../../bin/%2").arg(_buildFolder.path()).arg(_sourceFolder.dirName());
        QString target=QString("%1/%3.rcc").arg(targetPath).arg(FKUtility::sizeToString(_targetSizes.at(s)));
        QDir().mkpath(targetPath);
        QProcess* process=new QProcess;
        process->setProgram(command);
        process->setArguments(QStringList()<<"-binary"   <<path+"/package.qrc"
                                           <<"-o"        <<target);
        processPool.append(process);
        output(QString("RCC task for %1 added").arg(path));
    }
    QList<bool> results=QtConcurrent::blockingMapped(processPool,processResource);
    for(QList<bool>::ConstIterator r=results.constBegin();r!=results.constEnd();++r){
        if(!(*r))return false;
    }
    output("Success");
    return true;
}

void PackageGenerator::output(const QString& msg){
    _output<<msg<<"\n";
}

bool PackageGenerator::cleanImages(const bool excessiveOnly){
    output("Clean images");
    if(!excessiveOnly){
        return _buildFolder.removeRecursively() && _buildFolder.mkpath(".");
    }else{
        QStringList sizes=_buildFolder.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach(QString size,sizes){
            QDir dir(_buildFolder.filePath(size));
            if(!_targetSizes.contains(FKUtility::stringToSize(size))){
                if(!dir.removeRecursively()){
                    return false;
                }
            }else{
                QStringList images=dir.entryList(QDir::Files);
                foreach(QString image,images){
                    if(!_imageSettings.contains(image)){
                        if(!dir.remove(image)){
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }
}

bool PackageGenerator::processImage(const QString& image, const QSize& sourceSize, const QSize& targetSize, const bool crop){
    QDir targetDir(_buildFolder.filePath(FKUtility::sizeToString(targetSize)));
    if(targetDir.exists(image)){
        return true;
    }
    if(!targetDir.mkpath(".")){
        output(QString("Unable create image target folder %1").arg(targetDir.path()));
        return false;
    }
    QString targetFilePath(targetDir.filePath(image));
    QDir sourceDir(_sourceFolder.filePath(FKUtility::sizeToString(sourceSize)));
    QString sourceFilePath(sourceDir.filePath(image));
    QImage sourceImage;
    if(!sourceImage.load(sourceFilePath)){
        output(QString("Unable read image %1").arg(sourceFilePath));
        return false;
    }

    qreal scaleFactor=std::max(((qreal)targetSize.height())/((qreal)sourceSize.height()),
                               ((qreal)targetSize.width())/((qreal)sourceSize.width()));

    QSize finalSize(sourceImage.size().width()*scaleFactor,sourceImage.size().height()*scaleFactor);

    if(!std::islessequal(std::abs(scaleFactor-1.0),std::numeric_limits<double>::epsilon())){
        //if scale factor != 1
        sourceImage=sourceImage.scaled(finalSize,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    }

    if(crop && (finalSize.width()>targetSize.width() || finalSize.height()>targetSize.height())){
        //if need crop
        sourceImage=sourceImage.copy((finalSize.width()-targetSize.width())/2,
                                     (finalSize.height()-targetSize.height())/2,
                                     finalSize.width(),finalSize.height());
    }

    if(!sourceImage.save(targetFilePath)){
        output(QString("Unable save image %1").arg(targetFilePath));
        return false;
    }

    return true;
}








