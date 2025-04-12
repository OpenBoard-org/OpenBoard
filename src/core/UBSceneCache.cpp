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



#include "UBSceneCache.h"

#include "domain/UBGraphicsScene.h"

#include <adaptors/UBSvgSubsetAdaptor.h>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "document/UBDocumentProxy.h"

#include "core/memcheck.h"

UBSceneCache::UBSceneCache()
{
    // NOOP
}


UBSceneCache::~UBSceneCache()
{
    // NOOP
}


std::shared_ptr<UBGraphicsScene> UBSceneCache::createScene(std::shared_ptr<UBDocumentProxy> proxy, int pageId, bool useUndoRedoStack)
{
    auto newScene = std::make_shared<UBGraphicsScene>(proxy, useUndoRedoStack);

    insert(proxy, pageId, newScene);

    return newScene;
}

std::shared_ptr<UBGraphicsScene> UBSceneCache::prepareLoading(std::shared_ptr<UBDocumentProxy> proxy, int pageId)
{
    if (mSceneCache.contains({proxy, pageId}))
    {
        return value(proxy, pageId);
    }

    // no entry in cache; create a cache entry to load scene
    qDebug() << "Preparing to load scene" << pageId;
    auto cacheEntry = std::make_shared<SceneCacheEntry>(proxy, pageId);

    insertEntry({proxy, pageId}, cacheEntry);
    cacheEntry->startLoading();
    return nullptr;
}


void UBSceneCache::insert (std::shared_ptr<UBDocumentProxy> proxy, int pageId, std::shared_ptr<UBGraphicsScene> scene)
{
    // remove all entries pointing to this scene
    const auto keylist = mSceneCache.keys();

    for (const auto& key : keylist)
    {
        auto entry = mSceneCache.value(key);

        if (entry->isSceneAvailable() && entry->scene() == scene)
        {
            mSceneCache.remove(key);
            mCachedKeyFIFO.removeAll(key);
        }
    }

    UBSceneCacheID key{proxy, pageId};
    insertEntry(key, std::make_shared<SceneCacheEntry>(scene));

    // restore view state
    if (mViewStates.contains(key))
    {
        scene->setViewState(mViewStates.value(key));
    }
}


bool UBSceneCache::contains(std::shared_ptr<UBDocumentProxy> proxy, int pageId) const
{
    return mSceneCache.contains({proxy, pageId});
}


std::shared_ptr<UBGraphicsScene> UBSceneCache::value(std::shared_ptr<UBDocumentProxy> proxy, int pageId)
{
    UBSceneCacheID key{proxy, pageId};

    if (mSceneCache.contains(key))
    {
        auto entry = mSceneCache.value(key);

        mCachedKeyFIFO.removeAll(key);
        mCachedKeyFIFO.enqueue(key);

        return entry->scene();
    }
    else
    {
        return nullptr;
    }
}


void UBSceneCache::removeScene(std::shared_ptr<UBDocumentProxy> proxy, int pageId)
{
    UBSceneCacheID key{proxy, pageId};

    if (!mSceneCache.contains(key))
    {
        return;
    }

    auto entry = mSceneCache.value(key);

    if (!entry->isSceneAvailable() || !entry->scene()->isActive())
    {
        int count = mSceneCache.remove(key);
        mCachedKeyFIFO.removeAll(key);

        if (entry->isSceneAvailable())
        {
            mViewStates.insert(key, entry->scene()->viewState());
        }
    }
}


void UBSceneCache::removeAllScenes(std::shared_ptr<UBDocumentProxy> proxy)
{
    for(int i = 0 ; i < proxy->pageCount(); i++)
    {
        removeScene(proxy, i);
    }
}


void UBSceneCache::moveScene(std::shared_ptr<UBDocumentProxy> proxy, int sourceIndex, int targetIndex)
{
    UBSceneCacheID keySource(proxy, sourceIndex);

    std::shared_ptr<SceneCacheEntry> entry;
    bool hasEntry = mSceneCache.contains(keySource);

    if (hasEntry)
    {
        entry = mSceneCache.value(keySource);
        mCachedKeyFIFO.removeAll(keySource);
    }

    if (sourceIndex < targetIndex)
    {
        for (int i = sourceIndex + 1; i <= targetIndex; i++)
        {
            internalMoveScene(proxy, i, i - 1);
        }
    }
    else
    {
        for (int i = sourceIndex - 1; i >= targetIndex; i--)
        {
            internalMoveScene(proxy, i, i + 1);
        }
    }

    UBSceneCacheID keyTarget(proxy, targetIndex);

    if (hasEntry)
    {
        insertEntry(keyTarget, entry);
        mCachedKeyFIFO.enqueue(keyTarget);
    }
    else if (mSceneCache.contains(keyTarget))
    {
        entry = mSceneCache.take(keyTarget);
        mCachedKeyFIFO.removeAll(keyTarget);
    }

}

