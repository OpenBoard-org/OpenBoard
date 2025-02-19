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



#include "UBPersistenceManager.h"
#include "gui/UBMainWindow.h"

#include <QtXml>
#include <QVariant>
#include <QDomDocument>
#include <QXmlStreamWriter>
#include <QModelIndex>
#include <QtConcurrent>

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "document/UBDocumentProxy.h"

#include "adaptors/UBExportPDF.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsSvgItem.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "document/UBDocumentController.h"

#include "core/memcheck.h"

const QString UBPersistenceManager::imageDirectory = "images"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::objectDirectory = "objects"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::widgetDirectory = "widgets"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::videoDirectory = "videos"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::audioDirectory = "audios"; // added to
const QString UBPersistenceManager::fileDirectory = "files"; // Issue 1683 (Evolution) - AOU - 20131206

const QString UBPersistenceManager::myDocumentsName = "MyDocuments";
const QString UBPersistenceManager::modelsName = "Models";
const QString UBPersistenceManager::untitledDocumentsName = "UntitledDocuments";
const QString UBPersistenceManager::fFolders = "folders.xml";
const QString UBPersistenceManager::tFolder = "folder";
const QString UBPersistenceManager::aName = "name";


UBPersistenceManager * UBPersistenceManager::sSingleton = 0;

UBPersistenceManager::UBPersistenceManager(QObject *pParent)
    : QObject(pParent)
    , mHasPurgedDocuments(false)
    , mIsWorkerFinished(false)
    , mReplaceDialogReturnedReplaceAll(false)
    , mReplaceDialogReturnedCancel(false)
{

    xmlFolderStructureFilename = "model";

    mDocumentSubDirectories << imageDirectory;
    mDocumentSubDirectories << objectDirectory;
    mDocumentSubDirectories << widgetDirectory;
    mDocumentSubDirectories << videoDirectory;
    mDocumentSubDirectories << audioDirectory;
    mDocumentSubDirectories << fileDirectory; // Issue 1683 (Evolution) - AOU - 20131206

    mDocumentRepositoryPath = UBSettings::userDocumentDirectory();
    mFoldersXmlStorageName =  mDocumentRepositoryPath + "/" + fFolders;

    mDocumentTreeStructureModel = new UBDocumentTreeModel(this);
    createDocumentProxiesStructure();

    mThread = new QThread;
    mWorker = new UBPersistenceWorker();
    mWorker->moveToThread(mThread);

    connect(mWorker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(mThread, SIGNAL(started()), mWorker, SLOT(process()));
    connect(mWorker, SIGNAL(finished()), mThread, SLOT(quit()));
    connect(mWorker, SIGNAL(finished()), this, SLOT(onWorkerFinished()));
    connect(mWorker, SIGNAL(finished()), mWorker, SLOT(deleteLater()));
    connect(mThread, SIGNAL(finished()), mThread, SLOT(deleteLater()));
    connect(mWorker, &UBPersistenceWorker::scenePersisted, this, &UBPersistenceManager::onScenePersisted);

    mThread->start();
}

UBPersistenceManager* UBPersistenceManager::persistenceManager()
{
    if (!sSingleton)
    {
        sSingleton = new UBPersistenceManager(UBApplication::staticMemoryCleaner);
    }

    return sSingleton;
}

void UBPersistenceManager::destroy()
{
    if (sSingleton)
        delete sSingleton;
    sSingleton = NULL;
}

void UBPersistenceManager::onWorkerFinished()
{
    mIsWorkerFinished = true;
}

void UBPersistenceManager::onScenePersisted(UBGraphicsScene *scene)
{
    // delete the copy
    mScenesToSave.removeAll(scene->shared_from_this());
}

UBPersistenceManager::~UBPersistenceManager()
{
    mIsApplicationClosing = true;

    if(mWorker)
        mWorker->applicationWillClose();

    QElapsedTimer t;
    t.start();
    qDebug() << "start waiting";

    while(!mIsWorkerFinished)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    qDebug() << "stop waiting after " << t.elapsed() << " ms";

    // to be sure that all the scenes are stored on disk
}

void UBPersistenceManager::errorString(QString error)
{
    qWarning() << "An error occured in the peristence thread " << error;
}

void UBPersistenceManager::createDocumentProxiesStructure(const QFileInfoList &contentInfoList, bool interactive)
{
    // Create a QFutureWatcher and connect signals and slots.
    QFutureWatcher<std::shared_ptr<UBDocumentProxy>> futureWatcher;
    QObject::connect(&futureWatcher, &QFutureWatcher<void>::finished, &mProgress, &QProgressDialog::reset);
    QObject::connect(&futureWatcher,  &QFutureWatcher<void>::progressRangeChanged, &mProgress, &QProgressDialog::setRange);
    QObject::connect(&futureWatcher, &QFutureWatcher<void>::progressValueChanged,  &mProgress, &QProgressDialog::setValue);

    // Start the computation.
    std::function<std::shared_ptr<UBDocumentProxy> (QFileInfo contentInfo)> createDocumentProxyLambda = [=](QFileInfo contentInfo) {
        return createDocumentProxyStructure(contentInfo);
    };

    QFuture<std::shared_ptr<UBDocumentProxy>> proxiesFuture = QtConcurrent::mapped(contentInfoList, createDocumentProxyLambda);
    futureWatcher.setFuture(proxiesFuture);

    // Display the dialog and start the event loop.
    mProgress.exec();

    futureWatcher.waitForFinished();

    QList<std::shared_ptr<UBDocumentProxy>> proxies = futureWatcher.future().results();

    for (auto&& proxy : std::as_const(proxies))
    {
        if (proxy)
        {
            QString docGroupName = proxy->metaData(UBSettings::documentGroupName).toString();
            QModelIndex parentIndex = mDocumentTreeStructureModel->goTo(docGroupName);
            if (parentIndex.isValid())
            {
                if (!interactive)
                   mDocumentTreeStructureModel->addDocument(proxy, parentIndex);
                else
                {
                   processInteractiveReplacementDialog(proxy, true);
                }
            }
            else
            {
                qDebug() << "something went wrong";
            }
        }
    }

    mReplaceDialogReturnedReplaceAll = false;
    mReplaceDialogReturnedCancel = false;
}

void UBPersistenceManager::createDocumentProxiesStructure(bool interactive)
{
    mDocumentRepositoryPath = UBSettings::userDocumentDirectory();

    QDir rootDir(mDocumentRepositoryPath);
    rootDir.mkpath(rootDir.path());

    QFileInfoList contentInfoList = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);

    mProgress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    mProgress.setLabelText(tr("Retrieving all your documents (found : %1)").arg(contentInfoList.size()));
    mProgress.setCancelButton(nullptr);

    createDocumentProxiesStructure(contentInfoList, interactive);

    if (QFileInfo(mFoldersXmlStorageName).exists()) {
        QDomDocument xmlDom;
        QFile inFile(mFoldersXmlStorageName);
        if (inFile.open(QIODevice::ReadOnly)) {
            QString domString(inFile.readAll());

            int errorLine = 0; int errorColumn = 0;
            QString errorStr;

            if (xmlDom.setContent(domString, &errorStr, &errorLine, &errorColumn)) {
                loadFolderTreeFromXml("", xmlDom.firstChildElement());
            } else {
                qDebug() << "Error reading content of " << mFoldersXmlStorageName << '\n'
                         << "Error:" << inFile.errorString()
                         << "Line:" << errorLine
                         << "Column:" << errorColumn;
            }
            inFile.close();
        } else {
            qDebug() << "Error reading" << mFoldersXmlStorageName << '\n'
                     << "Error:" << inFile.errorString();
        }
    }
}

