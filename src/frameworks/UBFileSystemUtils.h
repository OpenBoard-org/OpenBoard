/*
 * UBFileSystemUtils.h
 *
 *  Created on: Dec 5, 2008
 *      Author: Luc
 */

#ifndef UBFILESYSTEMUTILS_H_
#define UBFILESYSTEMUTILS_H_

#include <QtCore>

class QuaZipFile;
class UBProcessingProgressListener;

class UBFileSystemUtils
{
    public:

        UBFileSystemUtils();
        virtual ~UBFileSystemUtils();

        static QString defaultTempDirName() { return QCoreApplication::applicationName(); }
        static QString defaultTempDirPath();
        static QString createTempDir(const QString& templateString = defaultTempDirName(), bool autoDeleteOnExit = true);
        static void cleanupGhostTempFolders(const QString& templateString = defaultTempDirName());

        static void deleteAllTempDirCreatedDuringSession();

        static QFileInfoList allElementsInDirectory(const QString& pDirPath);

        static QStringList allFiles(const QString& pDirPath);

        static bool deleteDir(const QString& pDirPath);

        static bool copyDir(const QString& pSourceDirPath, const QString& pTargetDirPath);

        static bool moveDir(const QString& pSourceDirPath, const QString& pTargetDirPath);

        static QString cleanName(const QString& name);

        static QString digitFileFormat(const QString& s, int digit);

        static QString thumbnailPath(const QString& path);

        static QString mimeTypeFromFileName(const QString& filename);

        static QString fileExtensionFromMimeType(const QString& pMimeType);

        static QString normalizeFilePath(const QString& pFilePath);

        static QString extension(const QString& filaname);

        static QString lastPathComponent(const QString& path);

        static QString getFirstExistingFileFromList(const QString& path, const QStringList& files);

        static bool isAZipFile(QString &filePath);

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
