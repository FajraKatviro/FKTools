#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include <QJsonObject>
#include <QString>
#include <QDir>
#include <QMap>
#include <QList>
#include <QTextStream>

class PackageManager{
public:
    PackageManager(const QString& path);
    bool readData();
    bool writeData();
    void refreshPackage();
    void addSizeset(const QString& size);
    void removeSizeset(const QString& size);
private:
    void readSources();
    bool readSettings();
    void output(const QString& msg);
    QDir _folder;
    QJsonObject _target;
    QMap<QString,QStringList> _sources;
    QTextStream _output;
};

#endif // PACKAGEMANAGER_H