std::shared_ptr<UBDocumentProxy> UBPersistenceManager::createDocumentProxyStructure(QFileInfo& contentInfo)
{
    QString fullPath = contentInfo.absoluteFilePath();
    QDir dir(fullPath);

    if (dir.entryList(QDir::Files | QDir::NoDotAndDotDot).size() > 0)
    {
        QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(fullPath);
        QString docGroupName = metadatas.value(UBSettings::documentGroupName, QString()).toString();
        QString docName = metadatas.value(UBSettings::documentName, QString()).toString();

        if (docName.isEmpty()) {
            qDebug() << "Group name and document name are empty in UBPersistenceManager::createDocumentProxiesStructure()";
            return nullptr;
        }

        std::shared_ptr<UBDocumentProxy> docProxy = std::make_shared<UBDocumentProxy>(fullPath);
        foreach(QString key, metadatas.keys())
        {
            docProxy->setMetaData(key, metadatas.value(key));
        }

        docProxy->setPageCount(sceneCount(docProxy));

        return docProxy;
    }

    return nullptr;
};

QDialog::DialogCode UBPersistenceManager::processInteractiveReplacementDialog(std::shared_ptr<UBDocumentProxy> pProxy, bool multipleFiles)
{
    QDialog::DialogCode result = QDialog::Rejected;

    if (UBApplication::documentController && UBApplication::documentController->mainWidget())
    {
        QString docGroupName = pProxy->metaData(UBSettings::documentGroupName).toString();
        QModelIndex parentIndex = mDocumentTreeStructureModel->goTo(docGroupName);
        if (!parentIndex.isValid())
        {
            return QDialog::Rejected;
        }

        QStringList docList = mDocumentTreeStructureModel->nodeNameList(parentIndex, true);
        QString docName = pProxy->metaData(UBSettings::documentName).toString();

        if (docList.contains(docName))
        {
            if (!mReplaceDialogReturnedReplaceAll && !mReplaceDialogReturnedCancel)
            {
                UBDocumentReplaceDialog *replaceDialog = new UBDocumentReplaceDialog(docName
                                                                                     , docList
                                                                                     , multipleFiles
                                                                                     , /*UBApplication::documentController->mainWidget()*/0
                                                                                     , Qt::Widget);
                if (replaceDialog->exec() == QDialog::Accepted)
                {
                    UBApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                    result = QDialog::Accepted;
                    QString resultName = replaceDialog->lineEditText();
                    int i = docList.indexOf(resultName);
                    if (i != -1)
                    { //replace
                        QModelIndex replacedIndex = mDocumentTreeStructureModel->index(i, 0, parentIndex);
                        std::shared_ptr<UBDocumentProxy> replacedProxy = mDocumentTreeStructureModel->proxyData(replacedIndex);

                        if (replacedProxy)
                        {
                            deleteDocument(replacedProxy);
                        }

                        if (replacedIndex.isValid()) {
                            mDocumentTreeStructureModel->removeRow(i, parentIndex);
                        }

                        if (docName != resultName)
                        {
                            pProxy->setMetaData(UBSettings::documentName, resultName);
                            UBMetadataDcSubsetAdaptor::persist(pProxy);
                        }

                        // create new index before trying to select it (if avtive index)
                        mDocumentTreeStructureModel->addDocument(pProxy, parentIndex);

                        if (UBApplication::documentController->selectedDocument() == replacedProxy)
                        {
                            UBApplication::documentController->pureSetDocument(pProxy);
                        }

                        if (UBApplication::boardController->selectedDocument() == replacedProxy)
                        {
                            UBApplication::boardController->pureSetDocument(pProxy);
                        }
                    }
                    else
                    {
                        if (docName != resultName)
                        {
                            pProxy->setMetaData(UBSettings::documentName, resultName);
                            UBMetadataDcSubsetAdaptor::persist(pProxy);
                        }

                        mDocumentTreeStructureModel->addDocument(pProxy, parentIndex);
                    }

                    if (replaceDialog->replaceAllClicked())
                    {
                        mReplaceDialogReturnedReplaceAll = true;
                    }
                    UBApplication::restoreOverrideCursor();
                }
                else
                {
                    if (replaceDialog->cancelClicked())
                    {
                        mReplaceDialogReturnedCancel = true;
                    }
                }
                replaceDialog->setParent(0);
                delete replaceDialog;
            }
            else
            {
                if (mReplaceDialogReturnedReplaceAll)
                {
                    result = QDialog::Accepted;
                    int i = docList.indexOf(docName);
                    if (i > -1)
                    {
                        QModelIndex replacedIndex = mDocumentTreeStructureModel->index(i, 0, parentIndex);
                        std::shared_ptr<UBDocumentProxy> replacedProxy = mDocumentTreeStructureModel->proxyData(replacedIndex);

                        if (replacedProxy)
                        {
                            deleteDocument(replacedProxy);
                        }

                        if (replacedIndex.isValid())
                        {
                            mDocumentTreeStructureModel->removeRow(i, parentIndex);
                        }

                        // create new index before trying to select it (if avtive index)
                        mDocumentTreeStructureModel->addDocument(pProxy, parentIndex);

                        if (UBApplication::documentController->selectedDocument() == replacedProxy)
                        {
                            UBApplication::documentController->pureSetDocument(pProxy);
                        }

                        if (UBApplication::boardController->selectedDocument() == replacedProxy)
                        {
                            UBApplication::boardController->pureSetDocument(pProxy);
                        }
                    }
                    else
                    {
                        mDocumentTreeStructureModel->addDocument(pProxy, parentIndex);
                    }

                }
            }
        } else {
            mDocumentTreeStructureModel->addDocument(pProxy, parentIndex);
            result = QDialog::Accepted;
        }
    }

    return result;
}

