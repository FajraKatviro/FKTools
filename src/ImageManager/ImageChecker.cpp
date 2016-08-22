#include "ImageChecker.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QProcessEnvironment>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QPainter>

#include "selectBestSizeset.h"
#include "sizeString.h"

const int ImageChecker::SelfIndexRole=Qt::UserRole+1;
const int ImageChecker::ImageCropRole=Qt::UserRole+2;
const int ImageChecker::AutoSizeRole=Qt::UserRole+3;
const int ImageChecker::SourceSizesRole=Qt::UserRole+4;
const int ImageChecker::PackageSizeRole=Qt::UserRole+5;
const int ImageChecker::CustomSizeRole=Qt::UserRole+6;
const int ImageChecker::AutoSizeValueRole=Qt::UserRole+7;

class CustomImageItem:public QStandardItem{
    virtual QVariant data(int role = Qt::UserRole + 1) const override{
        if(role==Qt::DisplayRole){
            if(data(ImageChecker::AutoSizeRole).toBool()){
                return data(ImageChecker::AutoSizeValueRole);
            }else{
                return data(ImageChecker::CustomSizeRole);
            }
        }else if(role==ImageChecker::AutoSizeValueRole){
            QJsonArray sourseImageSizes=parent()->data(ImageChecker::SourceSizesRole).toJsonArray();
            QList<QSize> sourceSizes;
            for(auto s=sourseImageSizes.constBegin();s!=sourseImageSizes.constEnd();++s){
                sourceSizes.append(FKUtility::stringToSize((*s).toString()));
            }
            QSize bestSize=FKUtility::selectBestSizeset(sourceSizes,FKUtility::stringToSize(data(ImageChecker::PackageSizeRole).toString()));
            return FKUtility::sizeToString(bestSize);
        }
        return QStandardItem::data(role);
    }
public:
    CustomImageItem(const QString& text):QStandardItem(text){}
};

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
        runManager();
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
    if(!_packageLoaded){
        emit packageManagerOutput("Unable save package, load package first");
        return;
    }
    applySettings();
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
    roleNames[SourceSizesRole]="sourceSizes";
    roleNames[CustomSizeRole]="customSize";
    roleNames[AutoSizeValueRole]="autoSizeValue";
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
            QString imageSize=customImageSizes.at(i).toString();
            QStandardItem* imageSizeItem=new CustomImageItem(imageSize);
            row->appendRow(imageSizeItem);
            imageSizeItem->setData(targetImageSizes.at(i),PackageSizeRole);
            imageSizeItem->setData(imageSize.isEmpty(),AutoSizeRole);
            if(imageSize.isEmpty())imageSize=imageSizeItem->data(Qt::DisplayRole).toString();
            imageSizeItem->setData(imageSize,CustomSizeRole);
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

void ImageChecker::spawnPackage(const QUrl url){
    if(!_packageLoaded){
        emit packageManagerOutput("Unable spawn package, load package first");
        return;
    }
    QStringList args("--add");
    //args<<"--qrc"<<"--rcc";
    runGenerator(url.toLocalFile(),args);
}

void ImageChecker::spawnImage(const QUrl source, const QUrl target, const QString templateName){

    emit packageManagerOutput(QString("Start spawn image"));

    QImage sourceImage(source.toLocalFile());
    QDir targetDir(target.toLocalFile());

    QFile templateFile(":/deployTemplates/" + templateName);
    if(!templateFile.open(QIODevice::ReadOnly)){
        emit packageManagerOutput(QString("Unable open template %1").arg(templateName));
        return;
    }
    QTextStream textStream(&templateFile);

    bool success = false;

    while (true){
        QString token(textStream.readLine());
        if (token.isNull()){
            break;
        }

        QStringList args(token.split(':'));
        QString targetName(args.takeFirst());
        if(args.isEmpty()){
            emit packageManagerOutput(QString("Invalid template %1 (no size provided). Operation interrupted").arg(templateName));
            return;
        }

        qint32 width = args.takeFirst().toInt();
        qint32 height = args.isEmpty() ? width : args.takeFirst().toInt();
        if(width<=0 || height<=0){
            emit packageManagerOutput(QString("Invalid template %1 (invalid size provided). Operation interrupted").arg(templateName));
            return;
        }

        if(!args.isEmpty()){
            emit packageManagerOutput(QString("Invalid template %1 (excessive symbols provided). Operation interrupted").arg(templateName));
            return;
        }

        QImage targetImage(sourceImage.scaled(width,height,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
        QString filePath(targetDir.filePath(targetName));
        QFileInfo(filePath).dir().mkpath(".");
        if(!targetImage.save(filePath,"PNG")){
            emit packageManagerOutput(QString("Failed write %1").arg(targetDir.filePath(targetName)));
        }else{
            success = true;
        }
    }

    if(success)emit packageManagerOutput(QString("Image spawned to %1").arg(targetDir.absolutePath()));

}

void ImageChecker::createSpriteSheet(const QUrl source, const QUrl target, const int spriteWidth, const int spriteHeight)
{
    QDir sourceDir(source.toLocalFile());
    QStringList files(sourceDir.entryList(QStringList("*.png")));
    int rowCount=8;
    int columnCount=files.count()/rowCount;
    if(files.isEmpty() || rowCount*columnCount!=files.count()){
        emit packageManagerOutput(QString("Invalid imageset for spritesheet"));
    }

    QImage spriteSheet(spriteWidth*columnCount,spriteHeight*rowCount,QImage::Format_ARGB32);
    QPainter painter(&spriteSheet);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(spriteSheet.rect(),Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    for(int y=0;y<rowCount;++y){
        for(int x=0;x<columnCount;++x){
            QImage image(sourceDir.absoluteFilePath(files.at(y*columnCount+x)));
            painter.drawImage(x*spriteWidth+(spriteWidth-image.width())/2.0,y*spriteHeight+(spriteHeight-image.height()),image);
        }
    }

    spriteSheet.save(target.toLocalFile() + "/spritesheet.png","PNG");

    packageManagerOutput(QString("Spritesheet created"));

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

QString ImageChecker::toolsFolder() const{
#ifdef Q_OS_MAC
    return QProcessEnvironment::systemEnvironment().value("HOME") + "/Applications/FKTools";
#elif defined(Q_OS_WIN32)
    return QProcessEnvironment::systemEnvironment().value("APPDATA") + "/FKTools";
#else
    return "";
#endif
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
        _rebuildPackageProcess->start(QString("%2/PackageManager%1").arg(platformSuffix).arg(toolsFolder()),arguments);
        emit isRefreshingChanged();
    }
}

void ImageChecker::runGenerator(const QString& targetPath, const QStringList& args)
{
    QString platformSuffix;
#ifdef Q_OS_WIN32
    platformSuffix=".exe";
#endif
    QStringList commandArgs(_packageFolder);
    commandArgs<<targetPath<<args;
    QProcess::startDetached(QString("%2/PackageGenerator%1").arg(platformSuffix).arg(toolsFolder()),commandArgs);
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

