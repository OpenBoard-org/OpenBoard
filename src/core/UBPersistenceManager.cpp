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

#include "UBPersistenceManager.h"
#include "gui/UBMainWindow.h"

#include <QtXml>

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "gui/UBDockTeacherGuideWidget.h"
#include "gui/UBTeacherGuideWidget.h"

#include "document/UBDocumentProxy.h"

#include "adaptors/UBExportPDF.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "interfaces/IDataStorage.h"

#include "core/memcheck.h"

const QString UBPersistenceManager::imageDirectory = "images"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::objectDirectory = "objects"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::widgetDirectory = "widgets"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::videoDirectory = "videos"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::audioDirectory = "audios"; // added to
const QString UBPersistenceManager::teacherGuideDirectory = "teacherGuideObjects";

UBPersistenceManager * UBPersistenceManager::sSingleton = 0;

UBPersistenceManager::UBPersistenceManager(QObject *pParent)
    : QObject(pParent)
    , mHasPurgedDocuments(false)
{

    mDocumentSubDirectories << imageDirectory;
    mDocumentSubDirectories << objectDirectory;
    mDocumentSubDirectories << widgetDirectory;
    mDocumentSubDirectories << videoDirectory;
    mDocumentSubDirectories << audioDirectory;
    mDocumentSubDirectories << teacherGuideDirectory;

    documentProxies = allDocumentProxies();
    emit proxyListChanged();
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

UBPersistenceManager::~UBPersistenceManager()
{
    foreach(QPointer<UBDocumentProxy> proxyGuard, documentProxies)
    {
        if (!proxyGuard.isNull())
            delete proxyGuard.data();
    }
}

QList<QPointer<UBDocumentProxy> > UBPersistenceManager::allDocumentProxies()
{
    mDocumentRepositoryPath = UBSettings::userDocumentDirectory();

    QDir rootDir(mDocumentRepositoryPath);
    rootDir.mkpath(rootDir.path());


    QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
    watcher->addPath(mDocumentRepositoryPath);

    connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(documentRepositoryChanged(const QString&)));

    QList<QPointer<UBDocumentProxy> > proxies;

    foreach(QString path, rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot,
            QDir::Time | QDir::Reversed))
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
    QString currentDate =  UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime());
    doc->setMetaData(UBSettings::documentUpdatedAt,currentDate);
    doc->setMetaData(UBSettings::documentDate,currentDate);

    if (withEmptyPage) createDocumentSceneAt(doc, 0);

    documentProxies.insert(0, QPointer<UBDocumentProxy>(doc));

    emit proxyListChanged();

    emit documentCreated(doc);

    mDocumentCreatedDuringSession << doc;

    return doc;
}

UBDocumentProxy* UBPersistenceManager::createDocumentFromDir(const QString& pDocumentDirectory, const QString& pGroupName, const QString& pName, bool withEmptyPage, bool addTitlePage)
{
    checkIfDocumentRepositoryExists();

    UBDocumentProxy* doc = new UBDocumentProxy(pDocumentDirectory); // deleted in UBPersistenceManager::destructor

    if (pGroupName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroupName);
    }

    if (pName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentName, pName);
    }
    if(withEmptyPage) createDocumentSceneAt(doc, 0);
    if(addTitlePage) persistDocumentScene(doc, mSceneCache.createScene(doc, 0, false), 0);

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

    emit proxyListChanged();

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

    emit proxyListChanged();

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

    emit proxyListChanged();

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
        emit documentSceneWillBeDeleted(proxy, index);
    }

    QString sourceGroupName = proxy->metaData(UBSettings::documentGroupName).toString();
    QString sourceName = proxy->metaData(UBSettings::documentName).toString();
    UBDocumentProxy *trashDocProxy = createDocument(UBSettings::trashedDocumentGroupNamePrefix + sourceGroupName, sourceName, false);

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
                QFile::copy(source, target);
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
    if (mSceneCache.contains(proxy, sceneIndex))
        return mSceneCache.value(proxy, sceneIndex);
    else {
        UBGraphicsScene* scene = UBSvgSubsetAdaptor::loadScene(proxy, sceneIndex);
        if(!scene && UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool()){
            createDocumentSceneAt(proxy,0);
            scene = UBSvgSubsetAdaptor::loadScene(proxy, 0);
        }

        if (scene)
            mSceneCache.insert(proxy, sceneIndex, scene);

        return scene;
    }
}