void UBSceneCache::reassignDocProxy(std::shared_ptr<UBDocumentProxy> newDocument, std::shared_ptr<UBDocumentProxy> oldDocument)
{
    if (!newDocument || !oldDocument) {
        return;
    }
    if (newDocument->pageCount() != oldDocument->pageCount()) {
        return;
    }
    if (!QFileInfo(oldDocument->persistencePath()).exists()) {
        return;
    }
    for (int i = 0; i < oldDocument->pageCount(); i++) {

        UBSceneCacheID sourceKey(oldDocument, i);
        auto entry = mSceneCache.value(sourceKey);

        if (entry->isSceneAvailable())
        {
            entry->scene()->setDocument(newDocument);
        }

        mCachedKeyFIFO.removeAll(sourceKey);
        int count = mSceneCache.remove(sourceKey);

        insertEntry({newDocument, i}, entry);
    }
}


void UBSceneCache::shiftUpScenes(std::shared_ptr<UBDocumentProxy> proxy, int startIncIndex, int endIncIndex)
{
    for(int i = endIncIndex; i >= startIncIndex; i--)
    {
        UBApplication::showMessage(QObject::tr("Moving cached scenes (%1/%2)").arg(i).arg(endIncIndex));
        internalMoveScene(proxy, i, i + 1);
    }
}


void UBSceneCache::internalMoveScene(std::shared_ptr<UBDocumentProxy> proxy, int sourceIndex, int targetIndex)
{
    UBSceneCacheID sourceKey(proxy, sourceIndex);

    if (mSceneCache.contains(sourceKey))
    {
        auto scene = mSceneCache.take(sourceKey);
        mCachedKeyFIFO.removeAll(sourceKey);

        UBSceneCacheID targetKey(proxy, targetIndex);
        mSceneCache.insert(targetKey, scene);
        mCachedKeyFIFO.enqueue(targetKey);

    }
    else
    {
        UBSceneCacheID targetKey(proxy, targetIndex);
        if (mSceneCache.contains(targetKey))
        {
            mSceneCache.take(targetKey);

            mCachedKeyFIFO.removeAll(targetKey);
        }
    }
}

void UBSceneCache::insertEntry(UBSceneCacheID key, std::shared_ptr<SceneCacheEntry> entry)
{
    mSceneCache.insert(key, entry);
    mCachedKeyFIFO.removeAll(key);
    mCachedKeyFIFO.enqueue(key);

    // remove LRU entries if cache size grows beyond limit
    auto entries = mCachedKeyFIFO.size();

    while (entries-- > UBSettings::settings()->pageCacheSize->get().toInt())
    {
        qDebug() << "cache full, size" << entries;
        const auto key = mCachedKeyFIFO.dequeue();
        auto entry = mSceneCache.value(key);

        // remove if still loading or inactive
        if ((entry->isSceneAvailable() && !entry->scene()->isActive())
                || !entry->isSceneAvailable())
        {
            qDebug() << "removing page" << key.mPageId << "of" << key.mDocumentProxy->documentFolderName();
            removeScene(key.mDocumentProxy, key.mPageId);
            entry = nullptr;
            qDebug() << "removed page" << key.mPageId << "of" << key.mDocumentProxy->documentFolderName();
            break;
        }
    }
}

UBSceneCache::SceneCacheEntry::SceneCacheEntry(std::shared_ptr<UBDocumentProxy> proxy, int pageId)
{
    mContext = UBSvgSubsetAdaptor::prepareLoadingScene(proxy, pageId);
}

UBSceneCache::SceneCacheEntry::SceneCacheEntry(std::shared_ptr<UBGraphicsScene> scene)
{
    mScene = scene;
}

UBSceneCache::SceneCacheEntry::~SceneCacheEntry()
{
    if (mTimer)
    {
        delete mTimer;
    }
}

void UBSceneCache::SceneCacheEntry::startLoading()
{
    mTimer = new QTimer;
    QObject::connect(mTimer, &QTimer::timeout, mTimer, [this](){
        if (UBApplication::isClosing)
        {
            mTimer->stop();
            delete mTimer;
            return;
        }

        if (mContext)
        {
            mContext->step();

            if (mContext->isFinished())
            {
                mScene = mContext->scene();
                mContext = nullptr;
                mTimer->stop();
                delete mTimer;
                mTimer = nullptr;
            }
        }
    });

    mTimer->start();
}

bool UBSceneCache::SceneCacheEntry::isSceneAvailable() const
{
    return mScene != nullptr;
}

std::shared_ptr<UBGraphicsScene> UBSceneCache::SceneCacheEntry::scene()
{
    if (mContext && !mScene)
    {
        // finish loading
        if (mTimer)
        {
            mTimer->stop();
            delete mTimer;
            mTimer = nullptr;
        }

        while (!mContext->isFinished())
        {
            mContext->step();
        }

        mScene = mContext->scene();
        mContext = nullptr;
    }

    return mScene;
}
