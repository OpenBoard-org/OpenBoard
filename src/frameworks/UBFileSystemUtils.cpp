/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBFileSystemUtils.h"

#include <QtGui>

#include "core/UBApplication.h"

#include "globals/UBGlobals.h"

THIRD_PARTY_WARNINGS_DISABLE
#ifdef Q_OS_OSX
    #include <quazipfile.h>
#else
    #include "quazipfile.h"
#endif
#include <openssl/md5.h>
THIRD_PARTY_WARNINGS_ENABLE

#include "core/memcheck.h"

QStringList UBFileSystemUtils::sTempDirToCleanUp;


UBFileSystemUtils::UBFileSystemUtils()
{
    // NOOP
}


UBFileSystemUtils::~UBFileSystemUtils()
{
    // NOOP
}


QString UBFileSystemUtils::removeLocalFilePrefix(QString input)
{
#ifdef Q_OS_WIN
    if(input.startsWith("file:///"))
        return input.mid(8);
    else
        return input;
#else
    if(input.startsWith("file://"))
        return input.mid(7);
    else
        return input;
#endif
}

bool UBFileSystemUtils::isAZipFile(QString &filePath)
{
   if(QFileInfo(filePath).isDir()) return false;
   QFile file(filePath);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
       return false;

   bool result = false;
   QByteArray responseArray = file.readLine(10);
   QString responseString(responseArray);

   result = responseString.startsWith("pk", Qt::CaseInsensitive);

   file.close();
   return result;
}

bool UBFileSystemUtils::copyFile(const QString &source, const QString &destination, bool overwrite)
{
    if (!QFile::exists(source)) {
        qDebug() << "file" << source << "does not present in fs";
        return false;
    }

    QString normalizedDestination = destination;
    if (QFile::exists(normalizedDestination)) {
        if  (QFileInfo(normalizedDestination).isFile() && overwrite) {
            QFile::remove(normalizedDestination);
        }
    } else {
        normalizedDestination = normalizedDestination.replace(QString("\\"), QString("/"));
        int pos = normalizedDestination.lastIndexOf("/");
        if (pos != -1) {
            QString newpath = normalizedDestination.left(pos);
            if (!QDir().mkpath(newpath)) {
                qDebug() << "can't create a new path at " << newpath;
            }
        }
    }
    return QFile::copy(source, normalizedDestination);
}

bool UBFileSystemUtils::copy(const QString &source, const QString &destination, bool overwrite)
{
    if (QFileInfo(source).isDir()) {
        return copyDir(source, destination, overwrite);
    } else {
        return copyFile(source, destination, overwrite);
    }
}

bool UBFileSystemUtils::deleteFile(const QString &path)
{
    QFile f(path);
    f.setPermissions(path, QFile::ReadOwner | QFile::WriteOwner);
    return f.remove();
}

QString UBFileSystemUtils::defaultTempDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + defaultTempDirName();
}

QString UBFileSystemUtils::createTempDir(const QString& templateString, bool autoDeleteOnExit)
{
    QString appTempDir =  QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                                  + "/" + templateString;

    int index = 0;
    QDir dir;

    do
    {
        index++;
        QString dirName = appTempDir + QString("%1").arg(index);
        dir = QDir(dirName);
    }
    while(dir.exists() && index < 10000);

    dir.mkpath(dir.path());

    if (autoDeleteOnExit)
        UBFileSystemUtils::sTempDirToCleanUp << dir.path();

    return dir.path();
}


QString UBFileSystemUtils::nextAvailableFileName(const QString& filename, const QString& inter)
{
    QFile f(filename);

    if (!f.exists())
        return filename;

    int index = 0;

    QString uniqueFilename;
    QFileInfo fi(filename);

    QString base = fi.dir().path() + "/" + fi.baseName();
    QString suffix = fi.suffix();

    do
    {
        index++;
        uniqueFilename = base + QString("%1%2").arg(inter).arg(index) + "." + suffix;
        f.setFileName(uniqueFilename);
    }
    while(f.exists() && index < 10000);

    return uniqueFilename;

}


void UBFileSystemUtils::deleteAllTempDirCreatedDuringSession()
{
    foreach (QString dirPath, sTempDirToCleanUp)
    {
        qWarning() << "will delete" << dirPath;

        deleteDir(dirPath);
    }
}



