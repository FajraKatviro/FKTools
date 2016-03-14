#ifndef PACKAGEGENERATOR_H
#define PACKAGEGENERATOR_H

#include <QTextStream>
#include <QDir>
#include <QMap>
#include <QSize>

class QString;

class PackageGenerator{
public:
    PackageGenerator(const QString& sourcePath,const QString& buildPath);
    bool readSetting();

    bool cleanImages(const bool excessiveOnly);
    bool addImages();
    bool buildQRC();
    bool buildRCC();
private:
    void output(const QString& msg);

    bool processImage(const QString& image,const QSize& sourceSize,const QSize& targetSize,const bool crop);

    QDir _sourceFolder;
    QDir _buildFolder;
    QTextStream _output;

    struct ImageSetting{
        QList<QSize> sourceSizes;
        QList<QSize> usedSizes;
        bool crop;
    };

    QList<QSize> _targetSizes;
    QMap<QString,ImageSetting> _imageSettings;
};

#endif // PACKAGEGENERATOR_H
