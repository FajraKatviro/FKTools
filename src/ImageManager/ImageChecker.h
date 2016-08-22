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
    void setPackageUrl(const QUrl url);
    void addSizeset(const QString size);
    void removeSizeset(const QString size);
    void spawnPackage(const QUrl url);
    void spawnImage(const QUrl source, const QUrl target, const QString templateName);
    void createSpriteSheet(const QUrl source, const QUrl target,const int spriteWidth,const int spriteHeight);
private slots:
    void refreshError();
    void rebuildModel(int returnCode);
private:
    void runManager(const QStringList& addSizes=QStringList(),const QStringList& removeSizes=QStringList());
    void runGenerator(const QString& targetPath,const QStringList& args=QStringList());
    void applySettings();
    QString toolsFolder()const;
    QString _packageFolder;
    ImagesetModel* _model;
    QJsonObject _packageMap;
    QProcess* _rebuildPackageProcess;
    QJsonObject readPackageMap();
    bool writePackageMap();
    bool _packageLoaded=false;

    static const int SelfIndexRole;
    static const int ImageCropRole;
    static const int AutoSizeRole;
    static const int SourceSizesRole;
    static const int PackageSizeRole;
    static const int CustomSizeRole;
    static const int AutoSizeValueRole;
    friend class CustomImageItem;
};
Q_DECLARE_METATYPE(ImagesetModel*)

#endif // IMAGECHECKER_H
