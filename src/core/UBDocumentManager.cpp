/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBDocumentManager.h"

#include "frameworks/UBStringUtils.h"

#include "adaptors/UBExportFullPDF.h"
#include "adaptors/UBExportDocument.h"
#include "adaptors/UBExportWeb.h"
#include "adaptors/UBExportCFF.h"
#include "adaptors/UBWebPublisher.h"
#include "adaptors/UBImportDocument.h"
#include "adaptors/UBImportPDF.h"
#include "adaptors/UBImportImage.h"
#include "adaptors/UBImportCFF.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"

#include "document/UBDocumentProxy.h"

#include "UBApplication.h"
#include "UBSettings.h"
#include "UBPersistenceManager.h"

#include "../adaptors/UBExportWeb.h"

#include "core/memcheck.h"

UBDocumentManager* UBDocumentManager::sDocumentManager = 0;

UBDocumentManager* UBDocumentManager::documentManager()
{
    if (!sDocumentManager)
    {
        sDocumentManager = new UBDocumentManager(qApp);
    }
    return sDocumentManager;
}


UBDocumentManager::UBDocumentManager(QObject *parent)
    :QObject(parent)
{
    // TODO UB 4.7 string used in document persistence (folder names)
    QString dummyImages = tr("images");
    QString dummyVideos = tr("videos");
    QString dummyObjects = tr("objects");
    QString dummyWidgets = tr("widgets");

    UBExportCFF* cffExporter = new UBExportCFF(this);
    mExportAdaptors.append(cffExporter);
    UBExportFullPDF* exportFullPdf = new UBExportFullPDF(this);
    mExportAdaptors.append(exportFullPdf);
    UBExportDocument* exportDocument = new UBExportDocument(this);
    mExportAdaptors.append(exportDocument);
//     UBExportWeb* exportWeb = new UBExportWeb(this);
//     mExportAdaptors.append(exportWeb);
    UBWebPublisher* webPublished = new UBWebPublisher(this);
    mExportAdaptors.append(webPublished);

    UBImportDocument* documentImport = new UBImportDocument(this);
    mImportAdaptors.append(documentImport);
    UBImportPDF* pdfImport = new UBImportPDF(this);
    mImportAdaptors.append(pdfImport);
    UBImportImage* imageImport = new UBImportImage(this);
    mImportAdaptors.append(imageImport);
    UBImportCFF* cffImport = new UBImportCFF(this);
    mImportAdaptors.append(cffImport);
}


UBDocumentManager::~UBDocumentManager()
{
    // NOOP
}


QStringList UBDocumentManager::importFileExtensions()
{
    QStringList result;

    foreach (UBImportAdaptor *importAdaptor, mImportAdaptors)
    {
        result << importAdaptor->supportedExtentions();
    }
    return result;
}


QString UBDocumentManager::importFileFilter()
{
    QString result;

    result += tr("All supported files (*.%1)").arg(importFileExtensions().join(" *."));
    foreach (UBImportAdaptor *importAdaptor, mImportAdaptors)
    {
        if (importAdaptor->importFileFilter().length() > 0)
        {
            if (result.length())
            {
                result += ";;";
            }
            result += importAdaptor->importFileFilter();
        }
    }
    qDebug() << "import file filter" << result;
    return result;
}


UBDocumentProxy* UBDocumentManager::importFile(const QFile& pFile, const QString& pGroup)
{
    QFileInfo fileInfo(pFile);

    foreach (UBImportAdaptor *adaptor, mImportAdaptors)
    {
        if (adaptor->supportedExtentions().lastIndexOf(fileInfo.suffix().toLower()) != -1)
        {
            UBDocumentProxy* document;
            UBApplication::setDisabled(true);

            if (adaptor->isDocumentBased())
            {
                UBDocumentBasedImportAdaptor* importAdaptor = (UBDocumentBasedImportAdaptor*)adaptor;

                document = importAdaptor->importFile(pFile, pGroup);
            
            }
            else
            {
                UBPageBasedImportAdaptor* importAdaptor = (UBPageBasedImportAdaptor*)adaptor;

                // Document import procedure.....
                QString documentName = QFileInfo(pFile.fileName()).completeBaseName();
                document = UBPersistenceManager::persistenceManager()->createDocument(pGroup, documentName);

                QUuid uuid = QUuid::createUuid();
                QString filepath = pFile.fileName();
                if (importAdaptor->folderToCopy() != "")
                {
                    bool b = UBPersistenceManager::persistenceManager()->addFileToDocument(document, pFile.fileName(), importAdaptor->folderToCopy() , uuid, filepath);
                    if (!b)
                    {
                        UBPersistenceManager::persistenceManager()->deleteDocument(document);
                        UBApplication::setDisabled(false);
                        return NULL;
                    }
                }

                QList<UBGraphicsItem*> pages = importAdaptor->import(uuid, filepath);
                int nPage = 0;
                foreach(UBGraphicsItem* page, pages)
                {
                    UBApplication::showMessage(tr("Inserting page %1 of %2").arg(++nPage).arg(pages.size()), true);
                    int pageIndex = document->pageCount();
                    UBGraphicsScene* scene = UBPersistenceManager::persistenceManager()->createDocumentSceneAt(document, pageIndex);
                    importAdaptor->placeImportedItemToScene(scene, page);
                    UBPersistenceManager::persistenceManager()->persistDocumentScene(document, scene, pageIndex);
                }

                UBPersistenceManager::persistenceManager()->persistDocumentMetadata(document);
                UBApplication::showMessage(tr("Import successful."));
            }

            UBApplication::setDisabled(false);
            return document;
        }

    }
    return NULL;
}