void UBFileSystemUtils::cleanupGhostTempFolders(const QString& templateString)
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    foreach (QFileInfo dirContent, dir.entryInfoList(QDir::Dirs
          | QDir::NoDotAndDotDot | QDir::Hidden , QDir::Name))
    {
        if (dirContent.fileName().startsWith(templateString))
        {
            deleteDir(dirContent.absoluteFilePath());
        }
    }
}


QStringList UBFileSystemUtils::allFiles(const QString& pDirPath, bool isRecursive)
{
    QStringList result;
    if (pDirPath == "" || pDirPath == "." || pDirPath == "..")
        return result;

    QDir dir(pDirPath);

    foreach(QFileInfo dirContent, dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot , QDir::Name))
    {
        if (isRecursive && dirContent.isDir())
        {
            result << allFiles(dirContent.absoluteFilePath());
        }
        else
        {
            result << dirContent.absoluteFilePath();
        }
    }
    return result;
}

QFileInfoList UBFileSystemUtils::allElementsInDirectory(const QString& pDirPath)
{
    QDir dir = QDir(pDirPath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setSorting(QDir::DirsFirst);

    return QFileInfoList(dir.entryInfoList());
}


bool UBFileSystemUtils::deleteDir(const QString& pDirPath)
{
    if (pDirPath == "" || pDirPath == "." || pDirPath == "..")
        return false;

    QDir dir(pDirPath);

    if (dir.exists())
    {
        foreach(QFileInfo dirContent, dir.entryInfoList(QDir::Files | QDir::Dirs
                | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System , QDir::Name))
        {
            if (dirContent.isDir())
            {
                deleteDir(dirContent.absoluteFilePath());
            }
            else
            {
                if (!dirContent.dir().remove(dirContent.fileName()))
                {
                    return false;
                }
            }
        }
    }

    return dir.rmdir(pDirPath);
}


bool UBFileSystemUtils::copyDir(const QString& pSourceDirPath, const QString& pTargetDirPath, bool overwite)
{
    if (pSourceDirPath == "" || pSourceDirPath == "." || pSourceDirPath == "..")
        return false;

    QDir dirSource(pSourceDirPath);
    QDir dirTarget(pTargetDirPath);

    if (!dirTarget.mkpath(pTargetDirPath))
        return false;

    bool successSoFar = true;

    foreach(QFileInfo dirContent, dirSource.entryInfoList(QDir::Files | QDir::Dirs
            | QDir::NoDotAndDotDot | QDir::Hidden , QDir::Name))
    {
        if (successSoFar)
        {
            if (dirContent.isDir())
            {
                successSoFar = copyDir(pSourceDirPath + "/" + dirContent.fileName(), pTargetDirPath + "/" + dirContent.fileName());
            }
            else
            {
                successSoFar = copyFile(pSourceDirPath + "/" + dirContent.fileName(), pTargetDirPath + "/" + dirContent.fileName(), overwite);
            }
        }
        else
        {
            break;
        }
    }

    return successSoFar;
}


bool UBFileSystemUtils::moveDir(const QString& pSourceDirPath, const QString& pTargetDirPath)
{
    bool copySuccess = copyDir(pSourceDirPath, pTargetDirPath);

    if (copySuccess)
    {
        return deleteDir(pSourceDirPath);
    }
    else
    {
        return false;
    }
}



QString UBFileSystemUtils::cleanName(const QString& name)
{
    QString result = name;
    result = result.remove("/");
    result = result.remove(":");
    result = result.remove("?");
    result = result.remove("*");
    result = result.remove("\\");

    //http://support.microsoft.com/kb/177506

    result = result.remove("<");
    result = result.remove(">");
    result = result.remove("|");

    return result;
}

QString UBFileSystemUtils::normalizeFilePath(const QString& pFilePath)
{
    QString result = pFilePath;
    return result.replace("\\", "/");
}

QString UBFileSystemUtils::digitFileFormat(const QString& s, int digit)
{
    return s.arg(digit, 3, 10, QLatin1Char('0'));
}


QString UBFileSystemUtils::thumbnailPath(const QString& path)
{
    QFileInfo pathInfo(path);

    return pathInfo.dir().absolutePath() + "/" + pathInfo.completeBaseName() + ".thumbnail.png";
}

QString UBFileSystemUtils::extension(const QString& fileName)
{
    QString extension("");

    int lastDotIndex = fileName.lastIndexOf(".");

    if (lastDotIndex > 0)
    {
        extension = fileName.right(fileName.length() - lastDotIndex - 1).toLower();

        if (extension.endsWith("/") || extension.endsWith("\\"))
            extension = extension.left(extension.length() - 1);
    }

    return extension;
}

QString UBFileSystemUtils::lastPathComponent(const QString& path)
{
    QString lastPathComponent = normalizeFilePath(path);

    int lastSeparatorIndex = lastPathComponent.lastIndexOf("/");

    if (lastSeparatorIndex + 1 == path.length()) {
        lastPathComponent = lastPathComponent.left(lastPathComponent.length() - 1);
        lastSeparatorIndex = lastPathComponent.lastIndexOf("/");
    }

    if (lastSeparatorIndex > 0){
        lastPathComponent = lastPathComponent.right(lastPathComponent.length() - lastSeparatorIndex - 1);
    }
    else {
        return 0;
    }

    return lastPathComponent;
}

QString UBFileSystemUtils::mimeTypeFromFileName(const QString& fileName)
{
    QString ext = extension(fileName);

    if (ext == "xls" || ext == "xlsx") return "application/msexcel";
    if (ext == "ppt" || ext == "pptx") return "application/mspowerpoint";
    if (ext == "ief") return "image/ief";
    if (ext == "m3u") return "audio/x-mpegurl";
    if (ext == "key") return "application/x-iwork-keynote-sffkeynote";
    if (ext == "odf") return "application/vnd.oasis.opendocument.formula";
    if (ext == "aif" || ext == "aiff" || ext == "aifc") return "audio/x-aiff";
    if (ext == "odp") return "application/vnd.oasis.opendocument.presentation";
    if (ext == "xml") return "application/xml";
    if (ext == "rgb") return "image/x-rgb";
    if (ext == "ods") return "application/vnd.oasis.opendocument.spreadsheet";
    if (ext == "rtf") return "text/rtf";
    if (ext == "odt") return "application/vnd.oasis.opendocument.text";
    if (ext == "xbm") return "image/x-xbitmap";
    if (ext == "lsx" || ext == "lsf") return "video/x-la-asf";
    if (ext == "jfif") return "image/pipeg";
    if (ext == "ppm") return "image/x-portable-pixmap";
    if (ext == "csv") return "text/csv";
    if (ext == "pgm") return "image/x-portable-graymap";
    if (ext == "odc") return "application/vnd.oasis.opendocument.chart";
    if (ext == "odb") return "application/vnd.oasis.opendocument.database";
    if (ext == "cmx") return "image/x-cmx";
    if (ext == "ico") return "image/x-icon";
    if (ext == "mp3") return "audio/mpeg";
    if (ext == "wav") return "audio/x-wav";
    if (ext == "pbm") return "image/x-portable-bitmap";
    if (ext == "ras") return "image/x-cmu-raster";
    if (ext == "txt") return "text/plain";
    if (ext == "xpm") return "image/x-xpixmap";
    if (ext == "ra" || ext == "ram") return "audio/x-pn-realaudio";
    if (ext == "numbers") return "application/x-iwork-numbers-sffnumbers";
    if (ext == "snd" || ext == "au") return "audio/basic";
    if (ext == "zip") return "application/zip";
    if (ext == "pages") return "application/x-iwork-pages-sffpages";
    if (ext == "movie") return "video/x-sgi-movie";
    if (ext == "xwd") return "image/x-xwindowdump";
    if (ext == "pnm") return "image/x-portable-anymap";
    if (ext == "cod") return "image/cis-cod";
    if (ext == "doc" || ext == "docx") return "application/msword";
    if (ext == "html") return "text/html";
    if (ext == "mid" || ext == "rmi") return "audio/mid";
    if (ext == "jpeg" || ext == "jpg" || ext == "jpe") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "bmp") return "image/bmp";
    if (ext == "tiff" || ext == "tif") return "image/tiff";
    if (ext == "gif") return "image/gif";
    if (ext == "svg" || ext == "svgz") return "image/svg+xml";
    if (ext == "pdf") return "application/pdf";
    if (ext == "mov" || ext == "qt") return "video/quicktime";
    if (ext == "mpg" || ext == "mpeg" || ext == "mp2" || ext == "mpe" || ext == "mpa" || ext == "mpv2") return "video/mpeg";
    if (ext == "mp4") return "video/mp4";
    if (ext == "asf" || ext == "asx" || ext == "asr") return "video/x-ms-asf";
    if (ext == "wmv") return "video/x-ms-wmv";
    if (ext == "wvx") return "video/x-ms-wvx";
    if (ext == "wm") return "video/x-ms-wm";
    if (ext == "wmx") return "video/x-ms-wmx";
    if (ext == "avi") return "video/x-msvideo";
    if (ext == "ogv") return "video/ogg";
    if (ext == "flv") return "video/x-flv"; // TODO UB 4.x  ... we need to be smarter ... flash may need an external plugin :-(
    if (ext == "m4v") return "video/x-m4v";
    // W3C widget
    if (ext == "wgt") return "application/widget";
    if (ext == "wgs") return "application/search";
    // Apple widget
    if (ext == "wdgt") return "application/vnd.apple-widget"; //mime type invented by us :-(
    if (ext == "swf") return "application/x-shockwave-flash";

    return "";

}


