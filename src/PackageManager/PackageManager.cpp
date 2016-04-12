#include "PackageManager.h"

#include <QFileInfo>

#include <QJsonArray>
#include <QJsonDocument>

PackageManager::PackageManager(const QString& path):
    _folder(QDir::cleanPath(path)),_output(stdout){

}

bool PackageManager::readData(){
    if(!_folder.exists()){
        output("Package folder does not exists");
        return false;
    }
    if(!readSettings()){
        output("Unable read package settings");
        return false;
    }
    readSources();
    return true;
}

bool PackageManager::writeData(){
    QFile json(_folder.filePath("package.json"));
    if(!json.open(QIODevice::WriteOnly)){
        output("Unable open package json for writing");
        return false;
    }
    QJsonDocument doc(_target);
    QByteArray data=doc.toJson();
    if(json.write(data)<=0){
        output("Unable write package json");
        return false;
    }
    output("Done");
    return true;
}

void PackageManager::refreshPackage(){
    QList<QString> sourceImages(_sources.keys());

    QJsonArray packageImages(_target.value("images").toArray());
    QJsonArray packageSizes(_target.value("sizes").toArray());

    QJsonArray autoSizes;
    qint32 sizeCount=packageSizes.count();
    for(qint32 i=0;i<sizeCount;++i){
        autoSizes.append("");
    }

    //remove deleted images
    for(QJsonArray::iterator i=packageImages.begin();i!=packageImages.end();){
        QJsonObject imageRecord=(*i).toObject();
        QString image=imageRecord.value("path").toString();
        int index=sourceImages.indexOf(image);
        if(index<0){
            i=packageImages.erase(i);
            output(QString("Remove old image %1 from index").arg(image));
        }else{
            sourceImages.removeAt(index);

            //sync sizes
            QJsonArray sourceSizes=QJsonArray::fromStringList(_sources.value(image));
            imageRecord["sourceSizes"]=sourceSizes;
            //const qint32 sourceSizesCount=sourceSizes.count();
            QJsonArray usedSizes=imageRecord.value("usedSizes").toArray();
            while(usedSizes.count()>sizeCount){
                usedSizes.removeLast();
                output(QString("Remove excessive element from usedSizes for %1 image").arg(image));
            }
            for(QJsonArray::iterator s=usedSizes.begin();s!=usedSizes.end();++s){
                if(!s->toString().isEmpty() && !sourceSizes.contains(*s)){
                    output(QString("Remove %1 usedSize from index for %2 image (source size does not exists)").arg(s->toString()).arg(image));
                    *s="";
                }
            }
            while(usedSizes.count()<sizeCount){
                output(QString("Add missing element to usedSizes for %1 image").arg(image));
                usedSizes.append("");
            }
            imageRecord["usedSizes"]=usedSizes;
            *i=imageRecord;

            ++i;
        }
    }

    //add new images
    for(QList<QString>::const_iterator i=sourceImages.constBegin();i!=sourceImages.constEnd();++i){
        QJsonObject imageRecord;
        imageRecord.insert("path",*i);
        imageRecord.insert("sourceSizes",QJsonArray::fromStringList(_sources.value(*i)));
        imageRecord.insert("usedSizes",autoSizes);
        packageImages.append(imageRecord);
        output(QString("Add new image %1 to index").arg(*i));
    }

    _target["images"]=packageImages;
    _target["sizes"]=packageSizes;
}

void PackageManager::addSizeset(const QString& size){
    QJsonArray sizes=_target.value("sizes").toArray();
    if(!sizes.contains(size)){
        sizes.append(size);
        _target["sizes"]=sizes;
        QJsonArray images(_target.value("images").toArray());
        for(QJsonArray::iterator img=images.begin();img!=images.end();++img){
            QJsonObject imageRecord=img->toObject();
            QJsonArray usedSizes(imageRecord.value("usedSizes").toArray());
            usedSizes.append("");
            imageRecord["usedSizes"]=usedSizes;
            *img=imageRecord;
        }
        _target["images"]=images;
        output(QString("Sizeset %1 added").arg(size));
    }else{
        output(QString("Sizeset %1 already exists").arg(size));
    }
}

void PackageManager::removeSizeset(const QString& size){
    QJsonArray sizes=_target.value("sizes").toArray();
    const qint32 length=sizes.size();
    for(qint32 i=0;i<length;++i){
        if(sizes.at(i).toString()==size){
            sizes.removeAt(i);
            _target["sizes"]=sizes;
            QJsonArray images(_target.value("images").toArray());
            for(QJsonArray::iterator img=images.begin();img!=images.end();++img){
                QJsonObject imageRecord=img->toObject();
                QJsonArray usedSizes(imageRecord.value("usedSizes").toArray());
                usedSizes.removeAt(i);
                imageRecord["usedSizes"]=usedSizes;
                *img=imageRecord;
            }
            _target["images"]=images;
            output(QString("Sizeset %1 removed").arg(size));
            return;
        }
    }
    output(QString("Unable remove %1 sizeset (does not exists)").arg(size));
}

void PackageManager::readSources(){
    foreach(QFileInfo size,_folder.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)){
        QDir sizeDir(size.filePath());
        foreach(QString image, sizeDir.entryList(QDir::Files)){
            _sources[image].append(size.fileName());
        }
    }
}

bool PackageManager::readSettings(){
    QFile json(_folder.filePath("package.json"));
    if(json.exists()){
        if(!json.open(QIODevice::ReadOnly)){
            output("Unable open package json for reading");
            return false;
        }
        QByteArray data(json.readAll());
        _target=QJsonDocument::fromJson(data).object();
    }else{
        _target=QJsonObject();
    }
    if(_target.isEmpty()){
        _target["sizes"]=QJsonArray();
        _target["images"]=QJsonArray();
    }
    return true;
}

void PackageManager::output(const QString& msg){
    _output<<msg<<"\n";
}
