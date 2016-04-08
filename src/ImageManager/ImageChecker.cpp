#include "ImageChecker.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QFile>
#include <QDir>

#include "../FKUtility/selectBestSizeset.h"
#include "../FKUtility/sizeString.h"

const int ImageChecker::SelfIndexRole=Qt::UserRole+1;
const int ImageChecker::ImageCropRole=Qt::UserRole+2;
const int ImageChecker::AutoSizeRole=Qt::UserRole+3;
const int ImageChecker::SourceSizesRole=Qt::UserRole+4;

ImageChecker::ImageChecker(QObject *parent) : QObject(parent),_model(nullptr)
{
    _rebuildPackageProcess=new QProcess(this);
    connect(_rebuildPackageProcess,SIGNAL(finished(int)),SLOT(rebuildModel(int)));
    connect(_rebuildPackageProcess,SIGNAL(error(QProcess::ProcessError)),SLOT(refreshError()));
}

QString ImageChecker::packageFolder() const
{
    return _packageFolder;
}

void ImageChecker::setPackageFolder(const QString& arg)
{
    if(_packageFolder!=arg){
        _packageFolder=arg;
        _packageLoaded=false;
        emit packageFolderChanged();
        refreshPackage();
    }
}

QStringList ImageChecker::sizes() const
{
    return _packageMap.value("sizes").toVariant().toStringList();
}

ImagesetModel* ImageChecker::model() const
{
    return _model;
}

bool ImageChecker::isRefreshing() const
{
    return _rebuildPackageProcess->state()!=QProcess::NotRunning;
}

void ImageChecker::refreshPackage(){
    runManager();
}

void ImageChecker::rebuildModel(int returnCode)
{
    if(returnCode){
        emit packageManagerOutput(QString("Package manager error %1 has occured").arg(QString::number(returnCode)));
    }

    QString output(_rebuildPackageProcess->readAllStandardOutput());
    emit packageManagerOutput(output);

    if(_model)_model->deleteLater();

    _packageMap=readPackageMap();

    _model=new ImagesetModel(this);
    QHash<int,QByteArray> roleNames=_model->roleNames();
    roleNames[SelfIndexRole]="selfIndex";
    roleNames[ImageCropRole]="imageCrop";
    roleNames[AutoSizeRole]="autoSize";
    _model->setItemRoleNames(roleNames);

    QStringList targetImageSizes(sizes());
    qint32 sizeCount=targetImageSizes.count();
    QJsonArray images=_packageMap.value("images").toArray();

    for(auto img=images.constBegin();img!=images.constEnd();++img){
        QJsonObject image=img->toObject();
        QStandardItem* row=new QStandardItem(image.value("path").toString());
        row->setData(image.value("crop").toBool(),ImageCropRole);

        QJsonArray customImageSizes=image.value("usedSizes").toArray();
        QJsonArray sourseImageSizes=image.value("sourceSizes").toArray();
        row->setData(sourseImageSizes,SourceSizesRole);

        QList<QSize> sourceSizes;
        for(auto s=sourseImageSizes.constBegin();s!=sourseImageSizes.constEnd();++s){
            sourceSizes.append(FKUtility::stringToSize((*s).toString()));
        }
        for(qint32 i=0;i<sizeCount;++i){
            bool autoSize=false;
            QString imageSize=customImageSizes.at(i).toString();
            if(imageSize.isEmpty()){
                autoSize=true;
                QSize bestSize=FKUtility::selectBestSizeset(sourceSizes,FKUtility::stringToSize(targetImageSizes.at(i)));
                imageSize=FKUtility::sizeToString(bestSize);
            }
            QStandardItem* imageSizeItem=new QStandardItem(imageSize);
            imageSizeItem->setData(autoSize,AutoSizeRole);
            row->appendRow(imageSizeItem);
        }
        _model->appendRow(row);
        row->setData(row->index(),SelfIndexRole);
    }

    emit sizesChanged();
    emit modelChanged();
    emit isRefreshingChanged();
}

void ImageChecker::setPackageUrl(const QUrl url){
    QDir dir;
    setPackageFolder(dir.relativeFilePath(url.toLocalFile()));
}

