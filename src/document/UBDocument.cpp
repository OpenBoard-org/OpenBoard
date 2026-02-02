/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBDocument.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentController.h"
#include "document/UBDocumentToc.h"
#include "domain/UBMediaAssetItem.h"
#include "frameworks/UBBackgroundLoader.h"
#include "gui/UBMainWindow.h"
#include "gui/UBThumbnail.h"
#include "gui/UBThumbnailScene.h"

QList<std::weak_ptr<UBDocument>> UBDocument::sDocuments;


UBDocument::UBDocument(std::shared_ptr<UBDocumentProxy> proxy)
    : mProxy(proxy)
{
}

UBDocument::~UBDocument()
{
    if (mSceneAssetLoader)
    {
        mSceneAssetLoader->abort();
        delete mSceneAssetLoader;
    }

    if (QFile::exists(mProxy->persistencePath()))
    {
        if (mSceneHeaderLoader)
        {
            assureHeaderLoaderFinished(false);
            delete mSceneHeaderLoader;
        }

        mToc->save();
        deleteUnreferencedAssets();
    }
    else if (mSceneHeaderLoader)
    {
        mSceneHeaderLoader->abort();
        delete mSceneHeaderLoader;
    }
}

std::shared_ptr<UBDocumentProxy> UBDocument::proxy() const
{
    return mProxy;
}

void UBDocument::deletePages(QList<int> indexes)
{
    if (indexes.isEmpty())
    {
        return;
    }

    bool accepted{false};

    if (indexes.size() > 1)
    {
        accepted = UBApplication::mainWindow->yesNoQuestion(
            UBDocumentController::tr("Moving %1 pages of the document \"%2\" to trash")
                .arg(QString::number(indexes.size()), mProxy->name()),
            UBDocumentController::tr("You are about to move %1 pages of the document \"%2\" to trash. Are you sure ?")
                .arg(QString::number(indexes.size()), mProxy->name()),
            QPixmap(":/images/trash-document-page.png"));
    }
    else
    {
        accepted = UBApplication::mainWindow->yesNoQuestion(
            UBDocumentController::tr("Remove page %1").arg(indexes.at(0) + 1),
            UBDocumentController::tr("You are about to remove page %1 of the document \"%2\". Are you sure ?")
                .arg(indexes.at(0) + 1)
                .arg(mProxy->name()),
            QPixmap(":/images/trash-document-page.png"));
    }

    if (!accepted)
    {
        return;
    }

    // sort list, remove duplicates
    std::sort(indexes.begin(), indexes.end());
    indexes.erase(std::unique(indexes.begin(), indexes.end()), indexes.end());

    // copy to-be-deleted pages to trash document
    QString sourceName = mProxy->metaData(UBSettings::documentName).toString();
    auto trashDocProxy = UBPersistenceManager::persistenceManager()->createDocument(UBSettings::trashedDocumentGroupNamePrefix, sourceName, false);
    auto trashDocument = UBDocument::getDocument(trashDocProxy);
    int trashIndex = 0;
    QList<int> pageIds;

    for (auto index : indexes)
    {
        copyPage(index, trashDocument, trashIndex++);
        pageIds << mToc->pageId(index);
    }

    // delete the scenes
    if (pageIds.count() == pageCount())
    {
        // deleting all pages is blocked in the UI. Just log if it happens anyway
        qWarning() << "UBDocument::deletePages: declined attempt to remove all pages of a document";
        return;
    }

    UBPersistenceManager::persistenceManager()->deleteDocumentScenes(mProxy, pageIds);
    assureHeaderLoaderFinished();

    for (int i = indexes.size() - 1; i >= 0; --i)
    {
        mToc->remove(indexes.at(i));
        thumbnailScene()->deleteThumbnail(indexes.at(i), false);
        emit UBPersistenceManager::persistenceManager()->documentSceneDeleted(this, indexes.at(i));
    }

    thumbnailScene()->renumberThumbnails(indexes.first());
    thumbnailScene()->arrangeThumbnails(indexes.first());
    mToc->save();
}

