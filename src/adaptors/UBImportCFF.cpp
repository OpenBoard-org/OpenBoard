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



#include <QDir>
#include <QList>

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBDocumentManager.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsPDFItem.h"
#include "frameworks/UBFileSystemUtils.h"
#include "pdf/PDFRenderer.h"

#include "UBCFFSubsetAdaptor.h"
#include "UBImportCFF.h"

#include "globals/UBGlobals.h"

//THIRD_PARTY_WARNINGS_DISABLE
#ifdef Q_OS_OSX
    #include <quazip.h>
    #include <quazipfile.h>
    #include <quazipfileinfo.h>
#else
    #include "quazip.h"
    #include "quazipfile.h"
    #include "quazipfileinfo.h"
#endif
//THIRD_PARTY_WARNINGS_ENABLE

#include "core/memcheck.h"

UBImportCFF::UBImportCFF(QObject *parent)
    : UBDocumentBasedImportAdaptor(parent)
{
    // NOOP
}


UBImportCFF::~UBImportCFF()
{
    // NOOP
}


QStringList UBImportCFF::supportedExtentions()
{
    return QStringList("iwb");
}


QString UBImportCFF::importFileFilter()
{
    QString filter = tr("Common File Format (");
    QStringList formats = supportedExtentions();
    bool isFirst = true;

    foreach(QString format, formats)
    {
            if(isFirst)
                    isFirst = false;
            else
                    filter.append(" ");

        filter.append("*."+format);
    }

    filter.append(")");

    return filter;
}

bool UBImportCFF::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
    QFileInfo fi(pFile);
    UBApplication::showMessage(tr("Importing file %1...").arg(fi.baseName()), true);

    // first unzip the file to the correct place
    //TODO create temporary path for iwb file content
    QString path = QDir::tempPath();

    QString documentRootFolder = expandFileToDir(pFile, path);
        QString contentFile;
    if (documentRootFolder.isEmpty()) //if file has failed to unzip it is probably just xml file
        contentFile = pFile.fileName();
    else //get path to content xml (according to iwbcff specification)
        contentFile = documentRootFolder.append("/content.xml");

    if(!contentFile.length()){
            UBApplication::showMessage(tr("Import of file %1 failed.").arg(fi.baseName()));
            return false;
    }
    else{
        //TODO convert expanded CFF file content to the destination document
        //create destination document proxy
        //fill metadata and save
        UBDocumentProxy* destDocument = new UBDocumentProxy(UBPersistenceManager::persistenceManager()->generateUniqueDocumentPath());
        QDir dir;
        dir.mkdir(destDocument->persistencePath());

        //try to import cff to document
        if (UBCFFSubsetAdaptor::ConvertCFFFileToUbz(contentFile, destDocument))
        {
            UBPersistenceManager::persistenceManager()->addDirectoryContentToDocument(destDocument->persistencePath(), pDocument);
            UBFileSystemUtils::deleteDir(destDocument->persistencePath());
            delete destDocument;
            UBApplication::showMessage(tr("Import successful."));
            return true;
        }
        else
        {
            UBFileSystemUtils::deleteDir(destDocument->persistencePath());
            delete destDocument;
            UBApplication::showMessage(tr("Import failed."));
            return false;
        }
    }
}