QString UBFileSystemUtils::fileExtensionFromMimeType(const QString& pMimeType)
{
    // TODO  UB 4.x map from config file, based on a "good" source

    if (pMimeType == "application/msexcel") return "xls";
    if (pMimeType == "application/mspowerpoint") return "ppt";
    if (pMimeType == "image/ief") return "ief";
    if (pMimeType == "audio/x-mpegurl") return "m3u";
    if (pMimeType == "application/x-iwork-keynote-sffkeynote") return "key";
    if (pMimeType == "application/vnd.oasis.opendocument.formula") return "odf";
    if (pMimeType == "audio/x-aiff") return "aif";
    if (pMimeType == "application/vnd.oasis.opendocument.presentation") return "odp";
    if (pMimeType == "application/xml") return "xml";
    if (pMimeType == "image/x-rgb") return "rgb";
    if (pMimeType == "application/vnd.oasis.opendocument.spreadsheet") return "ods";
    if (pMimeType == "text/rtf") return "rtf";
    if (pMimeType == "application/vnd.oasis.opendocument.text") return "odt";
    if (pMimeType == "image/x-xbitmap") return "xbm";
    if (pMimeType == "video/x-la-asf") return "lsx";
    if (pMimeType == "image/pipeg") return "jfif";
    if (pMimeType == "image/x-portable-pixmap") return "ppm";
    if (pMimeType == "text/csv") return "csv";
    if (pMimeType == "image/x-portable-graymap") return "pgm";
    if (pMimeType == "application/vnd.oasis.opendocument.chart") return "odc";
    if (pMimeType == "application/vnd.oasis.opendocument.database") return "odb";
    if (pMimeType == "image/x-cmx") return "cmx";
    if (pMimeType == "image/x-icon") return "ico";
    if (pMimeType == "audio/mpeg") return "mp3";
    if (pMimeType == "audio/x-wav") return "wav";
    if (pMimeType == "image/x-portable-bitmap") return "pbm";
    if (pMimeType == "image/x-cmu-raster") return "ras";
    if (pMimeType == "text/plain") return "txt";
    if (pMimeType == "image/x-xpixmap") return "xpm";
    if (pMimeType == "audio/x-pn-realaudio") return "ram";
    if (pMimeType == "application/x-iwork-numbers-sffnumbers") return "numbers";
    if (pMimeType == "audio/basic") return "snd";
    if (pMimeType == "application/zip") return "zip";
    if (pMimeType == "application/x-iwork-pages-sffpages") return "pages";
    if (pMimeType == "video/x-sgi-movie") return "movie";
    if (pMimeType == "image/x-xwindowdump") return "xwd";
    if (pMimeType == "image/x-portable-anymap") return "pnm";
    if (pMimeType == "image/cis-cod") return "cod";
    if (pMimeType == "application/msword") return "doc";
    if (pMimeType == "text/html") return "html";
    if (pMimeType == "audio/mid") return "mid";
    if (pMimeType == "image/jpeg") return "jpeg";
    if (pMimeType == "image/png") return "png";
    if (pMimeType == "image/bmp") return "bmp";
    if (pMimeType == "image/tiff") return "tiff";
    if (pMimeType == "image/gif") return "gif";
    if (pMimeType == "image/svg+xml") return "svg";
    if (pMimeType == "application/pdf") return "pdf";
    if (pMimeType == "video/quicktime") return "mov";
    if (pMimeType == "video/mpeg") return "mpg";
    if (pMimeType == "video/mp4") return "mp4";
    if (pMimeType == "video/x-ms-asf") return "asf";
    if (pMimeType == "video/x-ms-wmv") return "wmv";
    if (pMimeType == "video/x-ms-wvx") return "wvx";
    if (pMimeType == "video/x-ms-wm") return "wm";
    if (pMimeType == "video/x-ms-wmx") return "wmx";
    if (pMimeType == "video/x-msvideo") return "avi";
    if (pMimeType == "video/ogg") return "ogv";
    if (pMimeType == "video/x-flv") return "flv";
    if (pMimeType == "video/x-m4v") return "m4v";
    if (pMimeType == "application/widget") return "wgt";
    if (pMimeType == "application/vnd.apple-widget") return "wdgt"; //mime type invented by us :-(
    if (pMimeType == "application/x-shockwave-flash") return "swf";

    return "";

}