void UBDocument::duplicatePage(int index)
{
    copyPage(index, this, index + 1);
    emit UBPersistenceManager::persistenceManager()->documentSceneDuplicated(this, index + 1);
}

void UBDocument::movePage(int fromIndex, int toIndex)
{
    auto scene = getScene(fromIndex);

    if (scene && scene->isModified())
    {
        persistPage(scene, fromIndex);
    }

    assureHeaderLoaderFinished();
    thumbnailScene()->moveThumbnail(fromIndex, toIndex);
    mToc->move(fromIndex, toIndex);
    mToc->save();
    emit UBPersistenceManager::persistenceManager()->documentSceneMoved(this, fromIndex, toIndex);
}

void UBDocument::copyPage(int fromIndex, std::shared_ptr<UBDocument> to, int toIndex)
{
    copyPage(fromIndex, to.get(), toIndex);
}

std::shared_ptr<UBGraphicsScene> UBDocument::createPage(int index, bool saveToc, bool cached, bool useUndoRedoStack)
{
    // create a new TOC entry for the page
    assureHeaderLoaderFinished();
    auto pageId = mToc->insert(index);
    auto scene = UBPersistenceManager::persistenceManager()->createDocumentSceneAt(mProxy, pageId, cached, useUndoRedoStack);
    mToc->setUuid(index, scene->uuid());

    if (saveToc)
    {
        mToc->save();
    }

    thumbnailScene()->insertThumbnail(index, false);

    return scene;
}

void UBDocument::persistPage(std::shared_ptr<UBGraphicsScene> scene, int index, bool isAutomaticBackup,
                             bool forceImmediateSaving, bool persistThumbnail, bool addToCache)
{
    const auto pageId = mToc->pageId(index);

    if (persistThumbnail)
    {
        const auto pixmap = UBThumbnailAdaptor::persistScene(this, scene, index);

        if (!pixmap.isNull())
        {
            thumbnailScene()->thumbnailAt(index)->setPixmap(pixmap);
        }
    }

    UBPersistenceManager::persistenceManager()->persistDocumentScene(mProxy, scene, pageId, isAutomaticBackup,
                                                                     forceImmediateSaving, addToCache);

    const auto assets = scene->relativeDependencies();

    if (assets != mToc->assets(index))
    {
        mToc->setAssets(index, assets);

        if (addToCache)
        {
            // when scene is cached it is also time to save the TOC
            mToc->save();
        }
    }
}

std::shared_ptr<UBGraphicsScene> UBDocument::loadScene(int index, bool cacheNeighboringScenes)
{
    if (cacheNeighboringScenes)
    {
        auto persistenceManager = UBPersistenceManager::persistenceManager();

        if(index + 1 < pageCount())
        {
            persistenceManager->prepareSceneLoading(mProxy, mToc->pageId(index + 1));
        }

        if(index + 2 < pageCount())
        {
            persistenceManager->prepareSceneLoading(mProxy, mToc->pageId(index + 2));
        }

        if(index - 1 >= 0)
        {
            persistenceManager->prepareSceneLoading(mProxy, mToc->pageId(index - 1));
        }
    }

    return UBPersistenceManager::persistenceManager()->loadDocumentScene(mProxy, mToc->pageId(index));
}

std::shared_ptr<UBGraphicsScene> UBDocument::getScene(int index)
{
    return UBPersistenceManager::persistenceManager()->getDocumentScene(mProxy, mToc->pageId(index));
}

QList<QString> UBDocument::pageRelativeDependencies(int index)
{
    if (mToc->hasAssetsEntry(index))
    {
        return mToc->assets(index);
    }

    auto scene = loadScene(index, false);

    if (scene)
    {
        const auto dependencies = scene->relativeDependencies();
        mToc->setAssets(index, dependencies);
        return dependencies;
    }

    return {};
}

