#ifndef JLCOMPRESSFOLDER_H_
#define JLCOMPRESSFOLDER_H_

#include "quazip.h"
#include "quazipfile.h"
#include "quazipfileinfo.h"
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QFile>

class JlCompress {
private:
    static bool compressFile(QuaZip* zip, QString fileName, QString fileDest);
    static bool compressSubDir(QuaZip* parentZip, QString dir, QString parentDir, bool recursive = true);
    static bool extractFile(QuaZip* zip, QString fileName, QString fileDest);

    static bool removeFile(QStringList listFile);

public:
    static bool compressFile(QString fileCompressed, QString file);
    static bool compressFiles(QString fileCompressed, QStringList files);
    static bool compressDir(QString fileCompressed, QString dir = QString(), bool recursive = true);

public:
    static QString extractFile(QString fileCompressed, QString file, QString fileDest = QString());
    static QStringList extractFiles(QString fileCompressed, QStringList files, QString dir = QString());
    static QStringList extractDir(QString fileCompressed, QString dir = QString());
    static QStringList getFileList(QString fileCompressed);
};

#endif /* JLCOMPRESSFOLDER_H_ */