UBMimeType::Enum UBFileSystemUtils::mimeTypeFromString(const QString& typeString)
{
    UBMimeType::Enum type = UBMimeType::UNKNOWN;

    if (typeString == "image/jpeg"
        || typeString == "image/png"
        || typeString == "image/gif"
        || typeString == "image/tiff"
        || typeString == "image/bmp")
    {
        type = UBMimeType::RasterImage;
    }
    else if (typeString == "image/svg+xml")
    {
        type = UBMimeType::VectorImage;
    }
    else if (typeString == "application/vnd.apple-widget")
    {
        type = UBMimeType::AppleWidget;
    }
    else if (typeString == "application/widget")
    {
        type = UBMimeType::W3CWidget;
    }
    else if (typeString.startsWith("video/"))
    {
        type = UBMimeType::Video;
    }
    else if (typeString.startsWith("audio/"))
    {
        type = UBMimeType::Audio;
    }
    else if (typeString.startsWith("application/x-shockwave-flash"))
    {
        type = UBMimeType::Flash;
    }
    else if (typeString.startsWith("application/pdf"))
    {
        type = UBMimeType::PDF;
    }
   /* else if (typeString.startsWith("application/vnd.mnemis-uniboard-tool"))
    {
        type = UBMimeType::UniboardTool;
    } */

    else if (typeString.startsWith("application/openboard-tool"))
    {
        type = UBMimeType::OpenboardTool;
    }
    return type;

}