void ImageChecker::addSizeset(const QString size){
    if(!_packageLoaded){
        emit packageManagerOutput("Unable add sizeset, load package first");
        return;
    }
    applySettings();
    runManager(QStringList(size));
}

void ImageChecker::removeSizeset(const QString size){
    if(!_packageLoaded){
        emit packageManagerOutput("Unable remove sizeset, load package first");
        return;
    }
    applySettings();
    runManager(QStringList(),QStringList(size));
}

void ImageChecker::applySettings(){
    qint32 rows=_model->rowCount();
    QJsonArray images;
    for(qint32 r=0;r<rows;++r){
        QStandardItem* item=_model->item(r);
        QJsonObject image;
        image["path"]=item->data(Qt::DisplayRole).toString();
        if(item->data(ImageCropRole).toBool()){
            image["crop"]=true;
        }
        image["sourceSizes"]=item->data(SourceSizesRole).toJsonArray();
        QJsonArray customImageSizes;
        qint32 usedSizesCount=item->rowCount();
        for(qint32 s=0;s<usedSizesCount;++s){
            customImageSizes.append(item->child(s)->data(AutoSizeRole).toBool() ?
                                        "" :
                                        item->child(s)->data(Qt::DisplayRole).toString());
        }
        image["usedSizes"]=customImageSizes;
        images.append(image);
    }
    _packageMap["images"]=images;

    writePackageMap();
}

void ImageChecker::refreshError(){
    QString output=QString("Refresh error: %1").arg(QString::number(_rebuildPackageProcess->error()));
    emit packageManagerOutput(output);

    rebuildModel(-1);
}

void ImageChecker::runManager(const QStringList& addSizes, const QStringList& removeSizes){
    if(_packageFolder.isEmpty()){
        emit packageManagerOutput("Unable run package manager, select package first");
        return;
    }
    if(!isRefreshing()){
        QString platformSuffix;
#ifdef Q_OS_WIN32
        platformSuffix=".exe";
#endif
        QStringList arguments(_packageFolder);
        foreach(QString size,addSizes){
            arguments.append("-a");
            arguments.append(size);
        }
        foreach(QString size,removeSizes){
            arguments.append("-r");
            arguments.append(size);
        }
        _rebuildPackageProcess->start(QString("PackageManager%1").arg(platformSuffix),arguments);
        emit isRefreshingChanged();
    }
}

QJsonObject ImageChecker::readPackageMap(){

    QFile dataSource(QString("%1/package.json").arg(_packageFolder));

    if(!dataSource.open(QIODevice::ReadOnly)){
        QString error("Unable open package.json");
        emit packageManagerOutput(error);
        return QJsonObject();
    }

    QByteArray data(dataSource.readAll());

/*
    QByteArray data(""
                    "{"
                    ""
                    ""
                    "   \"sizes\":[\"2732x1536\",\"1024x768\"],"
                    ""
                    "   \"images\":["
                    "       {\"path\":\"myImg.png\","
                    "        \"crop\":false,"
                    "        \"sourceSizes\":["
                    "           \"2732x1536\","
                    "           \"400x400\""
                    "        ],"
                    "        \"usedSizes\":["
                    "           \"\","
                    "           \"400x400\""
                    "        ]},"
                    "       {\"path\":\"compressed.jpg\","
                    "        \"crop\":false,"
                    "        \"sourceSizes\":["
                    "           \"2732x1536\""
                    "        ],"
                    "        \"usedSizes\":["
                    "           \"\","
                    "           \"\""
                    "        ]}"
                    "   ]"
                    "}"
                    "");
*/

    QJsonDocument doc(QJsonDocument::fromJson(data));
    _packageLoaded=true;
    return doc.object();
}

bool ImageChecker::writePackageMap(){
    QFile dataSource(QString("%1/package.json").arg(_packageFolder));

    if(!dataSource.open(QIODevice::WriteOnly)){
        QString error("Unable open package.json for write");
        emit packageManagerOutput(error);
        return false;
    }

    QJsonDocument doc(_packageMap);
    QByteArray data(doc.toJson());
    return dataSource.write(data);
}