QString UBImportCFF::expandFileToDir(const QFile& pZipFile, const QString& pDir)
{
    QuaZip zip(pZipFile.fileName());

    if(!zip.open(QuaZip::mdUnzip)) {
        qWarning() << "Import failed. Cause zip.open(): " << zip.getZipError();
        return "";
    }

    zip.setFileNameCodec("UTF-8");
    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    //create unique cff document root fodler
    //use current date/time and temp number for folder name
    QString documentRootFolder;
    int tmpNumber = 0;
    QDir rootDir;
    while (true) {
        QString tempPath = QString("%1/sank%2.%3")
                .arg(pDir)
                .arg(QDateTime::currentDateTime().toString("dd_MM_yyyy_HH-mm"))
                .arg(tmpNumber);
        if (!rootDir.exists(tempPath)) {
            documentRootFolder = tempPath;
            break;
        }
        tmpNumber++;
        if (tmpNumber == 100000) {
            qWarning() << "Import failed. Failed to create temporary directory for iwb file";
            return "";
        }
    }
    if (!rootDir.mkdir(documentRootFolder)) {
        qWarning() << "Import failed. Couse: failed to create temp folder for cff package";
    }

    QFile out;
    char c;
    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
        if(!zip.getCurrentFileInfo(&info)) {
            //TOD UB 4.3 O display error to user or use crash reporter
            qWarning() << "Import failed. Cause: getCurrentFileInfo(): " << zip.getZipError();
            return "";
        }
//        if(!file.open(QIODevice::ReadOnly)) {
//            qWarning() << "Import failed. Cause: file.open(): " << zip.getZipError();
//            return "";
//        }
        file.open(QIODevice::ReadOnly);
        if(file.getZipError()!= UNZ_OK) {
            qWarning() << "Import failed. Cause: file.getFileName(): " << zip.getZipError();
            return "";
        }

        QString newFileName = documentRootFolder + "/" + file.getActualFileName();

        QFileInfo newFileInfo(newFileName);
        rootDir.mkpath(newFileInfo.absolutePath());

        out.setFileName(newFileName);
        out.open(QIODevice::WriteOnly);

        while(file.getChar(&c))
            out.putChar(c);

        out.close();

        if(file.getZipError()!=UNZ_OK) {
            qWarning() << "Import failed. Cause: " << zip.getZipError();
            return "";
        }
        if(!file.atEnd()) {
            qWarning() << "Import failed. Cause: read all but not EOF";
            return "";
        }

        file.close();

        if(file.getZipError()!=UNZ_OK) {
            qWarning() << "Import failed. Cause: file.close(): " <<  file.getZipError();
            return "";
        }
    }

    zip.close();

    if(zip.getZipError()!=UNZ_OK) {
        qWarning() << "Import failed. Cause: zip.close(): " << zip.getZipError();
        return "";
    }

    return documentRootFolder;
}


UBDocumentProxy* UBImportCFF::importFile(const QFile& pFile, const QString& pGroup)
{
    Q_UNUSED(pGroup); // group is defined in the imported file

    QFileInfo fi(pFile);
    UBApplication::showMessage(tr("Importing file %1...").arg(fi.baseName()), true);

    // first unzip the file to the correct place
    //TODO create temporary path for iwb file content
    QString path = QDir::tempPath();

    QString documentRootFolder = expandFileToDir(pFile, path);
    QString contentFile;
    if (documentRootFolder.isEmpty())
        //if file has failed to umzip it is probably just xml file
        contentFile = pFile.fileName();
    else
        //get path to content xml
        contentFile = QString("%1/content.xml").arg(documentRootFolder);

    if(!contentFile.length()){
            UBApplication::showMessage(tr("Import of file %1 failed.").arg(fi.baseName()));
            return 0;
    }
    else{
        //create destination document proxy
        //fill metadata and save
        UBDocumentProxy* destDocument = new UBDocumentProxy(UBPersistenceManager::persistenceManager()->generateUniqueDocumentPath());
        QDir dir;
        dir.mkdir(destDocument->persistencePath());
        if (pGroup.length() > 0)
            destDocument->setMetaData(UBSettings::documentGroupName, pGroup);
        if (!fi.baseName().isEmpty())
            destDocument->setMetaData(UBSettings::documentName, fi.baseName());

        destDocument->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);
        destDocument->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));

        UBDocumentProxy* newDocument = NULL;
        //try to import cff to document
        if (UBCFFSubsetAdaptor::ConvertCFFFileToUbz(contentFile, destDocument))
        {
            newDocument = UBPersistenceManager::persistenceManager()->createDocumentFromDir(destDocument->persistencePath()
                                                                                            ,""
                                                                                            ,""
                                                                                            ,false
                                                                                            ,false
                                                                                            ,true);

            UBApplication::showMessage(tr("Import successful."));
        }
        else
        {
            UBFileSystemUtils::deleteDir(destDocument->persistencePath());
            UBApplication::showMessage(tr("Import failed."));
        }
        delete destDocument;

        if (documentRootFolder.length() != 0)
            UBFileSystemUtils::deleteDir(documentRootFolder);
        return newDocument;
    }
}