UBMimeType::Enum UBFileSystemUtils::mimeTypeFromUrl(const QUrl& url)
{
    return mimeTypeFromString(mimeTypeFromFileName(url.toString()));
}

QString UBFileSystemUtils::getFirstExistingFileFromList(const QString& path, const QStringList& files)
{

    QString fullpath = path;

    if (!path.endsWith("/"))
    {
        fullpath += "/";
    }

    foreach(QString filename, files)
    {
        QFile file;

        file.setFileName(fullpath + filename);

        if (file.exists())
        {
            return fullpath + filename;
        }
    }

    return "";

}


bool UBFileSystemUtils::compressDirInZip(const QDir& pDir, const QString& pDestPath, QuaZipFile *pOutZipFile, bool pRootDocumentFolder, UBProcessingProgressListener* progressListener)
{
    QFileInfoList files = pDir.entryInfoList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    QStringList filters;
    filters << "*.svg";
    QFileInfoList pageFiles = pDir.entryInfoList(filters);

    foreach (QFileInfo file, files)
    {
        if (file.isDir())
        {
            QDir dir(file.absoluteFilePath());
            if (!compressDirInZip(dir, pDestPath + dir.dirName() + "/" , pOutZipFile, false))
            {
                return false;
            }
        }

        if (file.isFile())
        {
            QString objectType;
            if (pRootDocumentFolder)
            {
                objectType = "Page";
            }
            else
            {
                objectType = pDir.dirName();
            }

            if (!pRootDocumentFolder)
            {
                if (progressListener)
                    progressListener->processing(objectType, files.indexOf(file), files.size());
            }
            // we ignore thumbnails message because it is very fast.
            else if (progressListener && file.suffix() == "svg")
            {
                progressListener->processing(objectType, pageFiles.indexOf(file), pageFiles.size());
            }

            QFile inFile(file.absoluteFilePath());
            if(!inFile.open(QIODevice::ReadOnly))
            {
                qWarning() << "Compression of file" << inFile.fileName() << " failed. Cause: inFile.open(): " << inFile.errorString();
                return false;
            }

            qDebug() << "will open" << pDestPath << file.fileName() << inFile.fileName();

            if(!pOutZipFile->open(QIODevice::WriteOnly, QuaZipNewInfo(pDestPath + file.fileName(), inFile.fileName())))
            {
                qWarning() << "Compression of file" << inFile.fileName() << " failed. Cause: outFile.open(): " << pOutZipFile->getZipError();
                inFile.close();
                return false;
            }

            pOutZipFile->write(inFile.readAll());
            if(pOutZipFile->getZipError() != UNZ_OK)
            {
                qWarning() << "Compression of file" << inFile.fileName() << " failed. Cause: outFile.write(): " << pOutZipFile->getZipError();

                inFile.close();
                pOutZipFile->close();
                return false;
            }

            pOutZipFile->close();
            if(pOutZipFile->getZipError() != UNZ_OK)
            {
                qWarning() << "Compression of file" << inFile.fileName() << " failed. Cause: outFile.close(): " << pOutZipFile->getZipError();

                inFile.close();
                return false;
            }

            inFile.close();
        }
    }

    return true;
}



