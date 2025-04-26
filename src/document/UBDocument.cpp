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

#include "adaptors/UBThumbnailAdaptor.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "document/UBDocumentController.h"
#include "document/UBToc.h"
#include "gui/UBMainWindow.h"
#include "gui/UBThumbnailScene.h"

QList<std::weak_ptr<UBDocument>> UBDocument::sDocuments;


UBDocument::UBDocument(std::shared_ptr<UBDocumentProxy> proxy)
    : mProxy(proxy)
    , mThumbnailScene(new UBThumbnailScene(this))
{
    toc();
    mThumbnailScene->createThumbnails();
}

UBDocument::~UBDocument()
{
    delete mThumbnailScene;
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

    for (int i = indexes.size() - 1; i >= 0; --i)
    {
        mThumbnailScene->deleteThumbnail(indexes.at(i), false);
        mToc->remove(indexes.at(i));
        emit UBPersistenceManager::persistenceManager()->documentSceneDeleted(this, indexes.at(i));
    }

    mThumbnailScene->renumberThumbnails(indexes.first());
    mThumbnailScene->arrangeThumbnails(indexes.first());
    mToc->save();
}

void UBDocument::duplicatePage(int index)
{
    copyPage(index, this, index + 1);
    emit UBPersistenceManager::persistenceManager()->documentSceneDuplicated(this, index + 1);
}

void UBDocument::movePage(int fromIndex, int toIndex)
{
    mThumbnailScene->moveThumbnail(fromIndex, toIndex);
    mToc->move(fromIndex, toIndex);
    mToc->save();
    emit UBPersistenceManager::persistenceManager()->documentSceneMoved(this, fromIndex, toIndex);
}

void UBDocument::copyPage(int fromIndex, std::shared_ptr<UBDocument> to, int toIndex)
{
    copyPage(fromIndex, to.get(), toIndex);
}

std::shared_ptr<UBGraphicsScene> UBDocument::createPage(int index, bool useUndoRedoStack)
{
    // create a new TOC entry for the page
    auto pageId = mToc->insert(index);
    auto scene = UBPersistenceManager::persistenceManager()->createDocumentSceneAt(mProxy, pageId, useUndoRedoStack);
    mToc->setUuid(index, scene->uuid());
    mToc->save();

    mThumbnailScene->insertThumbnail(index, scene);

    return scene;
}

void UBDocument::persistPage(std::shared_ptr<UBGraphicsScene> scene, int index, bool isAutomaticBackup,
                             bool forceImmediateSaving)
{
    const auto pageId = mToc->pageId(index);
    UBThumbnailAdaptor::persistScene(this, scene, index);
    UBPersistenceManager::persistenceManager()->persistDocumentScene(mProxy, scene, pageId, isAutomaticBackup,
                                                                     forceImmediateSaving);

    const auto assets = scene->relativeDependencies();

    if (assets != mToc->assets(index))
    {
        mToc->setAssets(index, assets);
        mToc->save();
    }

    mThumbnailScene->reloadThumbnail(index);
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

    return UBPersistenceManager::persistenceManager()->loadDocumentScene(mProxy, toc()->pageId(index));
}

std::shared_ptr<UBGraphicsScene> UBDocument::getScene(int index)
{
    return UBPersistenceManager::persistenceManager()->getDocumentScene(mProxy, toc()->pageId(index));
}

QList<QString> UBDocument::pageRelativeDependencies(int index)
{
    auto scene = loadScene(index, false);

    if (scene)
    {
        return scene->relativeDependencies();
    }

    return {};
}

UBThumbnailScene* UBDocument::thumbnailScene() const
{
    return mThumbnailScene;
}

UBToc* UBDocument::toc()
{
    if (!mToc)
    {
        mToc = new UBToc{mProxy->persistencePath()};

        if (!mToc->load() || QVersionNumber::fromString(mProxy->metaData(UBSettings::documentVersion).toString()) < QVersionNumber(4, 9, 0))
        {
            scan();
            mToc->save();
        }
    }

    return mToc;
}

int UBDocument::pageCount()
{
    return toc()->pageCount();
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
        sDocuments << document;
    }

    return document;
}

void UBDocument::scan()
{
    // Create a set of all scene UUIDs
    QSet<QUuid> tocSceneUuids;

    for (int i = 0; i < pageCount(); ++i)
    {
        tocSceneUuids.insert(mToc->uuid(i));
    }

    // Scan through the pages starting from the smallest number and counting up.
    QStringList pages = UBPersistenceManager::persistenceManager()->pageFiles(mProxy->persistencePath());

    if (pages.isEmpty())
    {
        qWarning() << "No pages found - " << mProxy->persistencePath();
        mProxy->setMetaData(UBSettings::documentName, UBPersistenceManager::tr("Broken - %1").arg(mProxy->documentFolderName()));
    }

    // Load the scenes and check the media assets.
    int pageProcessed = -1;

    for (const auto page : pages)
    {
        int pageId = page.mid(4, page.length() - 8).toInt();
        auto scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(mProxy, pageId);
        // Check the media asset UUIDs.
        // TODO Really?? Convert it to a SHA-1 based UUIDv5 if needed. Copy the asset file in this case.

        // Check whether the tocSceneUuids already contains this scene.
        // If yes, locate and update the TOC entry
        if (tocSceneUuids.contains(scene->uuid()))
        {
            const int index = mToc->findUuid(scene->uuid());

            if (index >= 0)
            {
                mToc->setAssets(index, scene->relativeDependencies());
                pageProcessed = index;
                tocSceneUuids.remove(scene->uuid());
            }
        }
        else
        {
            // add the file name, thumbnail file name and asset list to the TOC after the last processed TOC entry
            qDebug() << "scan: insert scene" << pageId << "at offset" << pageProcessed;
            ++pageProcessed;
            mToc->insert(pageProcessed);
            mToc->setUuid(pageProcessed, scene->uuid());
            mToc->setPageId(pageProcessed, pageId);
            mToc->setAssets(pageProcessed, scene->relativeDependencies());
        }
    }

    // When finished, remove all scenes still in tocSceneUuids from the TOC.
    // Those scenes might have been deleted using a previous version of OpenBoard.
    for (const auto uuid : tocSceneUuids)
    {
        qDebug() << "scan: removing scene from TOC" << uuid;
        mToc->remove(mToc->findUuid(uuid));
    }

    // TODO cleanup possibly later Go through the TOC and create a set of all referenced media assets.
    // Delete all unreferenced media asset files.
}

void UBDocument::copyPage(int fromIndex, UBDocument* to, int toIndex)
{
    const auto dependencies = pageRelativeDependencies(fromIndex);

    // copy scene
    const auto pageId = to->mToc->insert(toIndex);
    const auto uuid = UBPersistenceManager::persistenceManager()->copyDocumentScene(mProxy, mToc->pageId(fromIndex), to->proxy(), pageId, dependencies);

    // add thumbnail
    to->mThumbnailScene->insertThumbnail(toIndex);

    // update target TOC
    to->mToc->setUuid(toIndex, uuid);
    to->mToc->setAssets(toIndex, dependencies);
    to->mToc->save();
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
