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



#include <QDir>
#include <QList>
#include <QXmlStreamReader>

#include "adaptors/UBSvgSubsetAdaptor.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBDocumentManager.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "frameworks/UBFileSystemUtils.h"

#include "UBSvgSubsetAdaptor.h"

#include "UBImportSMART.h"

#include "globals/UBGlobals.h"

THIRD_PARTY_WARNINGS_DISABLE
#include "quazip.h"
#include "quazipfile.h"
#include "quazipfileinfo.h"
THIRD_PARTY_WARNINGS_ENABLE

#include "core/memcheck.h"

UBImportSMART::UBImportSMART(QObject *parent)
    : UBDocumentBasedImportAdaptor(parent)
{
    // NOOP
}


UBImportSMART::~UBImportSMART()
{
    // NOOP
}


QStringList UBImportSMART::supportedExtentions()
{
    QStringList formats = {"notebook", "xbk"};
    return formats;
}


QString UBImportSMART::importFileFilter()
{
    QString filter = tr("SMART Notebook (");
    QStringList formats = supportedExtentions();
    bool isFirst = true;

    for (const QString &format : formats)
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

QString UBImportSMART::expandFileToDir(const QFile& pZipFile, const QString& pDir)
{
    QuaZip zip(pZipFile.fileName());

    if(!zip.open(QuaZip::mdUnzip)) {
        qWarning() << "Import failed. Cause zip.open(): " << zip.getZipError();
        return "";
    }

    zip.setFileNameCodec("UTF-8");
    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    //create temporary document root folder
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


void UBImportSMART::importSinglePage(UBDocumentProxy* document,
                                     QByteArray xml)
{
    int pageIndex = document->pageCount();

    UBGraphicsScene* scene = UBSvgSubsetAdaptor::loadScene(document, xml);
    UBApplication::showMessage(tr("Inserting page %1").arg(pageIndex), true);

    // Adjust coordinates of scene items so (0,0) is the
    // very centre of the page.
    qreal adjust_x = scene->width() / 2;
    qreal adjust_y = scene->height() / 2;

    // Do not use the original height of the page, as the
    // page is scaled to appear on the screen.  Use a 4:3 ratio
    // instead so the page is large enough to read.
    qreal newHeight = scene->width() * 0.75;
    adjust_y -= 0.5 * (scene->height() - newHeight);

    const auto sceneItems = scene->items();
    for (QGraphicsItem* item : sceneItems)
    {
        if (nullptr == item->parentItem() && item->isVisible())
            item->setPos(item->x() - adjust_x, item->y() - adjust_y);
      // I don't know why but without the isVisible() check
      // mouse pointer movement is broken.  There must be an
      // invisible item that is used for pointer positioning.
    }

    QRectF rect = scene->sceneRect();
    rect.setHeight(newHeight);
    scene->setSceneRect(rect);

    QSize sceneSize;
    sceneSize.setWidth(rect.width());
    sceneSize.setHeight(newHeight);
    scene->setNominalSize(sceneSize);

    UBPersistenceManager::persistenceManager()->insertDocumentSceneAt(document, scene, pageIndex);
}

UBDocumentProxy* UBImportSMART::importFile(const QFile& pFile, const QString& pGroup)
{
    QList<UBGraphicsScene*> pages;

    QFileInfo fi(pFile);
    UBApplication::showMessage(tr("Importing file %1...").arg(fi.baseName()), true);

    // first unzip the file to the correct place
    QString path = QDir::tempPath();

    QString documentRootFolder = expandFileToDir(pFile, path);
    QString contentFile;

    QString documentName = QFileInfo(pFile.fileName()).completeBaseName();
    UBDocumentProxy* document = UBPersistenceManager::persistenceManager()->createDocument(pGroup, documentName, false, QString(), 0, true);

    QString documentPath = document->persistencePath();

    if (documentRootFolder.isEmpty())
    {
        QFile file(pFile.fileName());
        // If file has failed to unzip it may be .xbk extension (e.g.
        // SMART Notebook 8.0) with several <svg> elements.
        QDomDocument doc("smart-import");
        if (!doc.setContent(&file)) {
            file.close();
            return 0;
        }
        file.close();
        QDomNodeList pageList = doc.elementsByTagName("svg");
        int length = pageList.length();
        for (int pageIndex = 0; pageIndex < length; pageIndex++)
        {
            QDomNode node = pageList.at(pageIndex);
            QByteArray xml;
            QTextStream xmlWriter(&xml);
            node.save(xmlWriter, 0);

            importSinglePage(document, xml);
        }
    }
    else
    {
        //get path to imsmanifest xml
        contentFile = QString("%1/imsmanifest.xml").arg(documentRootFolder);

        if (!contentFile.length())
        {
            UBApplication::showMessage(tr("Import of file %1 failed.").arg(fi.baseName()));
            return 0;
        }
        QFile file(contentFile);

        if (!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Cannot open file " << contentFile << " for reading ...";
            return 0;
        }

        QDomDocument doc("smart-import");
        if (!doc.setContent(&file)) {
            file.close();
            return 0;
        }
        file.close();

        QDir imgDir(documentRootFolder + "/images");
        if (imgDir.exists())
        {
            if (!UBFileSystemUtils::copyDir(documentRootFolder + "/images",
                     documentPath + "/images"))
                return 0;
        }

        // In imsmanifest.xml, pages are named in the href attributes
        // of <file> elements.  They may be named more than once by elements
        // contained by different <resource> elements, so only import each
        // page the first time it is seen.
        QDomNodeList filelist = doc.elementsByTagName("file");
        QHash<QString, bool> fileSeen;

        int length = filelist.length();
        for (int fileIndex = 0; fileIndex < length; fileIndex++)
        {
            QDomNode node = filelist.at(fileIndex);
            QDomElement element = node.toElement();
            QString base = element.attribute("href");
            if (!base.isEmpty() && base.endsWith(".svg") && !fileSeen[base])
            {
                fileSeen[base] = true;
                QString pageFile;
                pageFile = QString("%1/%2").arg(documentRootFolder).arg(base);
                QFile file(pageFile);
                if (!file.open(QIODevice::ReadOnly))
                    return 0;

                QByteArray xml = file.readAll();
                importSinglePage(document, xml);
            }
        }
    }

    UBPersistenceManager::persistenceManager()->persistDocumentMetadata(document);
    UBApplication::showMessage(tr("Import successful."));
    UBFileSystemUtils::deleteDir(documentRootFolder);
    return document;
}

bool UBImportSMART::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
  qWarning() << "addFileToDocument not implemented";
  return false;
}