bool UBFileSystemUtils::expandZipToDir(const QFile& pZipFile, const QDir& pTargetDir)
{
    QuaZip zip(pZipFile.fileName());

    if(!zip.open(QuaZip::mdUnzip))
    {
        qWarning() << "ZIP expand failed. Cause zip.open(): " << zip.getZipError();
        return false;
    }

    zip.setFileNameCodec("UTF-8");
    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    QString documentRootFolder = pTargetDir.absolutePath();

    if(!pTargetDir.exists())
        pTargetDir.mkpath(documentRootFolder);

    QFile out;
    char c;
    for(bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {
        if(!zip.getCurrentFileInfo(&info))
        {
            //TOD UB 4.3 O display error to user or use crash reporter
            qWarning() << "ZIP expand failed. Cause: getCurrentFileInfo(): " << zip.getZipError();
            return false;
        }

        if(!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "ZIP expand failed. Cause: file.open(): " << zip.getZipError();
            return false;
        }

        if(file.getZipError()!= UNZ_OK)
        {
            qWarning() << "ZIP expand failed. Cause: file.getFileName(): " << zip.getZipError();
            return false;
        }

        QString newFileName = documentRootFolder + "/" + file.getActualFileName();
        QFileInfo newFileInfo(newFileName);
        QDir root(documentRootFolder);
        root.mkpath(newFileInfo.absolutePath());

        out.setFileName(newFileName);
        out.open(QIODevice::WriteOnly);

        // Slow like hell (on GNU/Linux at least), but it is not my fault.
        // Not ZIP/UNZIP package's fault either.
        // The slowest thing here is out.putChar(c).
        QByteArray outFileContent = file.readAll();
        if (out.write(outFileContent) == -1)
        {
            // qWarning() << "ZIP expand failed. Cause: Unable to write file";
            // this may happen if we are decompressing a directory
        }

        while(file.getChar(&c))
            out.putChar(c);

        out.close();

        if(file.getZipError()!= UNZ_OK)
        {
            qWarning() << "ZIP expand failed. Cause: " << zip.getZipError();
            return false;
        }

        if(!file.atEnd())
        {
            qWarning() << "ZIP expand failed. Cause: read all but not EOF";
            return false;
        }

        file.close();

        if(file.getZipError()!= UNZ_OK)
        {
            qWarning() << "ZIP expand failed. Cause: file.close(): " <<  file.getZipError();
            return false;
        }

    }

    zip.close();

    if(zip.getZipError()!= UNZ_OK)
    {
      qWarning() << "ZIP expand failed. Cause: zip.close(): " << zip.getZipError();
      return false;
    }

    return true;
}


QString UBFileSystemUtils::md5InHex(const QByteArray &pByteArray)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, pByteArray.data(), pByteArray.size());

    unsigned char result[16];
    MD5_Final(result, &ctx);

    return QString(QByteArray((char *)result, 16).toHex());
}

QString UBFileSystemUtils::md5(const QByteArray &pByteArray)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, pByteArray.data(), pByteArray.size());

    unsigned char result[16];
    MD5_Final(result, &ctx);
    QString s;

    for(int i = 0; i < 16; i++)
    {
        s += QChar(result[i]);
    }

    return s;
}

QString UBFileSystemUtils::readTextFile(QString path)
{
    QFile file(path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        return in.readAll();
    }

    return "";
}