UBThumbnailScene* UBDocument::thumbnailScene(bool startLoader)
{
    if (!mThumbnailScene)
    {
        mThumbnailScene = std::unique_ptr<UBThumbnailScene>(new UBThumbnailScene(this));

        if (startLoader)
        {
            mThumbnailScene->createThumbnails();
        }
    }

    return mThumbnailScene.get();
}

UBDocumentToc* UBDocument::toc()
{
    if (!mToc)
    {
        mToc = new UBDocumentToc{mProxy->persistencePath()};
        const auto tocPresent = mToc->load();

        if (!tocPresent ||
                QVersionNumber::fromString(mProxy->metaData(UBSettings::documentVersion).toString()) <
                QVersionNumber::fromString(UBSettings::currentFileVersion))
        {
            scan(tocPresent);
            mToc->save();
        }

        // scan assets if missing
        for (int index = 0; index < mToc->pageCount(); ++index)
        {
            if (!mToc->hasAssetsEntry(index))
            {
                scanAssets();
                break;
            }
        }
    }

    return mToc;
}

int UBDocument::pageCount()
{
    return mToc->pageCount();
}

QString UBDocument::sceneFile(int index)
{
    if (index >= pageCount())
    {
        qWarning() << "Index " << index << "above ToC size " << pageCount();
        return "";
    }

    const auto filename = UBPersistenceManager::persistenceManager()->sceneFilenameForId(mToc->pageId(index));
    return mProxy->persistencePath() + filename;
}

QString UBDocument::thumbnailFile(int index)
{
    if (index >= pageCount())
    {
        qWarning() << "Index " << index << "above ToC size " << pageCount();
        return "";
    }

    const auto filename = UBPersistenceManager::persistenceManager()->thumbnailFilenameForId(mToc->pageId(index));
    return mProxy->persistencePath() + filename;
}

void UBDocument::sceneLoaded(UBGraphicsScene* scene, std::shared_ptr<void> handle)
{
    mLoaderHandles.erase(handle);

    if (scene)
    {
        const auto persistenceManager = UBPersistenceManager::persistenceManager();
        auto index = mToc->findUuid(scene->uuid());

        if (index >= 0 && !mToc->hasAssetsEntry(index))
        {
            const auto mediaAssetItems = scene->mediaAssetItems();

            for (auto item : mediaAssetItems)
            {
                // Check the media asset UUIDs.
                // Convert it to a SHA-1 based UUIDv5 if needed. Copy the asset file in this case.
                QUuid assetUuid = item->mediaAssetUuid();

                if (assetUuid.version() != QUuid::Sha1)
                {
                    // lookup UUIDv5 in cache or create from content
                    const auto source = item->mediaAssets().at(0);
                    const auto sourcePath = mProxy->persistencePath() + "/" + source;

                    QUuid assetUuidV5;
                    bool needsCopy = false;

                    if (mUuidV5Map.contains(assetUuid))
                    {
                        assetUuidV5 = mUuidV5Map.value(assetUuid);
                    }
                    else
                    {
                        assetUuidV5 = UBMediaAssetItem::createMediaAssetUuid(sourcePath);
                        mUuidV5Map[assetUuid] = assetUuidV5;
                        needsCopy = true;
                    }

                    auto target = source;
                    target.replace(assetUuid.toString(QUuid::WithoutBraces), assetUuidV5.toString(QUuid::WithoutBraces));

                    // copy assets
                    if (needsCopy)
                    {
                        persistenceManager->copyAsset(mProxy, source, target);
                    }

                    item->setMediaAsset(mProxy->persistencePath(), target);
                    scene->setModified(true);
                }
            }

            mToc->setAssets(index, scene->relativeDependencies());
        }

        if (scene->isModified())
        {
            persistenceManager->persistDocumentScene(mProxy, scene->shared_from_this(), mToc->pageId(index));
        }

        // generate and load thumbnail from scene if not already existing and loaded
        mThumbnailScene->ensureThumbnail(index, scene);
    }

    // prepare next scene
    QTimer::singleShot(5, mSceneAssetLoader, [this](){
        if (mSceneAssetLoader)
        {
            mSceneAssetLoader->resultProcessed();
        }
    });
}

