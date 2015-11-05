/*
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

#include "core/memcheck.h"

const QString UBPersistenceManager::imageDirectory = "images"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::objectDirectory = "objects"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::widgetDirectory = "widgets"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::videoDirectory = "videos"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::audioDirectory = "audios"; // added to

UBPersistenceManager * UBPersistenceManager::sSingleton = 0;

UBPersistenceManager::UBPersistenceManager(QObject *pParent)
    : QObject(pParent)
    , mHasPurgedDocuments(false)
    , mIsWorkerFinished(false)
{

    mDocumentSubDirectories << imageDirectory;
    mDocumentSubDirectories << objectDirectory;
    mDocumentSubDirectories << widgetDirectory;
    mDocumentSubDirectories << videoDirectory;
    mDocumentSubDirectories << audioDirectory;

    documentProxies = allDocumentProxies();

    mThread = new QThread;
    mWorker = new UBPersistenceWorker();
    mWorker->moveToThread(mThread);
    connect(mWorker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(mThread, SIGNAL(started()), mWorker, SLOT(process()));
    connect(mWorker, SIGNAL(finished()), mThread, SLOT(quit()));
    connect(mWorker, SIGNAL(finished()), this, SLOT(onWorkerFinished()));
    connect(mWorker, SIGNAL(finished()), mWorker, SLOT(deleteLater()));
    connect(mThread, SIGNAL(finished()), mThread, SLOT(deleteLater()));
    connect(mWorker,SIGNAL(sceneLoaded(QByteArray,UBDocumentProxy*,int)),this,SLOT(onSceneLoaded(QByteArray,UBDocumentProxy*,int)));
    connect(mWorker,SIGNAL(scenePersisted(UBGraphicsScene*)),this,SLOT(onScenePersisted(UBGraphicsScene*)));
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

void UBPersistenceManager::onScenePersisted(UBGraphicsScene* scene)
{
    delete scene;
    scene = NULL;
}

void UBPersistenceManager::onWorkerFinished()
{
    mIsWorkerFinished = true;
}

UBPersistenceManager::~UBPersistenceManager()
{
    if(mWorker)
        mWorker->applicationWillClose();

    QTime time;
    time.start();
    qDebug() << "start waiting";

    while(!mIsWorkerFinished)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    qDebug() << "stop waiting after " << time.elapsed() << " ms";

    foreach(QPointer<UBDocumentProxy> proxyGuard, documentProxies)
    {
        if (!proxyGuard.isNull())
            delete proxyGuard.data();
    }

    // to be sure that all the scenes are stored on disk
}

void UBPersistenceManager::errorString(QString error)
{
    qDebug() << "peristence thread return the error " << error;
}

void UBPersistenceManager::onSceneLoaded(QByteArray scene, UBDocumentProxy* proxy, int sceneIndex)
{
    qDebug() << "scene loaded " << sceneIndex;
    QTime time;
    time.start();
    mSceneCache.insert(proxy,sceneIndex,UBSvgSubsetAdaptor::loadScene(proxy,scene));
    qDebug() << "millisecond for sceneCache " << time.elapsed();
}

QList<QPointer<UBDocumentProxy> > UBPersistenceManager::allDocumentProxies()
{
    mDocumentRepositoryPath = UBSettings::userDocumentDirectory();

    QDir rootDir(mDocumentRepositoryPath);
    rootDir.mkpath(rootDir.path());


    QStringList dirList = rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);

    foreach(QString path, dirList)
    {
        shiftPagesToStartWithTheZeroOne(rootDir.path() + "/" + path);
    }

//    QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
//    watcher->addPath(mDocumentRepositoryPath);

//    connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(documentRepositoryChanged(const QString&)));

    QList<QPointer<UBDocumentProxy> > proxies;

    foreach(QString path, dirList)
    {
        QString fullPath = rootDir.path() + "/" + path;

        QDir dir(fullPath);

        if (dir.entryList(QDir::Files | QDir::NoDotAndDotDot).size() > 0)
        {
            UBDocumentProxy* proxy = new UBDocumentProxy(fullPath); // deleted in UBPersistenceManager::destructor

            QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(fullPath);

            foreach(QString key, metadatas.keys())
            {
                proxy->setMetaData(key, metadatas.value(key));
            }

            proxy->setPageCount(sceneCount(proxy));

            proxies << QPointer<UBDocumentProxy>(proxy);

        }
    }

    return proxies;
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


UBDocumentProxy* UBPersistenceManager::createDocument(const QString& pGroupName, const QString& pName, bool withEmptyPage)
{
    checkIfDocumentRepositoryExists();

    UBDocumentProxy *doc = new UBDocumentProxy(); // deleted in UBPersistenceManager::destructor

    if (pGroupName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroupName);
    }

    if (pName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentName, pName);
    }

    doc->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);
    QString currentDate = UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime());
    doc->setMetaData(UBSettings::documentUpdatedAt,currentDate);
    doc->setMetaData(UBSettings::documentDate,currentDate);

    if (withEmptyPage)
        createDocumentSceneAt(doc, 0);
    else
        generatePathIfNeeded(doc);

    documentProxies.insert(0, QPointer<UBDocumentProxy>(doc));

    emit documentCreated(doc);

    mDocumentCreatedDuringSession << doc;

    return doc;
}

UBDocumentProxy* UBPersistenceManager::createDocumentFromDir(const QString& pDocumentDirectory, const QString& pGroupName, const QString& pName)
{
    checkIfDocumentRepositoryExists();

    UBPersistenceManager::persistenceManager()->shiftPagesToStartWithTheZeroOne(pDocumentDirectory);

    UBDocumentProxy* doc = new UBDocumentProxy(pDocumentDirectory); // deleted in UBPersistenceManager::destructor

    if (pGroupName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroupName);
    }

    if (pName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentName, pName);
    }

    QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(pDocumentDirectory);

    foreach(QString key, metadatas.keys())
    {
        doc->setMetaData(key, metadatas.value(key));
    }

    doc->setUuid(QUuid::createUuid());
    doc->setPageCount(sceneCount(doc));

    UBMetadataDcSubsetAdaptor::persist(doc);

    for(int i = 0; i < doc->pageCount(); i++)
    {
        UBSvgSubsetAdaptor::setSceneUuid(doc, i, QUuid::createUuid());
    }

    documentProxies << QPointer<UBDocumentProxy>(doc);


    emit documentCreated(doc);

    return doc;
}


void UBPersistenceManager::deleteDocument(UBDocumentProxy* pDocumentProxy)
{
    checkIfDocumentRepositoryExists();

    emit documentWillBeDeleted(pDocumentProxy);

    UBFileSystemUtils::deleteDir(pDocumentProxy->persistencePath());

    documentProxies.removeAll(QPointer<UBDocumentProxy>(pDocumentProxy));
    mDocumentCreatedDuringSession.removeAll(pDocumentProxy);

    mSceneCache.removeAllScenes(pDocumentProxy);

    pDocumentProxy->deleteLater();

}


UBDocumentProxy* UBPersistenceManager::duplicateDocument(UBDocumentProxy* pDocumentProxy)
{
    checkIfDocumentRepositoryExists();

    UBDocumentProxy *copy = new UBDocumentProxy(); // deleted in UBPersistenceManager::destructor

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

    copy->setPageCount(sceneCount(copy));

    documentProxies << QPointer<UBDocumentProxy>(copy);

    emit documentCreated(copy);

    return copy;

}


void UBPersistenceManager::deleteDocumentScenes(UBDocumentProxy* proxy, const QList<int>& indexes)
{
    checkIfDocumentRepositoryExists();

    int pageCount = UBPersistenceManager::persistenceManager()->sceneCount(proxy);

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

    foreach(int index, compactedIndexes)
    {
        // trig the reload of the thumbnails
        emit documentSceneWillBeDeleted(proxy, index);
    }

    QString sourceGroupName = proxy->metaData(UBSettings::documentGroupName).toString();
    QString sourceName = proxy->metaData(UBSettings::documentName).toString();
    UBDocumentProxy *trashDocProxy = createDocument(UBSettings::trashedDocumentGroupNamePrefix + sourceGroupName, sourceName, false);
    generatePathIfNeeded(trashDocProxy);

    foreach(int index, compactedIndexes)
    {
        UBGraphicsScene *scene = loadDocumentScene(proxy, index);
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
                QFile::rename(source, target);
            }

            insertDocumentSceneAt(trashDocProxy, scene, trashDocProxy->pageCount());
        }
    }

    for (int i = 1; i < pageCount; i++)
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

    qSort(compactedIndexes);

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




void UBPersistenceManager::duplicateDocumentScene(UBDocumentProxy* proxy, int index)
{
    checkIfDocumentRepositoryExists();

    int pageCount = UBPersistenceManager::persistenceManager()->sceneCount(proxy);

    for (int i = pageCount; i > index + 1; i--)
    {
        renamePage(proxy, i - 1 , i);

        mSceneCache.moveScene(proxy, i - 1, i);

    }

    copyPage(proxy, index , index + 1);


    //TODO: write a proper way to handle object on disk
    UBGraphicsScene *scene = loadDocumentScene(proxy, index + 1);

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
            mediaItem->mediaFileUrl(QUrl::fromLocalFile(destination));
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
            QString source = proxy->persistencePath() + "/" +  UBPersistenceManager::imageDirectory + "/" + pixmapItem->uuid().toString() + ".png";
            QString destination = source;
            QUuid newUuid = QUuid::createUuid();
            QString fileName = QFileInfo(source).completeBaseName();
            destination = destination.replace(fileName,newUuid.toString());
            QFile::copy(source,destination);
            pixmapItem->setUuid(newUuid);
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

    persistDocumentScene(proxy,scene, index + 1);

    proxy->incPageCount();

    emit documentSceneCreated(proxy, index + 1);
}


UBGraphicsScene* UBPersistenceManager::createDocumentSceneAt(UBDocumentProxy* proxy, int index, bool useUndoRedoStack)
{
    int count = sceneCount(proxy);

    for(int i = count - 1; i >= index; i--)
        renamePage(proxy, i , i + 1);

    mSceneCache.shiftUpScenes(proxy, index, count -1);

    UBGraphicsScene *newScene = mSceneCache.createScene(proxy, index, useUndoRedoStack);

    newScene->setBackground(UBSettings::settings()->isDarkBackground(),
            UBSettings::settings()->UBSettings::isCrossedBackground());

    persistDocumentScene(proxy, newScene, index);

    proxy->incPageCount();

    emit documentSceneCreated(proxy, index);

    return newScene;
}


void UBPersistenceManager::insertDocumentSceneAt(UBDocumentProxy* proxy, UBGraphicsScene* scene, int index)
{
    scene->setDocument(proxy);

    int count = sceneCount(proxy);

    for(int i = count - 1; i >= index; i--)
    {
        renamePage(proxy, i , i + 1);
    }

    mSceneCache.shiftUpScenes(proxy, index, count -1);

    mSceneCache.insert(proxy, index, scene);

    persistDocumentScene(proxy, scene, index);

    proxy->incPageCount();

    emit documentSceneCreated(proxy, index);

}


void UBPersistenceManager::moveSceneToIndex(UBDocumentProxy* proxy, int source, int target)
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


UBGraphicsScene* UBPersistenceManager::loadDocumentScene(UBDocumentProxy* proxy, int sceneIndex)
{
    UBGraphicsScene* scene = NULL;

    if (mSceneCache.contains(proxy, sceneIndex))
        scene = mSceneCache.value(proxy, sceneIndex);
    else {
        scene = UBSvgSubsetAdaptor::loadScene(proxy, sceneIndex);

        if (scene)
            mSceneCache.insert(proxy, sceneIndex, scene);
    }

    if(sceneIndex + 1 < proxy->pageCount() &&  !mSceneCache.contains(proxy, sceneIndex + 1))
        mWorker->readScene(proxy,sceneIndex+1);

    if(sceneIndex - 1 >= 0 &&  !mSceneCache.contains(proxy, sceneIndex - 1))
        mWorker->readScene(proxy,sceneIndex-1);

    return scene;
}

void UBPersistenceManager::persistDocumentScene(UBDocumentProxy* pDocumentProxy, UBGraphicsScene* pScene, const int pSceneIndex, bool isAnAutomaticBackup, bool forceImmediateSaving)
{
    checkIfDocumentRepositoryExists();

    if(!isAnAutomaticBackup)
        pScene->deselectAllItems();

    generatePathIfNeeded(pDocumentProxy);

    QDir dir(pDocumentProxy->persistencePath());
    dir.mkpath(pDocumentProxy->persistencePath());

    mSceneCache.insert(pDocumentProxy, pSceneIndex, pScene);

    if (pDocumentProxy->isModified())
        UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);

    if (pScene->isModified())
    {
        UBThumbnailAdaptor::persistScene(pDocumentProxy, pScene, pSceneIndex);
        if(forceImmediateSaving)
            UBSvgSubsetAdaptor::persistScene(pDocumentProxy,pScene,pSceneIndex);
        else{
            UBGraphicsScene* copiedScene = pScene->sceneDeepCopy();
            mWorker->saveScene(pDocumentProxy, copiedScene, pSceneIndex);
            pScene->setModified(false);

        }
    }
}


UBDocumentProxy* UBPersistenceManager::persistDocumentMetadata(UBDocumentProxy* pDocumentProxy)
{
    UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);

    emit documentMetadataChanged(pDocumentProxy);

    return pDocumentProxy;
}


void UBPersistenceManager::renamePage(UBDocumentProxy* pDocumentProxy, const int sourceIndex, const int targetIndex)
{
    QFile svg(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", sourceIndex));
    svg.rename(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg",  targetIndex));

    QFile thumb(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex));
    thumb.rename(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex));
}


void UBPersistenceManager::copyPage(UBDocumentProxy* pDocumentProxy, const int sourceIndex, const int targetIndex)
{
    QFile svg(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg",sourceIndex));
    svg.copy(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", targetIndex));

    UBSvgSubsetAdaptor::setSceneUuid(pDocumentProxy, targetIndex, QUuid::createUuid());

    QFile thumb(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex));
    thumb.copy(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex));
}


int UBPersistenceManager::sceneCount(const UBDocumentProxy* proxy)
{
    const QString pPath = proxy->persistencePath();
    int pageIndex = 0;
    bool moreToProcess = true;

    while (moreToProcess)
    {
        QString fileName = pPath + UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex);

        QFile file(fileName);

        if (file.exists())
            pageIndex++;
        else
            moreToProcess = false;
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


void UBPersistenceManager::generatePathIfNeeded(UBDocumentProxy* pDocumentProxy)
{
    if (pDocumentProxy->persistencePath().length() == 0)
    {
        pDocumentProxy->setPersistencePath(generateUniqueDocumentPath());
    }
}


bool UBPersistenceManager::addDirectoryContentToDocument(const QString& documentRootFolder, UBDocumentProxy* pDocument)
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

    pDocument->setPageCount(sceneCount(pDocument));

    return false;
}


UBDocumentProxy* UBPersistenceManager::documentByUuid(const QUuid& pUuid)
{
    for(int i = 0 ; i < documentProxies.length(); i++)
    {
        UBDocumentProxy* proxy = documentProxies.at(i);

        if (proxy && proxy->uuid() == pUuid)
        {
            return proxy;
        }
    }

    return 0;

}


bool UBPersistenceManager::isEmpty(UBDocumentProxy* pDocumentProxy)
{
    if(!pDocumentProxy)
        return true;

    if (pDocumentProxy->pageCount() > 1)
        return false;

    UBGraphicsScene *theSoleScene = mSceneCache.value(pDocumentProxy, 0) ? mSceneCache.value(pDocumentProxy, 0) : UBSvgSubsetAdaptor::loadScene(pDocumentProxy, 0);

    bool empty = false;

    if (theSoleScene)
    {
        empty = theSoleScene->isEmpty();
        if(empty){
            mSceneCache.removeScene(pDocumentProxy,0);
            delete theSoleScene;
            theSoleScene = NULL;
        }
        else{
            //the scene can contain Delegate buttons and the selection frame
            // but this doesn't means that there is something useful on the frame
            bool usefulItemFound = false;
            foreach(QGraphicsItem* eachItem, theSoleScene->getFastAccessItems()){
                if(eachItem->type() > QGraphicsItem::UserType
                        && eachItem->type() != UBGraphicsItemType::DelegateButtonType
                        && eachItem->type() != UBGraphicsItemType::SelectionFrameType){
                    usefulItemFound = true;
                    break;
                }
            }
            if(!usefulItemFound){
                mSceneCache.removeScene(pDocumentProxy,0);
                delete theSoleScene;
                theSoleScene = NULL;
                empty = true;
            }
        }
    }
    else
    {
        empty = true;
    }

    return empty;
}


void UBPersistenceManager::purgeEmptyDocuments()
{
    if(!mHasPurgedDocuments) // hack to workaround the fact that app closing is called twice :-(
    {
        QList<UBDocumentProxy*> toBeDeleted;

        foreach(UBDocumentProxy* docProxy, mDocumentCreatedDuringSession)
        {
            if (isEmpty(docProxy))
            {
                toBeDeleted << docProxy;
            }
        }

        foreach(UBDocumentProxy* docProxy, toBeDeleted)
        {
            deleteDocument(docProxy);
        }

        mHasPurgedDocuments = true;
    }
}

bool UBPersistenceManager::addFileToDocument(UBDocumentProxy* pDocumentProxy,
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

    QString fileName = subdir + "/" + objectUuid.toString() + "." + fi.suffix();
    destinationPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destinationPath))
    {
        QDir dir;
        dir.mkpath(pDocumentProxy->persistencePath() + "/" + subdir);
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

bool UBPersistenceManager::addGraphicsWidgetToDocument(UBDocumentProxy *pDocumentProxy,
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

        UBApplication::mainWindow->warning(tr("Document Repository Loss"),qApp->applicationName() + tr("has lost access to the document repository '%1'. Unfortunately the application must shut down to avoid data corruption. Latest changes may be lost as well.").arg(humanPath));

        UBApplication::quit();
    }
}

void UBPersistenceManager::shiftPagesToStartWithTheZeroOne(QString persistencePath)
{
    if(!QFile(persistencePath + "/page000.svg").exists() && QFile(persistencePath + "/page001.svg").exists()){
        int i = 1;

        while(QFile(persistencePath + UBFileSystemUtils::digitFileFormat("/page%1.svg",i)).exists()){
            QFile svg(persistencePath + UBFileSystemUtils::digitFileFormat("/page%1.svg", i));
            svg.rename(persistencePath + UBFileSystemUtils::digitFileFormat("/page%1.svg", i-1));

            QFile thumb(persistencePath + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", i));
            thumb.rename(persistencePath + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", i-1));

            i+=1;
        }
    }
}