QString UBPersistenceManager::adjustDocumentVirtualPath(const QString &str)
{
    QStringList pathList = str.split("/", UB::SplitBehavior::SkipEmptyParts);

    if (pathList.isEmpty()) {
        pathList.append(myDocumentsName);
        pathList.append(untitledDocumentsName);
    }

    if (pathList.first() != myDocumentsName
            && pathList.first() != UBSettings::trashedDocumentGroupNamePrefix
            && pathList.first() != modelsName) {
        pathList.prepend(myDocumentsName);
    }

    return pathList.join("/");
}

void UBPersistenceManager::closing()
{
    QDir rootDir(mDocumentRepositoryPath);
    rootDir.mkpath(rootDir.path());

    QFile outFile(mFoldersXmlStorageName);
    if (outFile.open(QIODevice::WriteOnly)) {
        QXmlStreamWriter writer(&outFile);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("content");
        saveFoldersTreeToXml(writer, QModelIndex());
        writer.writeEndElement();
        writer.writeEndDocument();

        outFile.close();
    } else {
        qDebug() << "failed to open document" <<  mFoldersXmlStorageName << "for writing" << '\n'
                 << "Error string:" << outFile.errorString();
    }
}

bool UBPersistenceManager::isSceneInCached(std::shared_ptr<UBDocumentProxy> proxy, int index) const
{
    return mSceneCache.contains(proxy, index);
}

QStringList UBPersistenceManager::allShapes()
{
    QString shapeLibraryPath = UBSettings::settings()->applicationShapeLibraryDirectory();

    QDir dir(shapeLibraryPath);

    if (!dir.exists())
        dir.mkpath(shapeLibraryPath);

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(shapeLibraryPath + QString("/") + file);
    }

    return paths;
}

QStringList UBPersistenceManager::allGips()
{
    QString gipLibraryPath = UBSettings::settings()->userGipLibraryDirectory();

    QDir dir(gipLibraryPath);

    QStringList files = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        QFileInfo fi(file);

        if (UBSettings::settings()->widgetFileExtensions.contains(fi.suffix()))
            paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}

QStringList UBPersistenceManager::allImages(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


QStringList UBPersistenceManager::allVideos(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


QStringList UBPersistenceManager::allWidgets(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        QFileInfo fi(file);

        if (UBSettings::settings()->widgetFileExtensions.contains(fi.suffix()))
            paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


std::shared_ptr<UBDocumentProxy> UBPersistenceManager::createDocument(const QString& pGroupName
                                                      , const QString& pName
                                                      , bool withEmptyPage
                                                      , QString directory
                                                      , int pageCount
                                                      , bool promptDialogIfExists)
{
    std::shared_ptr<UBDocumentProxy> doc;
    if(directory.length() != 0 )
    {
        doc = std::make_shared<UBDocumentProxy>(directory);
        doc->setPageCount(pageCount);
    }
    else{
        checkIfDocumentRepositoryExists();
        doc = std::make_shared<UBDocumentProxy>();
    }

    if (pGroupName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroupName);
    }

    if (pName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentName, pName);
    }

    doc->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);
    QString currentDate =  UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime());
    doc->setMetaData(UBSettings::documentUpdatedAt,currentDate);
    doc->setMetaData(UBSettings::documentDate,currentDate);

    if (withEmptyPage)
    {
        createDocumentSceneAt(doc, 0);
    }
    else
    {
        generatePathIfNeeded(doc);
        QDir dir(doc->persistencePath());
        if (!dir.mkpath(doc->persistencePath()))
        {
            return nullptr; // if we can't create the path, abort function.
        }
    }

    bool documentAdded = false;
    if (!promptDialogIfExists)
    {
        documentAdded = true;
        mDocumentTreeStructureModel->addDocument(doc);
    }
    else if (processInteractiveReplacementDialog(doc) == QDialog::Accepted)
    {
        documentAdded = true;
    }

    if (documentAdded)
    {
        persistDocumentMetadata(doc);
    }
    else
    {
        deleteDocument(doc);
        doc = nullptr;
    }

    return doc;
}

