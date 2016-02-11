#ifndef IMAGECHECKER_H
#define IMAGECHECKER_H

#include <QObject>

#include <QStringList>

//class ImagesetModel;
#include <QStandardItemModel>
typedef QStandardItemModel ImagesetModel;

class ImageChecker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList sizes READ sizes NOTIFY sizesChanged)
    Q_PROPERTY(ImagesetModel* model READ model NOTIFY modelChanged)
    Q_PROPERTY(QString packageFolder READ packageFolder WRITE setPackageFolder NOTIFY packageFolderChanged)
public:
    explicit ImageChecker(QObject *parent = 0);

    QString packageFolder()const;
    void setPackageFolder(const QString& arg);

    QStringList sizes()const;
    ImagesetModel* model()const;
signals:
    void sizesChanged();
    void modelChanged();
    void packageFolderChanged();
    void packageManagerOutput(QString output);
public slots:
    void refreshPackage();
    void rebuildModel();
private:
    QString _packageFolder;
    ImagesetModel* _model;
};
Q_DECLARE_METATYPE(ImagesetModel*)

#endif // IMAGECHECKER_H
