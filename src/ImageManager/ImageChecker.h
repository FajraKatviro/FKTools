#ifndef IMAGECHECKER_H
#define IMAGECHECKER_H

#include <QObject>

#include <QStringList>
#include <QJsonObject>
#include <QUrl>

#include <QStandardItemModel>
typedef QStandardItemModel ImagesetModel;

class QProcess;

class ImageChecker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList sizes READ sizes NOTIFY sizesChanged)
    Q_PROPERTY(ImagesetModel* model READ model NOTIFY modelChanged)
    Q_PROPERTY(QString packageFolder READ packageFolder WRITE setPackageFolder NOTIFY packageFolderChanged)
    Q_PROPERTY(bool isRefreshing READ isRefreshing NOTIFY isRefreshingChanged)
public:
    explicit ImageChecker(QObject *parent = 0);

    QString packageFolder()const;
    void setPackageFolder(const QString& arg);

    QStringList sizes()const;
    ImagesetModel* model()const;

    bool isRefreshing()const;
signals:
    void sizesChanged();
    void modelChanged();
    void packageFolderChanged();
    void packageManagerOutput(QString output);
    void isRefreshingChanged();
public slots:
    void refreshPackage();
    void rebuildModel();
    void setPackageUrl(const QUrl url);
private slots:
    void refreshError();
private:
    QString _packageFolder;
    ImagesetModel* _model;
    QJsonObject _packageMap;
    QProcess* _rebuildPackageProcess;
    QJsonObject readPackageMap();
};
Q_DECLARE_METATYPE(ImagesetModel*)

#endif // IMAGECHECKER_H