std::shared_ptr<UBDocumentProxy> UBPersistenceManager::createNewDocument(const QString& pGroupName
                                                      , const QString& pName
                                                      , bool withEmptyPage
                                                      , QString directory
                                                      , int pageCount
                                                      , bool promptDialogIfExists)
{
    std::shared_ptr<UBDocumentProxy> resultDoc = createDocument(pGroupName, pName, withEmptyPage, directory, pageCount, promptDialogIfExists);

    return resultDoc;
}

std::shared_ptr<UBDocumentProxy> UBPersistenceManager::createDocumentFromDir(const QString& pDocumentDirectory
                                                             , const QString& pGroupName
                                                             , const QString& pName
                                                             , bool withEmptyPage
                                                             , bool addTitlePage
                                                             , bool promptDialogIfExists)
{
    checkIfDocumentRepositoryExists();

    std::shared_ptr<UBDocumentProxy> doc = std::make_shared<UBDocumentProxy>(pDocumentDirectory); // deleted in UBPersistenceManager::destructor

    QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(pDocumentDirectory);

    if(withEmptyPage)
    {
        createDocumentSceneAt(doc, 0);
    }

    if(addTitlePage)
    {
        persistDocumentScene(doc, mSceneCache.createScene(doc, 0, false), 0);
    }

    foreach(QString key, metadatas.keys())
    {
        doc->setMetaData(key, metadatas.value(key));
    }

    if (pGroupName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroupName);
    }

    if (pName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentName, pName);
    }

    doc->setUuid(QUuid::createUuid());
    doc->setPageCount(sceneCount(doc));

    for(int i = 0; i < doc->pageCount(); i++)
    {
        UBSvgSubsetAdaptor::setSceneUuid(doc, i, QUuid::createUuid());
    }

    //work around the
    bool addDoc = false;
    if (!promptDialogIfExists) {
        addDoc = true;
        mDocumentTreeStructureModel->addDocument(doc);
    } else if (processInteractiveReplacementDialog(doc) == QDialog::Accepted) {
        addDoc = true;
    }
    if (addDoc) {
        persistDocumentMetadata(doc);
        emit documentCreated(doc);
    } else {
        deleteDocument(doc);
        doc = 0;
    }

    return doc;
}


void UBPersistenceManager::deleteDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    qWarning() << "deleting dir with path: " << pDocumentProxy->persistencePath();
    checkIfDocumentRepositoryExists();

    if (QFileInfo(pDocumentProxy->persistencePath()).exists())
        UBFileSystemUtils::deleteDir(pDocumentProxy->persistencePath());

    mSceneCache.removeAllScenes(pDocumentProxy);
}

std::shared_ptr<UBDocumentProxy> UBPersistenceManager::duplicateDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    checkIfDocumentRepositoryExists();

    std::shared_ptr<UBDocumentProxy> copy = std::make_shared<UBDocumentProxy>();

    generatePathIfNeeded(copy);

    UBFileSystemUtils::copyDir(pDocumentProxy->persistencePath(), copy->persistencePath());

    // regenerate scenes UUIDs
    for(int i = 0; i < pDocumentProxy->pageCount(); i++)
    {
        UBSvgSubsetAdaptor::setSceneUuid(pDocumentProxy, i, QUuid::createUuid());
    }

    foreach(QString key, pDocumentProxy->metaDatas().keys())
    {
        copy->setMetaData(key, pDocumentProxy->metaDatas().value(key));
    }    

    copy->setMetaData(UBSettings::documentName,
            pDocumentProxy->metaData(UBSettings::documentName).toString() + " " + tr("(copy)"));

    copy->setUuid(QUuid::createUuid());

    persistDocumentMetadata(copy);

    copy->setPageCount(copy->pageCount());

    emit documentCreated(copy);

    return copy;

}


void UBPersistenceManager::deleteDocumentScenes(std::shared_ptr<UBDocumentProxy> proxy, const QList<int>& indexes)
{
    checkIfDocumentRepositoryExists();

    int pageCount = proxy->pageCount();

    QList<int> compactedIndexes;

    foreach(int index, indexes)
    {
        if (!compactedIndexes.contains(index))
            compactedIndexes.append(index);
    }

    if (compactedIndexes.size() == pageCount)
    {
        deleteDocument(proxy);
        return;
    }

    if (compactedIndexes.size() == 0)
        return;

    QString sourceName = proxy->metaData(UBSettings::documentName).toString();
    std::shared_ptr<UBDocumentProxy> trashDocProxy = createDocument(UBSettings::trashedDocumentGroupNamePrefix/* + sourceGroupName*/, sourceName, false);

    foreach(int index, compactedIndexes)
    {
        std::shared_ptr<UBGraphicsScene> scene = loadDocumentScene(proxy, index);
        if (scene)
        {
            //scene is about to move into new document
            foreach (QUrl relativeFile, scene->relativeDependencies())
            {
                QString source = scene->document()->persistencePath() + "/" + relativeFile.toString();
                QString target = trashDocProxy->persistencePath() + "/" + relativeFile.toString();

                QFileInfo fi(target);
                QDir d = fi.dir();

                d.mkpath(d.absolutePath());
                QFile::copy(source, target);
            }
            insertDocumentSceneAt(trashDocProxy, scene, trashDocProxy->pageCount(), true, true);
        }
    }

    for (int i = 1; i < indexes.size(); i++)
    {
        renamePage(trashDocProxy, i , i - 1);
    }

    foreach(int index, compactedIndexes)
    {
        QString svgFileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", index);

        QFile::remove(svgFileName);

        QString thumbFileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", index);

        QFile::remove(thumbFileName);

        mSceneCache.removeScene(proxy, index);

        proxy->decPageCount();

    }

    std::sort(compactedIndexes.begin(), compactedIndexes.end());

    int offset = 1;

    for (int i = compactedIndexes.at(0) + 1; i < pageCount; i++)
    {
        if(compactedIndexes.contains(i))
        {
            offset++;
        }
        else
        {
            renamePage(proxy, i , i - offset);

            mSceneCache.moveScene(proxy, i, i - offset);

        }
    }
}


