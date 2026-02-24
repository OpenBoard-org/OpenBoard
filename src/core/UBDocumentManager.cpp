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


#include "UBDocumentManager.h"

#include "adaptors/UBExportFullPDF.h"
#include "adaptors/UBExportDocument.h"
#include "adaptors/UBExportWeb.h"
#include "adaptors/UBExportDocumentSetAdaptor.h"
#include "adaptors/UBImportDocument.h"
#include "adaptors/UBImportPDF.h"
#include "adaptors/UBImportImage.h"
#include "adaptors/UBImportDocumentSetAdaptor.h"

#include "board/UBBoardController.h"

#include "domain/UBGraphicsScene.h"

#include "document/UBDocument.h"
#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"
#include "document/UBDocumentToc.h"

#include "frameworks/UBStringUtils.h"

#include "gui/UBThumbnailScene.h"

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

    UBExportFullPDF* exportFullPdf = new UBExportFullPDF(this);
    UBExportDocument* exportDocument = new UBExportDocument(this);

    UBExportDocumentSetAdaptor *exportDocumentSet = new UBExportDocumentSetAdaptor(this);
    mExportAdaptors.append(exportDocument);
    mExportAdaptors.append(exportDocumentSet);
    //mExportAdaptors.append(webPublished);
    mExportAdaptors.append(exportFullPdf);

//     UBExportWeb* exportWeb = new UBExportWeb(this);
//     mExportAdaptors.append(exportWeb);

    UBImportDocument* documentImport = new UBImportDocument(this);
    mImportAdaptors.append(documentImport);
    UBImportDocumentSetAdaptor *documentSetImport = new UBImportDocumentSetAdaptor(this);
    mImportAdaptors.append(documentSetImport);
    UBImportPDF* pdfImport = new UBImportPDF(this);
    mImportAdaptors.append(pdfImport);
    UBImportImage* imageImport = new UBImportImage(this);
    mImportAdaptors.append(imageImport);
}


UBDocumentManager::~UBDocumentManager()
{
    // NOOP
}


QStringList UBDocumentManager::importFileExtensions(bool notUbx)
{
    QStringList result;

    foreach (UBImportAdaptor *importAdaptor, mImportAdaptors)
    {
        //issue 1629 - NNE - 20131213 : add test to remove ubx extention if necessary
        if(!(notUbx && importAdaptor->supportedExtentions().at(0) == "ubx")){
            result << importAdaptor->supportedExtentions();
        }
    }
    return result;
}


QString UBDocumentManager::importFileFilter(bool notUbx)
{
    QString result;

    result += tr("All supported files (*.%1)").arg(importFileExtensions(notUbx).join(" *."));
    foreach (UBImportAdaptor *importAdaptor, mImportAdaptors)
    {
        if (importAdaptor->importFileFilter().length() > 0)
        {
            //issue 1629 - NNE - 20131213 : Add a test on ubx before put in the list
            if(!(notUbx && importAdaptor->supportedExtentions().at(0) == "ubx")){
                if (result.length())
                {
                    result += ";;";
                }

                result += importAdaptor->importFileFilter();
            }
        }
    }
    qDebug() << "import file filter" << result;
    return result;
}

QFileInfoList UBDocumentManager::importUbx(const QString &Incomingfile, const QString &destination)
{
    UBImportDocumentSetAdaptor *docSetAdaptor = nullptr;
    foreach (UBImportAdaptor *curAdaptor, mImportAdaptors) {
        docSetAdaptor = qobject_cast<UBImportDocumentSetAdaptor*>(curAdaptor);
        if (docSetAdaptor) {
            break;
        }
    }
    if (!docSetAdaptor) {
        return QFileInfoList();
    }

    return docSetAdaptor->importData(Incomingfile, destination);
}

