/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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

#include "UBImportDocumentSetAdaptor.h"

#include "document/UBDocumentProxy.h"

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBPersistenceManager.h"

#include "globals/UBGlobals.h"

#ifdef Q_OS_WIN
    #include <quazip.h>
    #include <quazipfile.h>
    #include <quazipfileinfo.h>
#else
#include "quazip.h"
    #include "quazipfile.h"
    #include "quazipfileinfo.h"
#endif

#include "core/memcheck.h"

UBImportDocumentSetAdaptor::UBImportDocumentSetAdaptor(QObject *parent)
    :UBImportAdaptor(parent)
{
    // NOOP
}

UBImportDocumentSetAdaptor::~UBImportDocumentSetAdaptor()
{
    // NOOP
}


QStringList UBImportDocumentSetAdaptor::supportedExtentions()
{
    return QStringList("ubx");
}


QString UBImportDocumentSetAdaptor::importFileFilter()
{
    return tr("Openboard (set of documents) (*.ubx)");
}

QFileInfoList UBImportDocumentSetAdaptor::importData(const QString &zipFile, const QString &destination)
{
    //Create tmp directory to extract data, will be deleted after
    QString tmpDir;
    int i = 0;
    QFileInfoList result;
    do {
      tmpDir = QDir::tempPath() + "/Sankore_tmpImportUBX_" + QString::number(i++);
    } while (QFileInfo(tmpDir).exists());

    QDir(QDir::tempPath()).mkdir(tmpDir);

    QFile fZipFile(zipFile);

    if (!extractFileToDir(fZipFile, tmpDir)) {
        UBFileSystemUtils::deleteDir(tmpDir);
        return QFileInfoList();
    }

    QDir tDir(tmpDir);

    foreach(QFileInfo readDir, tDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden , QDir::Name)) {
        QString newFileName = readDir.fileName();
        if (QFileInfo(destination + "/" + newFileName).exists())
        {
            //if the generateUniqueDocumentPath is called twice in the same millisecond, the destination files are overwritten
            do
            {
                newFileName = QFileInfo(UBPersistenceManager::persistenceManager()->generateUniqueDocumentPath(tmpDir)).fileName();
            } while (QFileInfo(destination + "/" + newFileName).exists());
        }

        QString newFilePath = destination + "/" + newFileName;
        if (UBFileSystemUtils::copy(readDir.absoluteFilePath(), newFilePath, true)) {
            result.append(QFileInfo(newFilePath));
        }
    }

    UBFileSystemUtils::deleteDir(tmpDir);

    return result;
}


bool UBImportDocumentSetAdaptor::extractFileToDir(const QFile& pZipFile, const QString& pDir)
{
    QDir rootDir(pDir);
    QuaZip zip(pZipFile.fileName());

    if(!zip.open(QuaZip::mdUnzip))
    {
        qWarning() << "Import failed. Cause zip.open(): " << zip.getZipError();
        return false;
    }

    zip.setFileNameCodec("UTF-8");
    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    QFile out;
    char c;

    QString documentRoot = QFileInfo(pDir).absoluteFilePath();
    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
    {
        if(!zip.getCurrentFileInfo(&info))
        {
            //TOD UB 4.3 O display error to user or use crash reporter
            qWarning() << "Import failed. Cause: getCurrentFileInfo(): " << zip.getZipError();
            return false;
        }

        if(!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Import failed. Cause: file.open(): " << zip.getZipError();
            return false;
        }

        if(file.getZipError()!= UNZ_OK)
        {
            qWarning() << "Import failed. Cause: file.getFileName(): " << zip.getZipError();
            return false;
        }

        QString actFileName = file.getActualFileName();
//        int ind = actFileName.indexOf("/");
//        if ( ind!= -1) {
//            actFileName.remove(0, ind + 1);
//        }
        QString newFileName = documentRoot + "/" + actFileName;
        QFileInfo newFileInfo(newFileName);
        if (!rootDir.mkpath(newFileInfo.absolutePath()))
            return false;

        out.setFileName(newFileName);
        if (!out.open(QIODevice::WriteOnly))
            return false;

        // Slow like hell (on GNU/Linux at least), but it is not my fault.
        // Not ZIP/UNZIP package's fault either.
        // The slowest thing here is out.putChar(c).
        QByteArray outFileContent = file.readAll();
        if (out.write(outFileContent) == -1)
        {
            qWarning() << "Import failed. Cause: Unable to write file";
            out.close();
            return false;
        }

        while(file.getChar(&c))
            out.putChar(c);

        out.close();

        if(file.getZipError()!=UNZ_OK)
        {
            qWarning() << "Import failed. Cause: " << zip.getZipError();
            return false;
        }

        if(!file.atEnd())
        {
            qWarning() << "Import failed. Cause: read all but not EOF";
            return false;
        }

        file.close();

        if(file.getZipError()!=UNZ_OK)
        {
            qWarning() << "Import failed. Cause: file.close(): " <<  file.getZipError();
            return false;
        }

    }

    zip.close();

    if(zip.getZipError()!=UNZ_OK)
    {
      qWarning() << "Import failed. Cause: zip.close(): " << zip.getZipError();
      return false;
    }

    return true;
}