void UBPersistenceManager::duplicateDocumentScene(std::shared_ptr<UBDocumentProxy> proxy, int index)
{
    checkIfDocumentRepositoryExists();

    auto scene = mSceneCache.value(proxy, index);

    if (scene && scene->isModified())
    {
        persistDocumentScene(proxy, scene, index, false, true);
    }

    for (int i = proxy->pageCount(); i > index + 1; i--)
    {
        renamePage(proxy, i - 1 , i);

        mSceneCache.moveScene(proxy, i - 1, i);

    }

    copyPage(proxy, index , index + 1);

    //TODO: write a proper way to handle object on disk
    scene = loadDocumentScene(proxy, index + 1);

    foreach(QGraphicsItem* item, scene->items())
    {
        UBGraphicsMediaItem *mediaItem = qgraphicsitem_cast<UBGraphicsMediaItem*> (item);

        if (mediaItem){
            QString source = mediaItem->mediaFileUrl().toLocalFile();
            QString destination = source;
            QUuid newUuid = QUuid::createUuid();
            QString fileName = QFileInfo(source).completeBaseName();
            destination = destination.replace(fileName,newUuid.toString());
            QFile::copy(source,destination);
            mediaItem->setMediaFileUrl(QUrl::fromLocalFile(destination));
            continue;
        }

        UBGraphicsWidgetItem* widget = qgraphicsitem_cast<UBGraphicsWidgetItem*>(item);
        if(widget){
            QUuid newUUid = QUuid::createUuid();
            QString newUUidString = newUUid.toString().remove("{").remove("}");
            QString actualUuidString = widget->uuid().toString().remove("{").remove("}");

            QString widgetSourcePath = proxy->persistencePath() + "/" + UBPersistenceManager::widgetDirectory + "/{" + actualUuidString + "}.wgt";
            QString screenshotSourcePath = proxy->persistencePath() + "/" +  UBPersistenceManager::widgetDirectory + "/" + actualUuidString + ".png";

            QString widgetDestinationPath = widgetSourcePath;
            widgetDestinationPath = widgetDestinationPath.replace(actualUuidString,newUUidString);
            QString screenshotDestinationPath = screenshotSourcePath;
            screenshotDestinationPath = screenshotDestinationPath.replace(actualUuidString,newUUidString);

            UBFileSystemUtils::copyDir(widgetSourcePath,widgetDestinationPath);
            QFile::copy(screenshotSourcePath,screenshotDestinationPath);

            widget->setUuid(newUUid);

            widget->widgetUrl(QUrl::fromLocalFile(widgetDestinationPath));

            continue;
        }

        UBGraphicsPixmapItem* pixmapItem = qgraphicsitem_cast<UBGraphicsPixmapItem*>(item);
        if(pixmapItem){
            QDir imageDir = proxy->persistencePath() + "/" + UBPersistenceManager::imageDirectory;
            QStringList imageFiles = imageDir.entryList({pixmapItem->uuid().toString() + ".*"});

            if (!imageFiles.isEmpty())
            {
                QString source = proxy->persistencePath() + "/" +  UBPersistenceManager::imageDirectory + "/" + imageFiles.last();
                QString destination = source;
                QUuid newUuid = QUuid::createUuid();
                QString fileName = QFileInfo(source).completeBaseName();
                destination = destination.replace(fileName,newUuid.toString());
                QFile::copy(source,destination);
                pixmapItem->setUuid(newUuid);
            }

            continue;
        }

        UBGraphicsSvgItem* svgItem = qgraphicsitem_cast<UBGraphicsSvgItem*>(item);
        if(svgItem){
            QString source = proxy->persistencePath() + "/" +  UBPersistenceManager::imageDirectory + "/" + svgItem->uuid().toString() + ".svg";
            QString destination = source;
            QUuid newUuid = QUuid::createUuid();
            QString fileName = QFileInfo(source).completeBaseName();
            destination = destination.replace(fileName,newUuid.toString());
            QFile::copy(source,destination);
            svgItem->setUuid(newUuid);
            continue;
        }

    }
    scene->setModified(true);

    proxy->incPageCount();

    persistDocumentScene(proxy,scene, index + 1);

    persistDocumentMetadata(proxy);

    emit documentSceneCreated(proxy, index + 1);
}


