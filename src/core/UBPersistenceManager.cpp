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

#include "adaptors/UBSvgSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "board/UBBoardController.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "document/UBDocument.h"
#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"
#include "document/UBToc.h"

#include "frameworks/UBFileSystemUtils.h"

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
    QFuture<std::shared_ptr<UBDocumentProxy>> proxiesFuture = QtConcurrent::mapped(contentInfoList, &UBPersistenceManager::createDocumentProxyStructure);
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

std::shared_ptr<UBDocumentProxy> UBPersistenceManager::createDocumentProxyStructure(const QFileInfo& contentInfo)
{
    QString fullPath = contentInfo.absoluteFilePath();

    QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(fullPath);
    QString docName = metadatas.value(UBSettings::documentName, QString()).toString();

    if (docName.isEmpty()) {
        qWarning() << "Could not find a document name: " << fullPath;
        metadatas.insert(UBSettings::documentName, tr("Retrieved - %1").arg(contentInfo.baseName()));
    }

    std::shared_ptr<UBDocumentProxy> docProxy = std::make_shared<UBDocumentProxy>(fullPath);
    foreach(QString key, metadatas.keys())
    {
        docProxy->setMetaData(key, metadatas.value(key));
    }

    return docProxy;
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

/**
 * @brief Create a list of all SVG page files ordered by number
 * @param documentFolder Document folder
 * @return Ordered list of page files
 */
QStringList UBPersistenceManager::pageFiles(const QString& documentFolder)
{
    // create list of SVG files
    QDir dir{documentFolder};
    auto svgFiles = dir.entryList({ "page*.svg"}, QDir::Files);

    // sort list by page number
    std::sort(svgFiles.begin(), svgFiles.end(), [](QString& left, QString& right){
        return left.mid(4, left.length() - 8).toInt() < right.mid(4, right.length() - 8).toInt();
    });

    return svgFiles;
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

    generatePathIfNeeded(doc);
    QDir dir(doc->persistencePath());
    if (!dir.mkpath(doc->persistencePath()))
    {
        return nullptr; // if we can't create the path, abort function.
    }

    UBToc toc{doc->persistencePath()};

    if (withEmptyPage)
    {
        const auto pageId = toc.insert(0);
        const auto scene = createDocumentSceneAt(doc, pageId);
        toc.setUuid(0, scene->uuid());
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
        toc.save();
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
    auto document = UBDocument::getDocument(doc);

    for(int i = 0; i < document->pageCount(); i++)
    {
        UBSvgSubsetAdaptor::setSceneUuid(doc, i, QUuid::createUuid());
        UBThumbnailAdaptor::generateMissingThumbnail(document.get(), i);
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

    foreach(QString key, pDocumentProxy->metaDatas().keys())
    {
        copy->setMetaData(key, pDocumentProxy->metaDatas().value(key));
    }    

    copy->setMetaData(UBSettings::documentName,
            pDocumentProxy->metaData(UBSettings::documentName).toString() + " " + tr("(copy)"));

    copy->setUuid(QUuid::createUuid());

    persistDocumentMetadata(copy);

    emit documentCreated(copy);

    return copy;

}


void UBPersistenceManager::deleteDocumentScenes(std::shared_ptr<UBDocumentProxy> proxy, const QList<int>& pageIds)
{
    checkIfDocumentRepositoryExists();

    if (pageIds.size() == 0)
    {
        return;
    }

    for (auto pageId : pageIds)
    {
        QString svgFileName = proxy->persistencePath() + sceneFilenameForId(pageId);
        QFile::remove(svgFileName);

        QString thumbFileName = proxy->persistencePath() + thumbnailFilenameForId(pageId);
        QFile::remove(thumbFileName);

        mSceneCache.removeScene(proxy, pageId);
    }

    // update metadata
    QDateTime now = QDateTime::currentDateTime();
    proxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(now));
    persistDocumentMetadata(proxy);
}


QUuid UBPersistenceManager::copyDocumentScene(std::shared_ptr<UBDocumentProxy> from, int fromPageId,
                                             std::shared_ptr<UBDocumentProxy> to, int toPageId, QList<QString> dependencies)
{
    checkIfDocumentRepositoryExists();

    auto scene = UBApplication::boardController->activeScene();

    // save modified scene of the same document
    if (scene && scene->document() == from && scene->isModified())
    {
        auto page = UBApplication::boardController->activeSceneIndex();
        persistDocumentScene(from, scene, page, false, true);
    }

    // copy SVG and thumbnail
    const QUuid uuid = copyPage(from, fromPageId, to, toPageId);

    // copy dependencies, don't overwrite
    for (const auto relativeFile : dependencies)
    {
        QString source = from->persistencePath() + "/" + relativeFile;
        QString target = to->persistencePath() + "/" + relativeFile;
        UBFileSystemUtils::copy(source, target, false);
    }

    // update metadata
    QDateTime now = QDateTime::currentDateTime();
    to->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(now));
    persistDocumentMetadata(to);

    return uuid;
}


std::shared_ptr<UBGraphicsScene> UBPersistenceManager::createDocumentSceneAt(std::shared_ptr<UBDocumentProxy> proxy, int pageId, bool useUndoRedoStack)
{
    std::shared_ptr<UBGraphicsScene> newScene = mSceneCache.createScene(proxy, pageId, useUndoRedoStack);

    newScene->setBackground(UBSettings::settings()->isDarkBackground(),
            UBSettings::settings()->UBSettings::pageBackground());

    newScene->setBackgroundGridSize(UBSettings::settings()->crossSize);

    persistDocumentScene(proxy, newScene, pageId);

    return newScene;
}

std::shared_ptr<UBGraphicsScene> UBPersistenceManager::loadDocumentScene(std::shared_ptr<UBDocumentProxy> proxy, int pageId)
{
    mSceneCache.prepareLoading(proxy, pageId);
    return mSceneCache.value(proxy, pageId);
}

void UBPersistenceManager::prepareSceneLoading(std::shared_ptr<UBDocumentProxy> proxy, int pageId)
{
    if (!mSceneCache.contains(proxy, pageId))
    {
        mSceneCache.prepareLoading(proxy, pageId);
    }
}

std::shared_ptr<UBGraphicsScene> UBPersistenceManager::getDocumentScene(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int pageId)
{
    return mSceneCache.value(pDocumentProxy, pageId);
}

void UBPersistenceManager::persistDocumentScene(std::shared_ptr<UBDocumentProxy> pDocumentProxy, std::shared_ptr<UBGraphicsScene> pScene, int pageId, bool isAnAutomaticBackup, bool forceImmediateSaving)
{
    checkIfDocumentRepositoryExists();

    if (!isAnAutomaticBackup)
        pScene->deselectAllItems();

    generatePathIfNeeded(pDocumentProxy);

    QDir dir(pDocumentProxy->persistencePath());
    dir.mkpath(pDocumentProxy->persistencePath());

    if(forceImmediateSaving)
    {
        UBSvgSubsetAdaptor::persistScene(pDocumentProxy, pScene, pageId);
    }
    else
    {
       std::shared_ptr<UBGraphicsScene> copiedScene = pScene->sceneDeepCopy();
       mWorker->saveScene(pDocumentProxy, copiedScene.get(), pageId);

       // keep copiedScene alive until saving is finished
       mScenesToSave.append(copiedScene);
    }

    pScene->setModified(false);

    mSceneCache.insert(pDocumentProxy, pageId, pScene);
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


QUuid UBPersistenceManager::copyPage(std::shared_ptr<UBDocumentProxy> source, const int sourceIndex, std::shared_ptr<UBDocumentProxy> target, const int targetIndex)
{
    QFile thumb(source->persistencePath() + thumbnailFilenameForId(sourceIndex));
    thumb.copy(target->persistencePath() + thumbnailFilenameForId(targetIndex));

    const QString sourcePath = source->persistencePath() + sceneFilenameForId(sourceIndex);
    const QString targetPath = target->persistencePath() + sceneFilenameForId(targetIndex);

    const QUuid uuid{QUuid::createUuid()};
    UBSvgSubsetAdaptor::replicateScene(sourcePath, targetPath, uuid);
    return uuid;
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

QString UBPersistenceManager::sceneFilenameForId(int id)
{
    return UBFileSystemUtils::digitFileFormat("/page%1.svg", id);
}

QString UBPersistenceManager::thumbnailFilenameForId(int id)
{
    return UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", id);
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
    auto proxy = std::make_shared<UBDocumentProxy>(documentRootFolder);
    auto sourceDocument = UBDocument::getDocument(proxy);

    if (sourceDocument->pageCount() == 0)
    {
        return false;
    }

    auto targetDocument = UBDocument::getDocument(pDocument);
    int targetPageCount = targetDocument->pageCount();

    for (int sourceIndex = 0; sourceIndex < sourceDocument->pageCount(); sourceIndex++)
    {
        int targetIndex = targetPageCount + sourceIndex;
        sourceDocument->copyPage(sourceIndex, targetDocument, targetIndex);
    }

    //issue NC - NNE - 20131213 : At this point, all is well done.
    return true;
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
        return true;
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
        return true;
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
