#include "ImageChecker.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QFile>

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

void ImageChecker::refreshPackage()
{
    if(!isRefreshing()){
        QString platformSuffix;
#ifdef Q_OS_WIN32
        platformSuffix=".exe";
#endif
        _rebuildPackageProcess->start(QString("PackageManager%1").arg(platformSuffix));
        emit isRefreshingChanged();
    }
}

void ImageChecker::rebuildModel()
{
    QString output(_rebuildPackageProcess->readAll());
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
        QJsonObject& image=(*img).toObject();
        QStandardItem* row=new QStandardItem(image.value("path").toString());
        row->setData(image.value("crop"),ImageCropRole);

        QJsonArray customImageSizes=image.value("originSizes").toArray();
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

void ImageChecker::refreshError()
{
    QString output=QString("Refresh error: %1").arg(QString::number(_rebuildPackageProcess->error()));
    emit packageManagerOutput(output);

    rebuildModel();
}

QJsonObject ImageChecker::readPackageMap(){

    //QFile dataSource(QString("%1/packages.json").arg(_packageFolder));

    QByteArray data(""
                    "{"
                    ""
//                    "   \"sourceImages\":["
//                    "      {\"path\":\"myImg.png\","
//                    "       \"sizes\":["
//                    "           \"2732x1536\","
//                    "           \"400x400\""
//                    "       ]},"
//                    "      {\"path\":\"compressed.jpg\","
//                    "       \"sizes\":["
//                    "           \"2732x1536\""
//                    "       ]}"
//                    "   ],"
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
                    "        \"originSizes\":["
                    "           \"\","
                    "           \"400x400\""
                    "        ]},"
                    "       {\"path\":\"compressed.jpg\","
                    "        \"crop\":false,"
                    "        \"sourceSizes\":["
                    "           \"2732x1536\""
                    "        ],"
                    "        \"originSizes\":["
                    "           \"\","
                    "           \"\""
                    "        ]}"
                    "   ]"
                    "}"
                    "");

    QJsonDocument doc(QJsonDocument::fromJson(data));
    return doc.object();
}