std::shared_ptr<UBGraphicsScene> UBPersistenceManager::createDocumentSceneAt(std::shared_ptr<UBDocumentProxy> proxy, int index, bool useUndoRedoStack)
{
    int count = proxy->pageCount();

    for(int i = count - 1; i >= index; i--)
    {
        renamePage(proxy, i , i + 1);
    }

    mSceneCache.shiftUpScenes(proxy, index, count -1);

    std::shared_ptr<UBGraphicsScene> newScene = mSceneCache.createScene(proxy, index, useUndoRedoStack);

    newScene->setBackground(UBSettings::settings()->isDarkBackground(),
            UBSettings::settings()->UBSettings::pageBackground());

    newScene->setBackgroundGridSize(UBSettings::settings()->crossSize);

    proxy->incPageCount();

    persistDocumentScene(proxy, newScene, index);

    emit documentSceneCreated(proxy, index);

    return newScene;
}


void UBPersistenceManager::insertDocumentSceneAt(std::shared_ptr<UBDocumentProxy> proxy, std::shared_ptr<UBGraphicsScene> scene, int index, bool persist, bool deleting)
{
    scene->setDocument(proxy);

    int count = proxy->pageCount();

    for(int i = count - 1; i >= index; i--)
    {
        renamePage(proxy, i , i + 1);
    }

    mSceneCache.shiftUpScenes(proxy, index, count -1);

    mSceneCache.insert(proxy, index, scene);

    proxy->incPageCount();

    if (persist) {
        persistDocumentScene(proxy, scene, index);
    }

    if (!deleting)
        emit documentSceneCreated(proxy, index);

}


