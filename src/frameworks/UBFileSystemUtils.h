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




#ifndef UBFILESYSTEMUTILS_H_
#define UBFILESYSTEMUTILS_H_

#include <QtCore>
#include <QThread>

#include "core/UB.h"

class QuaZipFile;
class UBProcessingProgressListener;

class UBFileSystemUtils : public QObject
{
    Q_OBJECT

    public:

        UBFileSystemUtils();
        virtual ~UBFileSystemUtils();

        static QString removeLocalFilePrefix(QString input);

        static QString defaultTempDirName() { return QCoreApplication::applicationName(); }
        static QString defaultTempDirPath();
        static QString createTempDir(const QString& templateString = defaultTempDirName(), bool autoDeleteOnExit = true);
        static void cleanupGhostTempFolders(const QString& templateString = defaultTempDirName());

        static void deleteAllTempDirCreatedDuringSession();

        static QFileInfoList allElementsInDirectory(const QString& pDirPath);

        static QStringList allFiles(const QString& pDirPath, const bool isRecurive=true);

        static bool deleteDir(const QString& pDirPath);

        static bool copyDir(const QString& pSourceDirPath, const QString& pTargetDirPath, bool overwrite = false);

        static bool moveDir(const QString& pSourceDirPath, const QString& pTargetDirPath);

        static bool copyFile(const QString &source, const QString &destination, bool overwrite = false);

        static bool copy(const QString &source, const QString &Destination, bool overwrite = false);

        static QString cleanName(const QString& name);

        static QString digitFileFormat(const QString& s, int digit);

        static QString thumbnailPath(const QString& path);

        static QString mimeTypeFromFileName(const QString& filename);

        static QString fileExtensionFromMimeType(const QString& pMimeType);

        static UBMimeType::Enum mimeTypeFromString(const QString& typeString);

        static UBMimeType::Enum mimeTypeFromUrl(const QUrl& url);

        static QString normalizeFilePath(const QString& pFilePath);

        static QString extension(const QString& filaname);

        static QString lastPathComponent(const QString& path);

        static QString getFirstExistingFileFromList(const QString& path, const QStringList& files);

        static bool isAZipFile(QString &filePath);

        static bool deleteFile(const QString &path);
        /**
         * Compress a source directory in a zip file.
         * @arg pDir the directory to add in zip
         * @arg pDestPath the path inside the zip. Attention, if path is not empty it must end by a /.
         * @arg pOutZipFile the zip file we want to populate with the directory
         * @arg UBProcessingProgressListener an object listening to the compression progress
         * @return bool. true if compression is successful.
         */
        static bool compressDirInZip(const QDir& pDir, const QString& pDestDir, QuaZipFile *pOutZipFile
                        , bool pRootDocumentFolder, UBProcessingProgressListener* progressListener = 0);

        static bool expandZipToDir(const QFile& pZipFile, const QDir& pTargetDir);

        static QString md5InHex(const QByteArray &pByteArray);
        static QString md5(const QByteArray &pByteArray);

        static QString nextAvailableFileName(const QString& filename, const QString& inter = QString(""));

        static QString readTextFile(QString path);

    private:
        static QStringList sTempDirToCleanUp;

};


class UBProcessingProgressListener
{

    public:
        UBProcessingProgressListener()
        {
            //NOOP
        }

        virtual ~UBProcessingProgressListener()
        {
            //NOOP
        }

        virtual void processing(const QString& pOpType, int pCurrent, int pTotal) = 0;

};

#endif /* UBFILESYSTEMUTILS_H_ */