void UBPersistenceManager::persistDocumentScene(UBDocumentProxy* pDocumentProxy, UBGraphicsScene* pScene, const int pSceneIndex)
{
    checkIfDocumentRepositoryExists();

    pScene->deselectAllItems();

    generatePathIfNeeded(pDocumentProxy);

    QDir dir(pDocumentProxy->persistencePath());
    dir.mkpath(pDocumentProxy->persistencePath());

    UBBoardPaletteManager* paletteManager = UBApplication::boardController->paletteManager();
    bool teacherGuideModified = false;
    if(UBApplication::app()->boardController->currentPage() == pSceneIndex &&  paletteManager->teacherGuideDockWidget())
    	teacherGuideModified = paletteManager->teacherGuideDockWidget()->teacherGuideWidget()->isModified();

    if (pDocumentProxy->isModified() || teacherGuideModified)
        UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);

    if (pScene->isModified() || teacherGuideModified)
    {
        UBSvgSubsetAdaptor::persistScene(pDocumentProxy, pScene, pSceneIndex);

        UBThumbnailAdaptor::persistScene(pDocumentProxy, pScene, pSceneIndex);

        pScene->setModified(false);
    }

    mSceneCache.insert(pDocumentProxy, pSceneIndex, pScene);
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
            if(UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool() && pageIndex == 0){
                // the document has no zero file but doesn't means that it hasn't any file
                // at all. Just importing a document without the first page using a configuartion
                // that enables zero page.
                pageIndex++;
                addedMissingZeroPage = true;
            }
            else
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

    return baseFolder + QString("/Sankore Document %1").arg(dirName);
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


void UBPersistenceManager::upgradeDocumentIfNeeded(UBDocumentProxy* pDocumentProxy)
{
    int pageCount = pDocumentProxy->pageCount();

    for(int index = 0 ; index < pageCount; index++)
    {
        UBSvgSubsetAdaptor::upgradeScene(pDocumentProxy, index);
    }

    pDocumentProxy->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);

    UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);
}


void UBPersistenceManager::upgradeAllDocumentsIfNeeded()
{
    foreach(QPointer<UBDocumentProxy> proxy, documentProxies)
    {
        upgradeDocumentIfNeeded(proxy);
    }
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

    UBGraphicsScene *theSoleScene = UBSvgSubsetAdaptor::loadScene(pDocumentProxy, 0);

    bool empty = false;

    if (theSoleScene)
    {
        empty = theSoleScene->isEmpty();
        delete theSoleScene;
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

QString UBPersistenceManager::teacherGuideAbsoluteObjectPath(UBDocumentProxy* pDocumentProxy)
{
    return pDocumentProxy->persistencePath() + "/" + teacherGuideDirectory;
}

QString UBPersistenceManager::addObjectToTeacherGuideDirectory(UBDocumentProxy* pDocumentProxy, QString pPath)
{
    QString path = UBFileSystemUtils::removeLocalFilePrefix(pPath);
	QFileInfo fi(path);
    QString uuid = QUuid::createUuid();

    if (!fi.exists() || !pDocumentProxy)
        return "";

    QString fileName = UBPersistenceManager::teacherGuideDirectory + "/" + uuid + "." + fi.suffix();

    QString destPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destPath)){
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::teacherGuideDirectory);

        QFile source(path);

        source.copy(destPath);
    }

    return destPath;
}

QString UBPersistenceManager::addWidgetToTeacherGuideDirectory(UBDocumentProxy* pDocumentProxy, QString pPath)
{
    QString path = UBFileSystemUtils::removeLocalFilePrefix(pPath);
    QFileInfo fi(path);
    Q_ASSERT(fi.isDir());

    int lastIndex = path.lastIndexOf(".");
    QString extension("");
    if(lastIndex != -1)
        extension = path.right(path.length() - lastIndex);

    QString uuid = QUuid::createUuid();

    if (!fi.exists() || !pDocumentProxy)
        return "";

    QString directoryName = UBPersistenceManager::teacherGuideDirectory + "/" + uuid + extension;
    QString destPath = pDocumentProxy->persistencePath() + "/" + directoryName;

    if (!QDir(destPath).exists()){
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::teacherGuideDirectory);
        UBFileSystemUtils::copyDir(path,destPath);
    }

    return destPath;
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

bool UBPersistenceManager::addGraphicsWidgteToDocument(UBDocumentProxy *pDocumentProxy, 
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

        UBApplication::mainWindow->warning(tr("Document Repository Loss"),tr("Sankore has lost access to the document repository '%1'. Unfortunately the application must shut down to avoid data corruption. Latest changes may be lost as well.").arg(humanPath));

        UBApplication::quit();
    }
}


bool UBPersistenceManager::mayHaveVideo(UBDocumentProxy* pDocumentProxy)
{
    QDir videoDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::videoDirectory);

    return videoDir.exists() && videoDir.entryInfoList().length() > 0;
}

bool UBPersistenceManager::mayHaveAudio(UBDocumentProxy* pDocumentProxy)
{
    QDir audioDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::audioDirectory);

    return audioDir.exists() && audioDir.entryInfoList().length() > 0;
}

bool UBPersistenceManager::mayHavePDF(UBDocumentProxy* pDocumentProxy)
{
    QDir objectDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::objectDirectory);

    QStringList filters;
    filters << "*.pdf";

    return objectDir.exists() && objectDir.entryInfoList(filters).length() > 0;
}


bool UBPersistenceManager::mayHaveSVGImages(UBDocumentProxy* pDocumentProxy)
{
    QDir imageDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::imageDirectory);

    QStringList filters;
    filters << "*.svg";

    return imageDir.exists() && imageDir.entryInfoList(filters).length() > 0;
}


bool UBPersistenceManager::mayHaveWidget(UBDocumentProxy* pDocumentProxy)
{
    QDir widgetDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::widgetDirectory);

    return widgetDir.exists() && widgetDir.entryInfoList(QDir::Dirs).length() > 0;
}
