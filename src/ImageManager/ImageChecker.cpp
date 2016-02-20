#include "ImageChecker.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QFile>
#include <QDir>

#include "../FKUtility/selectBestSizeset.h"
#include "../FKUtility/sizeString.h"

ImageChecker::ImageChecker(QObject *parent) : QObject(parent),_model(0)
{
    _rebuildPackageProcess=new QProcess(this);
    connect(_rebuildPackageProcess,SIGNAL(finished(int)),SLOT(rebuildModel()));
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

void ImageChecker::rebuildModel()
{
    QString output(_rebuildPackageProcess->readAllStandardOutput());
    emit packageManagerOutput(output);

    if(_model)_model->deleteLater();

    _packageMap=readPackageMap();

    _model=new ImagesetModel(this);
    QHash<int,QByteArray> roleNames=_model->roleNames();
    const int SelfIndexRole=Qt::UserRole+1;
    const int ImageCropRole=Qt::UserRole+2;
    const int AutoSizeRole=Qt::UserRole+3;
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
        row->setData(image.value("crop"),ImageCropRole);

        QJsonArray customImageSizes=image.value("usedSizes").toArray();
        QJsonArray sourseImageSizes=image.value("sourceSizes").toArray();
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

void ImageChecker::setPackageUrl(const QUrl url)
{
    QDir dir;
    setPackageFolder(dir.relativeFilePath(url.toLocalFile()));
}

void ImageChecker::addSizeset(const QString size){
    runManager(QStringList(size));
}

void ImageChecker::removeSizeset(const QString size){
    runManager(QStringList(),QStringList(size));
}

void ImageChecker::refreshError()
{
    QString output=QString("Refresh error: %1").arg(QString::number(_rebuildPackageProcess->error()));
    emit packageManagerOutput(output);

    rebuildModel();
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
            arguments.append("r");
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

//    QByteArray data(""
//                    "{"
//                    ""
//                    ""
//                    "   \"sizes\":[\"2732x1536\",\"1024x768\"],"
//                    ""
//                    "   \"images\":["
//                    "       {\"path\":\"myImg.png\","
//                    "        \"crop\":false,"
//                    "        \"sourceSizes\":["
//                    "           \"2732x1536\","
//                    "           \"400x400\""
//                    "        ],"
//                    "        \"usedSizes\":["
//                    "           \"\","
//                    "           \"400x400\""
//                    "        ]},"
//                    "       {\"path\":\"compressed.jpg\","
//                    "        \"crop\":false,"
//                    "        \"sourceSizes\":["
//                    "           \"2732x1536\""
//                    "        ],"
//                    "        \"usedSizes\":["
//                    "           \"\","
//                    "           \"\""
//                    "        ]}"
//                    "   ]"
//                    "}"
//                    "");

    QJsonDocument doc(QJsonDocument::fromJson(data));
    return doc.object();
}