void UBDocument::scanAssets()
{
    assureHeaderLoaderFinished();

    // create a list of page IDs and paths of pages without asset info for background loading
    QList<std::pair<int, QString>> paths;
    const auto persistenceManager = UBPersistenceManager::persistenceManager();
    persistenceManager->waitForAllSaved();

    for (int index = 0; index < mToc->pageCount(); ++index)
    {
        if (!mToc->hasAssetsEntry(index))
        {
            int pageId = mToc->pageId(index);
            paths.append({pageId, mProxy->persistencePath() + persistenceManager->sceneFilenameForId(pageId)});
        }
    }

    if (!paths.empty())
    {
        qDebug() << "Scan" << paths.count() << "scenes for assets";
        // load scenes for scanning
        mSceneAssetLoader = new UBBackgroundLoader{UBBackgroundLoader::ByteArray};

        QObject::connect(mSceneAssetLoader, &UBBackgroundLoader::resultAvailable, mSceneAssetLoader, [this](int pageId, const QVariant& data){
            UBApplication::showMessage(UBDocumentController::tr("Scanning page %1 of %2").arg(pageId).arg(mToc->pageCount()), true);

            if (pageId >= 0 && !data.isNull())
            {
                auto scene = UBPersistenceManager::persistenceManager()->getDocumentScene(mProxy, pageId);

                if (scene)
                {
                    sceneLoaded(scene.get(), nullptr);
                    return;
                }

                const auto handle = UBPersistenceManager::persistenceManager()->prepareSceneLoading(mProxy, pageId, data.toByteArray(), false);

                if (handle)
                {
                    // loading started, keep handle until finished
                    mLoaderHandles.insert(handle);
                    return;
                }
            }

            // page deleted or already in cache, loading not necessary
            mSceneAssetLoader->resultProcessed();
        });

        QObject::connect(mSceneAssetLoader, &UBBackgroundLoader::finished, mSceneAssetLoader, [this](){
            qDebug() << "Asset scan completed";
            mSceneAssetLoader->deleteLater();
            mSceneAssetLoader = nullptr;
        });

        QObject::connect(mSceneAssetLoader, &QObject::destroyed, [](){
            // loader either completed or aborted
            UBApplication::showMessage(UBDocumentController::tr("Scan terminated"));
        });

        mSceneAssetLoader->load(paths, -1, [this](int pageId, QString path){
            // check that the scene file actually exists to avoid a race condition
            // with saving scene files during import
            while (!QFile::exists(path))
            {
                QThread::msleep(10);
            }
        });
    }
}

/**
 * @brief Get document for proxy.
 *
 * Retrieves UBDocument instance for a document proxy from the list of known instances.
 * If the UBDocument object does not exist, then a new one is created and added to the list.
 * At the same time expired instances are removed from the list.
 *
 * @note If UBDocument instances should be cached, then you have to make sure that a shared
 * pointer to the instance is kept somewhere. Not-referenced instances are automatically
 * deleted.
 *
 * @param proxy The document proxy of the document.
 * @return Not-null shared pointer to the UBDocument instance related to the document proxy
 * or nullptr if (and only if) proxy is nullptr.
 */
std::shared_ptr<UBDocument> UBDocument::getDocument(std::shared_ptr<UBDocumentProxy> proxy)
{
    if (!proxy)
    {
        return nullptr;
    }

    auto document = findDocument(proxy);

    if (!document)
    {
        document = std::shared_ptr<UBDocument>(new UBDocument(proxy));
        document->toc();  // load and initialize TOC
        sDocuments << document;
    }

    return document;
}