std::shared_ptr<UBDocument> UBDocumentManager::importFile(const QFile& pFile, const QString& pGroup)
{
    QFileInfo fileInfo(pFile);

    foreach (UBImportAdaptor *adaptor, mImportAdaptors)
    {
        if (adaptor->supportedExtentions().lastIndexOf(fileInfo.suffix().toLower()) != -1)
        {
            std::shared_ptr<UBDocumentProxy> document;
            std::shared_ptr<UBDocument> doc;
            UBApplication::setDisabled(true);

            if (adaptor->isDocumentBased())
            {
                UBDocumentBasedImportAdaptor* importAdaptor = (UBDocumentBasedImportAdaptor*)adaptor;

                document = importAdaptor->importFile(pFile, pGroup);
                doc = UBDocument::getDocument(document);
            }
            else
            {
                UBPageBasedImportAdaptor* importAdaptor = (UBPageBasedImportAdaptor*)adaptor;

                // Document import procedure.....
                QString documentName = QFileInfo(pFile.fileName()).completeBaseName();
                document = UBPersistenceManager::persistenceManager()->createDocument(pGroup
                                                                                      ,documentName
                                                                                      , false // Issue 1630 - CFA - 201410503 - suppression de la page vide ajoutee à l'import des pdfs
                                                                                      , QString()
                                                                                      , 0
                                                                                      , true);

                if (document)
                {
                    doc = UBDocument::getDocument(document);

                    // create the thumbnail scene before any pages are created
                    doc->thumbnailScene(false);

                    importPages(pFile, doc, importAdaptor);

                    UBApplication::showMessage(tr("Import successful."));
                }
            }

            UBApplication::setDisabled(false);
            return doc;
        }

    }
    return NULL;
}


int UBDocumentManager::addFilesToDocument(std::shared_ptr<UBDocumentProxy> document, QStringList fileNames)
{
    int nImportedDocuments = 0;
    auto doc = UBDocument::getDocument(document);

    foreach(const QString& fileName, fileNames)
    {
        UBApplication::showMessage(tr("Importing file %1").arg(fileName));

        QFile file(fileName);
        QFileInfo fileInfo(file);

        UBApplication::setDisabled(true);

        foreach (UBImportAdaptor *adaptor, mImportAdaptors)
        {
            if (adaptor->supportedExtentions().lastIndexOf(fileInfo.suffix().toLower()) != -1)
            {
                if (adaptor->isDocumentBased())
                {
                    //issue 1629 - NNE - 20131212 : Resolve a segfault, but for .ubx, actually
                    //the file will be not imported...
                    UBDocumentBasedImportAdaptor* importAdaptor = dynamic_cast<UBDocumentBasedImportAdaptor*>(adaptor);

                    if (importAdaptor && importAdaptor->addFileToDocument(document, file))
                    {
                        nImportedDocuments++;
                        break;
                    }
                }
                else
                {
                    UBPageBasedImportAdaptor* importAdaptor = (UBPageBasedImportAdaptor*)adaptor;
                    importPages(file, doc, importAdaptor);

                    UBApplication::showMessage(tr("Import of file %1 successful.").arg(file.fileName()));
                    nImportedDocuments++;
                    break;
                }
            }
        }

        UBApplication::setDisabled(false);
    }

    return nImportedDocuments;
}


int UBDocumentManager::addImageDirToDocument(const QDir& pDir, std::shared_ptr<UBDocumentProxy> pDocument)
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


std::shared_ptr<UBDocumentProxy> UBDocumentManager::importDir(const QDir& pDir, const QString& pGroup)
{
    std::shared_ptr<UBDocumentProxy> doc = UBPersistenceManager::persistenceManager()->createDocument(pGroup, pDir.dirName());

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

int UBDocumentManager::importPages(const QFile& file, std::shared_ptr<UBDocument> doc, UBPageBasedImportAdaptor* importAdaptor)
{
    std::shared_ptr<UBDocumentProxy> document = doc->proxy();
    QString filepath = file.fileName();
    QUuid uuid;

    if (importAdaptor->folderToCopy() != "")
    {
        uuid = UBPersistenceManager::persistenceManager()->addFileToDocument(document, file.fileName(), importAdaptor->folderToCopy(), filepath);

        if (uuid.isNull())
        {
            return 0;
        }
    }

    // NOTE when folderToCopy returns empty string, uuid parameter is not used for import
    QList<UBGraphicsItem*> pages = importAdaptor->import(uuid, filepath);
    const int startIndex = doc->pageCount();
    int pageIndex = startIndex;

    UBApplication::showMessage(tr("Creating %1 pages. Please wait...").arg(pages.size()), true);

    for (UBGraphicsItem* page : pages)
    {
        std::shared_ptr<UBGraphicsScene> scene = doc->createPage(pageIndex, false, false);
        importAdaptor->placeImportedItemToScene(scene, page);
        doc->persistPage(scene, pageIndex, false, false, false, false);
        pageIndex++;
    }

    UBPersistenceManager::persistenceManager()->persistDocumentMetadata(document);

    // remove all asset entries to force scanning
    for (int index = startIndex; index < pageIndex; ++index)
    {
        doc->toc()->unsetAssets(index);
    }

    doc->toc()->save();
    doc->scanAssets();

    return pageIndex - startIndex;
}