void UBPersistenceManager::moveSceneToIndex(std::shared_ptr<UBDocumentProxy> proxy, int source, int target)
{
    checkIfDocumentRepositoryExists();

    if (source == target)
        return;

    QFile svgTmp(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", source));
    svgTmp.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.tmp", target));

    QFile thumbTmp(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", source));
    thumbTmp.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.tmp", target));

    if (source < target)
    {
        for (int i = source + 1; i <= target; i++)
        {
            renamePage(proxy, i , i - 1);
        }
    }
    else
    {
        for (int i = source - 1; i >= target; i--)
        {
            renamePage(proxy, i , i + 1);
        }
    }

    QFile svg(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.tmp", target));
    svg.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", target));

    QFile thumb(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.tmp", target));
    thumb.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", target));

    mSceneCache.moveScene(proxy, source, target);
}


std::shared_ptr<UBGraphicsScene> UBPersistenceManager::loadDocumentScene(std::shared_ptr<UBDocumentProxy> proxy, int sceneIndex, bool cacheNeighboringScenes)
{
    mSceneCache.prepareLoading(proxy, sceneIndex);
    auto scene = mSceneCache.value(proxy, sceneIndex);
    qDebug() << "loadDocumentScene: got result from cache";

    if (cacheNeighboringScenes)
    {
        if(sceneIndex + 1 < proxy->pageCount() &&  !mSceneCache.contains(proxy, sceneIndex + 1))
            mSceneCache.prepareLoading(proxy,sceneIndex+1);

        if(sceneIndex + 2 < proxy->pageCount() &&  !mSceneCache.contains(proxy, sceneIndex + 2))
            mSceneCache.prepareLoading(proxy,sceneIndex+2);

        if(sceneIndex - 1 >= 0 &&  !mSceneCache.contains(proxy, sceneIndex - 1))
            mSceneCache.prepareLoading(proxy,sceneIndex-1);
    }

    return scene;
}

std::shared_ptr<UBGraphicsScene> UBPersistenceManager::getDocumentScene(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int sceneIndex)
{
    return mSceneCache.value(pDocumentProxy, sceneIndex);
}

void UBPersistenceManager::reassignDocProxy(std::shared_ptr<UBDocumentProxy> newDocument, std::shared_ptr<UBDocumentProxy> oldDocument)
{
    return mSceneCache.reassignDocProxy(newDocument, oldDocument);
}

void UBPersistenceManager::persistDocumentScene(std::shared_ptr<UBDocumentProxy> pDocumentProxy, std::shared_ptr<UBGraphicsScene> pScene, const int pSceneIndex, bool isAnAutomaticBackup, bool forceImmediateSaving)
{
    checkIfDocumentRepositoryExists();

    if (!isAnAutomaticBackup)
        pScene->deselectAllItems();

    generatePathIfNeeded(pDocumentProxy);

    QDir dir(pDocumentProxy->persistencePath());
    dir.mkpath(pDocumentProxy->persistencePath());

    if(forceImmediateSaving)
    {
        UBSvgSubsetAdaptor::persistScene(pDocumentProxy, pScene, pSceneIndex);
    }
    else
    {
       std::shared_ptr<UBGraphicsScene> copiedScene = pScene->sceneDeepCopy();
       mWorker->saveScene(pDocumentProxy, copiedScene.get(), pSceneIndex);

       // keep copiedScene alive until saving is finished
       mScenesToSave.append(copiedScene);
    }

    UBThumbnailAdaptor::persistScene(pDocumentProxy, pScene, pSceneIndex);
    pScene->setModified(false);

    mSceneCache.insert(pDocumentProxy, pSceneIndex, pScene);
}


std::shared_ptr<UBDocumentProxy> UBPersistenceManager::persistDocumentMetadata(std::shared_ptr<UBDocumentProxy> pDocumentProxy, bool forceImmediateSaving)
{
    //cleanupDocument(pDocumentProxy);

    if (forceImmediateSaving)
    {
        UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);
        emit documentMetadataChanged(pDocumentProxy);
    }
    else
    {
        std::shared_ptr<UBDocumentProxy> copy = pDocumentProxy->deepCopy();
        mWorker->saveMetadata(copy);
    }

    return pDocumentProxy;
}


void UBPersistenceManager::renamePage(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const int sourceIndex, const int targetIndex)
{
    UBApplication::showMessage(tr("Renaming pages (%1/%2)").arg(sourceIndex).arg(pDocumentProxy->pageCount()));
    QFile svg(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", sourceIndex));
    svg.rename(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg",  targetIndex));

    QFile thumb(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex));
    thumb.rename(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex));
}


void UBPersistenceManager::copyPage(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const int sourceIndex, const int targetIndex)
{
    QFile svg(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg",sourceIndex));
    svg.copy(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", targetIndex));

    UBSvgSubsetAdaptor::setSceneUuid(pDocumentProxy, targetIndex, QUuid::createUuid());

    QFile thumb(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex));
    thumb.copy(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex));
}


int UBPersistenceManager::sceneCount(const std::shared_ptr<UBDocumentProxy> proxy)
{
    const QString pPath = proxy->persistencePath();

    int pageIndex = 0;
    bool moreToProcess = true;
    bool addedMissingZeroPage = false;

    while (moreToProcess)
    {
        QString fileName = pPath + UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex);

        QFile file(fileName);

        if (file.exists())
        {
            pageIndex++;
        }
        else
        {
            moreToProcess = false;
        }
    }

    if(pageIndex == 1 && addedMissingZeroPage){
        // increment is done only to check if there are other pages than the missing zero page
        // This situation means -> no pages on the document
        return 0;
    }

    return pageIndex;
}

QStringList UBPersistenceManager::getSceneFileNames(const QString& folder)
{
    QDir dir(folder, "page???.svg", QDir::Name, QDir::Files);
    return dir.entryList();
}

QString UBPersistenceManager::generateUniqueDocumentPath(const QString& baseFolder)
{
    QDateTime now = QDateTime::currentDateTime();
    QString dirName = now.toString("yyyy-MM-dd hh-mm-ss.zzz");

    return baseFolder + QString("/OpenBoard Document %1").arg(dirName);
}

QString UBPersistenceManager::generateUniqueDocumentPath()
{
    return generateUniqueDocumentPath(UBSettings::userDocumentDirectory());
}


void UBPersistenceManager::generatePathIfNeeded(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    if (pDocumentProxy->persistencePath().length() == 0)
    {
        pDocumentProxy->setPersistencePath(generateUniqueDocumentPath());
    }
}


bool UBPersistenceManager::addDirectoryContentToDocument(const QString& documentRootFolder, std::shared_ptr<UBDocumentProxy> pDocument)
{
    QStringList sourceScenes = getSceneFileNames(documentRootFolder);
    if (sourceScenes.empty())
        return false;

    int targetPageCount = pDocument->pageCount();

    for(int sourceIndex = 0 ; sourceIndex < sourceScenes.size(); sourceIndex++)
    {
        int targetIndex = targetPageCount + sourceIndex;

        QFile svg(documentRootFolder + "/" + sourceScenes[sourceIndex]);
        if (!svg.copy(pDocument->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", targetIndex)))
            return false;

        UBSvgSubsetAdaptor::setSceneUuid(pDocument, targetIndex, QUuid::createUuid());

        QFile thumb(documentRootFolder + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex));
        // We can ignore error in this case, thumbnail will be genarated
        thumb.copy(pDocument->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex));
    }

    foreach(QString dir, mDocumentSubDirectories)
    {
        qDebug() << "copying " << documentRootFolder << "/" << dir << " to " << pDocument->persistencePath() << "/" + dir;

        QDir srcDir(documentRootFolder + "/" + dir);
        if (srcDir.exists())
            if (!UBFileSystemUtils::copyDir(documentRootFolder + "/" + dir, pDocument->persistencePath() + "/" + dir))
                return false;
    }

    pDocument->setPageCount(pDocument->pageCount());

    //issue NC - NNE - 20131213 : At this point, all is well done.
    return true;
}


bool UBPersistenceManager::isEmpty(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    if(!pDocumentProxy)
        return true;

    if (pDocumentProxy->pageCount() > 1)
        return false;

    std::shared_ptr<UBGraphicsScene> theSoleScene = UBSvgSubsetAdaptor::loadScene(pDocumentProxy, 0);

    bool empty = false;

    if (theSoleScene)
    {
        empty = theSoleScene->isEmpty();
    }
    else
    {
        empty = true;
    }

    return empty;
}

bool UBPersistenceManager::addFileToDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy,
                                                     QString path,
                                                     const QString& subdir,
                                                     QUuid objectUuid,
                                                     QString& destinationPath,
                                                     QByteArray* data)
{
    Q_ASSERT(path.length());
    QFileInfo fi(path);

    if (!pDocumentProxy || objectUuid.isNull())
        return false;
    if (data == NULL && !fi.exists())
        return false;

    qDebug() << fi.suffix();

    QString fileName = subdir + "/" + objectUuid.toString() + "." + fi.suffix();

    destinationPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destinationPath))
    {
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + subdir);
        if (!QFile::exists(pDocumentProxy->persistencePath() + "/" + subdir))
            return false;

        if (data == NULL)
        {
            QFile source(path);
            return source.copy(destinationPath);
        }
        else
        {
            QFile newFile(destinationPath);

            if (newFile.open(QIODevice::WriteOnly))
            {
                qint64 n = newFile.write(*data);
                newFile.flush();
                newFile.close();
                return n == data->size();
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
}

bool UBPersistenceManager::addGraphicsWidgetToDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy,
                                                       QString path,
                                                       QUuid objectUuid,
                                                       QString& destinationPath)
{
    QFileInfo fi(path);

    if (!fi.exists() || !pDocumentProxy || objectUuid.isNull())
        return false;

    QString widgetRootDir = path;
    QString extension = QFileInfo(widgetRootDir).suffix();

    destinationPath = pDocumentProxy->persistencePath() + "/" + widgetDirectory +  "/" + objectUuid.toString() + "." + extension;

    if (!QFile::exists(destinationPath)) {
        QDir dir;
        if (!dir.mkpath(destinationPath))
            return false;
        return UBFileSystemUtils::copyDir(widgetRootDir, destinationPath);
    }
    else
        return false;
}


