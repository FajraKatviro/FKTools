#include "ImageChecker.h"

#include <QStandardItemModel>

ImageChecker::ImageChecker(QObject *parent) : QObject(parent),_model(0)
{
    _model=new QStandardItemModel(this);
    QStringList images;
    images<<"bg.png"<<"icon.jpeg";
    QHash<int,QByteArray> roleNames=_model->roleNames();
    roleNames[Qt::UserRole]="selfIndex";
    _model->setItemRoleNames(roleNames);
    foreach(QString img,images){
        QStandardItem* row=new QStandardItem(img);
        foreach(QString size,sizes()){
            row->appendRow(new QStandardItem(size));
        }
        _model->appendRow(row);
        row->setData(row->index(),Qt::UserRole);
    }
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
    }
}

QStringList ImageChecker::sizes() const
{
    return QStringList()<<"2048x1756"<<"800x600";
}

ImagesetModel* ImageChecker::model() const
{
    return _model;
}

void ImageChecker::refreshPackage()
{

}

void ImageChecker::rebuildModel()
{

}