void UBDocument::scan(bool tocPresent)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // scan through the pages starting from the smallest number and counting up.
    QStringList pages = UBPersistenceManager::persistenceManager()->pageFiles(mProxy->persistencePath());

    if (pages.isEmpty())
    {
        qWarning() << "No pages found - " << mProxy->persistencePath();
        mProxy->setMetaData(UBSettings::documentName, UBPersistenceManager::tr("Broken - %1").arg(mProxy->documentFolderName()));
    }

    // create a list of page IDs and paths for background loading
    QList<std::pair<int, QString>> paths;

    for (const auto page : pages)
    {
        int pageId = page.mid(4, page.length() - 8).toInt();
        paths.append({pageId, mProxy->persistencePath() + "/" + page});
    }

    if (tocPresent)
    {
        // we have a TOC, but the document was last modified by an earlier version of OpenBoard
        // necessary to verify all scene UUIDs and update TOC accordingly

        // create a set of all scene UUIDs in the TOC
        QSet<QUuid> tocSceneUuids;

        for (int i = 0; i < pageCount(); ++i)
        {
            tocSceneUuids.insert(mToc->uuid(i));
        }

        // load the first 300 bytes of each file in background, enough to get version and UUID
        mSceneHeaderLoader = new UBBackgroundLoader{UBBackgroundLoader::ByteArray};
        int pageProcessed = -1;
        int currentPage = 0;
        const auto currentVersion = QVersionNumber::fromString(UBSettings::currentFileVersion);

        QObject::connect(mSceneHeaderLoader, &UBBackgroundLoader::resultAvailable, mSceneHeaderLoader, [&](int index, const QVariant& data){
            UBApplication::showMessage(UBDocumentController::tr("Scanning page %1 of %2").arg(++currentPage).arg(pages.count()), true);
            const auto uuid = UBSvgSubsetAdaptor::sceneUuid(data.toByteArray());

            // Check whether the tocSceneUuids already contains this scene.
            // If yes, locate and update the TOC entry
            if (tocSceneUuids.contains(uuid))
            {
                const int index = mToc->findUuid(uuid);

                if (index >= 0)
                {
                    pageProcessed = index;
                    tocSceneUuids.remove(uuid);

                    // invalidate assets if scene was modified by an earlier version
                    if (UBSvgSubsetAdaptor::sceneVersion(data.toByteArray()) < currentVersion)
                    {
                        mToc->unsetAssets(index);
                    }
                }
            }
            else
            {
                // add the file name and thumbnail file name to the TOC after the last processed TOC entry
                qDebug() << "scan: insert scene" << index << "at offset" << pageProcessed;
                ++pageProcessed;
                mToc->insert(pageProcessed);
                mToc->setUuid(pageProcessed, uuid);
                mToc->setPageId(pageProcessed, index);
            }

            mSceneHeaderLoader->resultProcessed();
        });

        QObject::connect(mSceneHeaderLoader, &UBBackgroundLoader::finished, mSceneHeaderLoader, [&](){
            // When finished, remove all scenes still in tocSceneUuids from the TOC.
            // Those scenes might have been deleted using a previous version of OpenBoard.
            for (const auto uuid : tocSceneUuids)
            {
                qDebug() << "scan: removing scene from TOC" << uuid;
                mToc->remove(mToc->findUuid(uuid));
            }

            UBApplication::showMessage(UBDocumentController::tr("Scan completed"));
            mSceneHeaderLoader->deleteLater();
            mSceneHeaderLoader = nullptr;
            mToc->save();
        });

        mSceneHeaderLoader->load(paths, 300);
        assureHeaderLoaderFinished();
    }
    else if (!paths.empty())
    {
        // start reading the UUIDs in background, but make sure it is completed when the UBDocument is deleted
        mSceneHeaderLoader = new UBBackgroundLoader{UBBackgroundLoader::ByteArray};
        mSceneHeaderLoader->setKeepAlive(shared_from_this());

        QObject::connect(mSceneHeaderLoader, &UBBackgroundLoader::resultAvailable, mSceneHeaderLoader, [this](int index, const QVariant& data){
            const auto uuid = UBSvgSubsetAdaptor::sceneUuid(data.toByteArray());
            mToc->setUuid(index, uuid);
            mSceneHeaderLoader->resultProcessed();
        });

        QObject::connect(mSceneHeaderLoader, &UBBackgroundLoader::finished, mSceneHeaderLoader, [this](){
            mSceneHeaderLoader->deleteLater();
            mSceneHeaderLoader = nullptr;
            mToc->save();
        });

        mSceneHeaderLoader->load(paths, 300);

        // set the page ID for the pages
        for (int index = 0; index < pages.count(); ++index)
        {
            mToc->setPageId(index, paths.at(index).first);
        }
    }

    // now we have updated the document version
    mProxy->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);
    UBMetadataDcSubsetAdaptor::persist(mProxy);

    UBApplication::showMessage(UBDocumentController::tr("Scan completed"));

    QApplication::restoreOverrideCursor();
}