int UBDocumentManager::addFilesToDocument(UBDocumentProxy* document, QStringList fileNames)
{
    int nImportedDocuments = 0;
    foreach(const QString& fileName, fileNames)
    {
        UBApplication::showMessage(tr("Importing file").arg(fileName));

        QFile file(fileName);
        QFileInfo fileInfo(file);

        foreach (UBImportAdaptor *adaptor, mImportAdaptors)
        {
            if (adaptor->supportedExtentions().lastIndexOf(fileInfo.suffix().toLower()) != -1)
            {
                UBApplication::setDisabled(true);

                if (adaptor->isDocumentBased())
                {
                    UBDocumentBasedImportAdaptor* importAdaptor = (UBDocumentBasedImportAdaptor*)adaptor;

                    if (importAdaptor->addFileToDocument(document, file))
                        nImportedDocuments++;
                }
                else
                {
                    UBPageBasedImportAdaptor* importAdaptor = (UBPageBasedImportAdaptor*)adaptor;

                    QUuid uuid = QUuid::createUuid();
                    QString filepath = file.fileName();
                    if (importAdaptor->folderToCopy() != "")
                    {
                        bool b = UBPersistenceManager::persistenceManager()->addFileToDocument(document, file.fileName(), importAdaptor->folderToCopy() , uuid, filepath);
                        if (!b)
                        {
                            continue;
                        }
                    }

                    QList<UBGraphicsItem*> pages = importAdaptor->import(uuid, filepath);
                    int nPage = 0;
                    foreach(UBGraphicsItem* page, pages)
                    {
                        UBApplication::showMessage(tr("Inserting page %1 of %2").arg(++nPage).arg(pages.size()), true);
                        int pageIndex = document->pageCount();
                        UBGraphicsScene* scene = UBPersistenceManager::persistenceManager()->createDocumentSceneAt(document, pageIndex);
                        importAdaptor->placeImportedItemToScene(scene, page);
                        UBPersistenceManager::persistenceManager()->persistDocumentScene(document, scene, pageIndex);
                    }

                    UBPersistenceManager::persistenceManager()->persistDocumentMetadata(document);
                    UBApplication::showMessage(tr("Import of file %1 successful.").arg(file.fileName()));
                    nImportedDocuments++;
                }

                UBApplication::setDisabled(false);
            }
        }
    }
    return nImportedDocuments;
}


int UBDocumentManager::addImageDirToDocument(const QDir& pDir, UBDocumentProxy* pDocument)
{
    QStringList filenames = pDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    filenames = UBStringUtils::sortByLastDigit(filenames);

    QStringList fileNames;

    foreach(QString f, filenames)
    {
        fileNames << pDir.absolutePath() + "/" + f;
    }

    return addFilesToDocument(pDocument, fileNames);

}


UBDocumentProxy* UBDocumentManager::importDir(const QDir& pDir, const QString& pGroup)
{
    UBDocumentProxy* doc = UBPersistenceManager::persistenceManager()->createDocument(pGroup, pDir.dirName());

    int result = addImageDirToDocument(pDir, doc);

    if (result > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroup);
        doc->setMetaData(UBSettings::documentName, pDir.dirName());

        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(doc);

        UBApplication::showMessage(tr("File %1 saved").arg(pDir.dirName()));

    }
    else
    {
        UBPersistenceManager::persistenceManager()->deleteDocument(doc);
    }

    return doc;
}


QList<UBExportAdaptor*> UBDocumentManager::supportedExportAdaptors()
{
    return mExportAdaptors;
}


void UBDocumentManager::emitDocumentUpdated(UBDocumentProxy* pDocument)
{
    emit documentUpdated(pDocument);
}
