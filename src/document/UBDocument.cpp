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
#include "core/UBPersistenceManager.h"
#include "gui/UBThumbnailScene.h"

QList<std::weak_ptr<UBDocument>> UBDocument::sDocuments;


UBDocument::UBDocument(std::shared_ptr<UBDocumentProxy> proxy)
    : mProxy(proxy)
    , mThumbnailScene(new UBThumbnailScene(this))
{
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

    std::sort(indexes.begin(), indexes.end());
    UBPersistenceManager::persistenceManager()->deleteDocumentScenes(mProxy, indexes);

    for (int i = indexes.size() - 1; i >= 0; --i)
    {
        mThumbnailScene->deleteThumbnail(indexes.at(i), false);
        emit UBPersistenceManager::persistenceManager()->documentSceneDeleted(mProxy, i);
    }

    mThumbnailScene->renumberThumbnails(indexes.first());
    mThumbnailScene->arrangeThumbnails(indexes.first());

    QDateTime now = QDateTime::currentDateTime();
    mProxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(now));
}

void UBDocument::duplicatePage(int index)
{
    UBPersistenceManager::persistenceManager()->duplicateDocumentScene(mProxy, index);

    mThumbnailScene->insertThumbnail(index + 1);

    QDateTime now = QDateTime::currentDateTime();
    mProxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(now));

    emit UBPersistenceManager::persistenceManager()->documentSceneSelected(mProxy, index + 1);
}

void UBDocument::movePage(int fromIndex, int toIndex)
{
    UBPersistenceManager::persistenceManager()->moveSceneToIndex(mProxy, fromIndex, toIndex);
    mThumbnailScene->moveThumbnail(fromIndex, toIndex);
    emit UBPersistenceManager::persistenceManager()->documentSceneSelected(mProxy, toIndex);
}

void UBDocument::copyPage(int fromIndex, std::shared_ptr<UBDocumentProxy> to, int toIndex)
{
    UBPersistenceManager::persistenceManager()->copyDocumentScene(mProxy, fromIndex, to, toIndex);

    const auto toDocument = findDocument(to);

    if (toDocument)
    {
        toDocument->mThumbnailScene->insertThumbnail(toIndex);
    }
}

void UBDocument::insertPage(std::shared_ptr<UBGraphicsScene> scene, int index, bool persist, bool deleting)
{
    UBPersistenceManager::persistenceManager()->insertDocumentSceneAt(mProxy, scene, index, persist, deleting);

    mThumbnailScene->insertThumbnail(index);
}

std::shared_ptr<UBGraphicsScene> UBDocument::createPage(int index, bool useUndoRedoStack)
{
    auto scene = UBPersistenceManager::persistenceManager()->createDocumentSceneAt(mProxy, index, useUndoRedoStack);

    mThumbnailScene->insertThumbnail(index, scene);

    return scene;
}

void UBDocument::persistPage(std::shared_ptr<UBGraphicsScene> scene, const int index, bool isAutomaticBackup,
                             bool forceImmediateSaving)
{
    UBPersistenceManager::persistenceManager()->persistDocumentScene(mProxy, scene, index, isAutomaticBackup,
                                                                     forceImmediateSaving);
    mThumbnailScene->reloadThumbnail(index);
}

UBThumbnailScene* UBDocument::thumbnailScene() const
{
    return mThumbnailScene;
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