void UBDocument::copyPage(int fromIndex, UBDocument* to, int toIndex)
{
    const auto dependencies = pageRelativeDependencies(fromIndex);

    // copy scene
    to->assureHeaderLoaderFinished();
    const auto pageId = to->mToc->insert(toIndex);
    const auto uuid = UBPersistenceManager::persistenceManager()->copyDocumentScene(mProxy, mToc->pageId(fromIndex), to->proxy(), pageId, dependencies);

    // add thumbnail
    to->thumbnailScene()->insertThumbnail(toIndex);

    // update target TOC
    to->mToc->setUuid(toIndex, uuid);
    to->mToc->setAssets(toIndex, dependencies);
    to->mToc->save();
}

void UBDocument::deleteUnreferencedAssets()
{
    // Go through the TOC and create a set of all referenced media assets.
    QSet<QString> referencedMediaAssets;

    for (int index = 0; index < mToc->pageCount(); ++index)
    {
        if (!mToc->hasAssetsEntry(index))
        {
            // no cleanup with incomplete asset information
            return;
        }

        const auto assets = mToc->assets(index);

        for (const auto asset : assets)
        {
            referencedMediaAssets << asset;
        }
    }

    // Cleanup unused media assets
    UBPersistenceManager::persistenceManager()->cleanupMediaAssets(mProxy, referencedMediaAssets);
}

void UBDocument::assureHeaderLoaderFinished(bool visualFeedback)
{
    if (mSceneHeaderLoader)
    {
        if (visualFeedback)
        {
            UBApplication::setOverrideCursor(Qt::WaitCursor);
            UBApplication::showMessage(UBDocumentController::tr("Wait for the document scan to finish"), true);
        }

        while (mSceneHeaderLoader)
        {
            QApplication::processEvents(QEventLoop::AllEvents, 100);
        }

        if (visualFeedback)
        {
            UBApplication::showMessage(UBDocumentController::tr("Document scan finished"));
            UBApplication::restoreOverrideCursor();
        }
    }
}

std::shared_ptr<UBDocument> UBDocument::findDocument(std::shared_ptr<UBDocumentProxy> proxy)
{
    if (!proxy)
    {
        return nullptr;
    }

    for (int i = 0; i < sDocuments.size();)
    {
        const auto document = sDocuments.at(i).lock();

        if (!document)
        {
            // weak pointer expired, clean up list
            sDocuments.removeAt(i);
        }
        else if (document->mProxy == proxy)
        {
            // document found
            return document;
        }
        else
        {
            // check next list entry
            ++i;
        }
    }

    return nullptr;
}
