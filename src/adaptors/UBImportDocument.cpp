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




#include "UBImportDocument.h"
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

UBImportDocument::UBImportDocument(QObject *parent)
    :UBDocumentBasedImportAdaptor(parent)
{
    // NOOP
}

UBImportDocument::~UBImportDocument()
{
    // NOOP
}


QStringList UBImportDocument::supportedExtentions()
{
    return QStringList("ubz");
}


QString UBImportDocument::importFileFilter()
{
    return tr("OpenBoard (*.ubz)");
}


bool UBImportDocument::extractFileToDir(const QFile& pZipFile, const QString& pDir, QString& documentRoot)
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
    documentRoot = UBPersistenceManager::persistenceManager()->generateUniqueDocumentPath(pDir);
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

        QString newFileName = documentRoot + "/" + file.getActualFileName();
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

UBDocumentProxy* UBImportDocument::importFile(const QFile& pFile, const QString& pGroup)
{
    Q_UNUSED(pGroup); // group is defined in the imported file

    QFileInfo fi(pFile);
    UBApplication::showMessage(tr("Importing file %1...").arg(fi.baseName()), true);

    // first unzip the file to the correct place
    QString path = UBSettings::userDocumentDirectory();

    QString documentRootFolder;

    if(!extractFileToDir(pFile, path, documentRootFolder)){
        UBApplication::showMessage(tr("Import of file %1 failed.").arg(fi.baseName()));
        return NULL;
    }

    UBDocumentProxy* newDocument = UBPersistenceManager::persistenceManager()->createDocumentFromDir(documentRootFolder, pGroup, "", false, false, true);
    UBApplication::showMessage(tr("Import successful."));
    return newDocument;
}

bool UBImportDocument::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
    QFileInfo fi(pFile);
    UBApplication::showMessage(tr("Importing file %1...").arg(fi.baseName()), true);

    QString path = UBFileSystemUtils::createTempDir();

    QString documentRootFolder;
    if (!extractFileToDir(pFile, path, documentRootFolder))
    {
        UBApplication::showMessage(tr("Import of file %1 failed.").arg(fi.baseName()));
        return false;
    }

    if (!UBPersistenceManager::persistenceManager()->addDirectoryContentToDocument(documentRootFolder, pDocument))
    {
        UBApplication::showMessage(tr("Import of file %1 failed.").arg(fi.baseName()));
        return false;
    }

    UBFileSystemUtils::deleteDir(path);

    UBApplication::showMessage(tr("Import successful."));

    return true;
}