void UBPersistenceManager::documentRepositoryChanged(const QString& path)
{
    Q_UNUSED(path);
    checkIfDocumentRepositoryExists();
}


void UBPersistenceManager::checkIfDocumentRepositoryExists()
{
    QDir rp(mDocumentRepositoryPath);

    if (!rp.exists())
    {
        // we have lost the document repository ..

        QString humanPath = QDir::cleanPath(mDocumentRepositoryPath);
        humanPath = QDir::toNativeSeparators(humanPath);

        UBApplication::mainWindow->warning(tr("Document Repository Loss"),tr("OpenBoard has lost access to the document repository '%1'. Unfortunately the application must shut down to avoid data corruption. Latest changes may be lost as well.").arg(humanPath));

        UBApplication::quit();
    }
}

void UBPersistenceManager::saveFoldersTreeToXml(QXmlStreamWriter &writer, const QModelIndex &parentIndex)
{
    for (int i = 0; i < mDocumentTreeStructureModel->rowCount(parentIndex); i++)
    {
        QModelIndex currentIndex = mDocumentTreeStructureModel->index(i, 0, parentIndex);
        if (mDocumentTreeStructureModel->isCatalog(currentIndex))
        {
            writer.writeStartElement(tFolder);
            writer.writeAttribute(aName, mDocumentTreeStructureModel->nodeFromIndex(currentIndex)->nodeName());
            saveFoldersTreeToXml(writer, currentIndex);
            writer.writeEndElement();
        }
    }
}

void UBPersistenceManager::loadFolderTreeFromXml(const QString &path, const QDomElement &element)
{

    QDomElement iterElement = element.firstChildElement();
    while(!iterElement.isNull())
    {
        QString leafPath;
        if (tFolder == iterElement.tagName())
        {
            leafPath = iterElement.attribute(aName);

            if (!leafPath.isEmpty())
            {
                mDocumentTreeStructureModel->goTo(path + "/" + leafPath);
                if (!iterElement.firstChildElement().isNull())
                    loadFolderTreeFromXml(path + "/" +  leafPath, iterElement);
            }
        }
        iterElement = iterElement.nextSiblingElement();
    }
}

void UBPersistenceManager::cleanupDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy) const
{
    static QRegularExpression uuidPattern("{[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}}");

    if (!pDocumentProxy->testAndResetCleanupNeeded())
    {
        return;
    }

    // scan pages and collect possible UUID references
    const QString path = pDocumentProxy->persistencePath() + "/";
    const QStringList pages = getSceneFileNames(path);

    if (pages.length() > 0)
    {
        QSet<QString> references;

        for (const QString& page : pages)
        {
            QFile svgFile(path + page);

            if (svgFile.open(QFile::ReadOnly))
            {
                const QString content = svgFile.readAll();
                auto matches = uuidPattern.globalMatch(content);

                while (matches.hasNext())
                {
                    QRegularExpressionMatch match = matches.next();
                    references << match.captured();
                }
            }
        }

        // scan folders and remove unreferenced files and directories
        static const QStringList folders = { ".", "audios", "videos", "objects" };

        for (const QString& folder : folders)
        {
            QFileInfoList entries = QDir(path + folder).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

            for (const QFileInfo& entry : entries)
            {
                QRegularExpressionMatch match = uuidPattern.match(entry.fileName());

                if (match.hasMatch() && !references.contains(match.captured()))
                {
                    const QString filename = folder + "/" + entry.fileName();
                    const QString absoluteFilePath = entry.absoluteFilePath();

                    // unreferenced file or directory
                    if (entry.isDir())
                    {
                        qWarning() << "Deleting unreferenced directory" << filename;
                        UBFileSystemUtils::deleteDir(absoluteFilePath);
                    }
                    else
                    {
                        qWarning() << "Deleting unreferenced file" << filename;
                        UBFileSystemUtils::deleteFile(absoluteFilePath);
                    }
                }
            }
        }
    }
    else
    {
        //in a network-storage context, can happen on a newly created document, depending on the client and the cache mode you're using
        qWarning() << "could not find any pages for the document " << pDocumentProxy->persistencePath();
    }
}

bool UBPersistenceManager::mayHaveVideo(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    QDir videoDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::videoDirectory);

    return videoDir.exists() && videoDir.entryInfoList().length() > 0;
}

bool UBPersistenceManager::mayHaveAudio(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    QDir audioDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::audioDirectory);

    return audioDir.exists() && audioDir.entryInfoList().length() > 0;
}

bool UBPersistenceManager::mayHavePDF(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    QDir objectDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::objectDirectory);

    QStringList filters;
    filters << "*.pdf";

    return objectDir.exists() && objectDir.entryInfoList(filters).length() > 0;
}


bool UBPersistenceManager::mayHaveSVGImages(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    QDir imageDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::imageDirectory);

    QStringList filters;
    filters << "*.svg";

    return imageDir.exists() && imageDir.entryInfoList(filters).length() > 0;
}


bool UBPersistenceManager::mayHaveWidget(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    QDir widgetDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::widgetDirectory);

    return widgetDir.exists() && widgetDir.entryInfoList(QDir::Dirs).length() > 0;
}
