/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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

#include "core/UBPersistenceManager.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "document/UBDocumentProxy.h"

#include "core/memcheck.h"

UBSceneCache::UBSceneCache()
    : mCachedSceneCount(0)
{
    // NOOP
}


UBSceneCache::~UBSceneCache()
{
    // NOOP
}


UBGraphicsScene* UBSceneCache::createScene(UBDocumentProxy* proxy, int pageIndex, bool useUndoRedoStack)
{
    UBGraphicsScene* newScene = new UBGraphicsScene(proxy, useUndoRedoStack);
    insert(proxy, pageIndex, newScene);

    return newScene;

}


void UBSceneCache::insert (UBDocumentProxy* proxy, int pageIndex, UBGraphicsScene* scene)
{
    QList<UBSceneCacheID> existingKeys = QHash<UBSceneCacheID, UBGraphicsScene*>::keys(scene);

    foreach(UBSceneCacheID key, existingKeys)
    {
        mCachedSceneCount -= QHash<UBSceneCacheID, UBGraphicsScene*>::remove(key);
    }

    UBSceneCacheID key(proxy, pageIndex);

    if (QHash<UBSceneCacheID, UBGraphicsScene*>::contains(key))
    {
        QHash<UBSceneCacheID, UBGraphicsScene*>::insert(key, scene);
        mCachedKeyFIFO.enqueue(key);
    }
    else
    {
        QHash<UBSceneCacheID, UBGraphicsScene*>::insert(key, scene);
        mCachedKeyFIFO.enqueue(key);

        mCachedSceneCount++;
    }

    if (mViewStates.contains(key))
    {
        scene->setViewState(mViewStates.value(key));
    }
}


bool UBSceneCache::contains(UBDocumentProxy* proxy, int pageIndex) const
{
    UBSceneCacheID key(proxy, pageIndex);
    return QHash<UBSceneCacheID, UBGraphicsScene*>::contains(key);
}


UBGraphicsScene* UBSceneCache::value(UBDocumentProxy* proxy, int pageIndex)
{
    UBSceneCacheID key(proxy, pageIndex);

    if (QHash<UBSceneCacheID, UBGraphicsScene*>::contains(key))
    {
        UBGraphicsScene* scene = QHash<UBSceneCacheID, UBGraphicsScene*>::value(key);

        mCachedKeyFIFO.removeAll(key);
        mCachedKeyFIFO.enqueue(key);

        return scene;
    }
    else
    {
        return 0;
    }
}


void UBSceneCache::removeScene(UBDocumentProxy* proxy, int pageIndex)
{
    UBGraphicsScene* scene = value(proxy, pageIndex);
    if (scene && !scene->isActive())
    {
        UBSceneCacheID key(proxy, pageIndex);
        int count = QHash<UBSceneCacheID, UBGraphicsScene*>::remove(key);
        mCachedKeyFIFO.removeAll(key);

        mViewStates.insert(key, scene->viewState());

        scene->deleteLater();

        mCachedSceneCount -= count;
    }
}


void UBSceneCache::removeAllScenes(UBDocumentProxy* proxy)
{
    for(int i = 0 ; i < proxy->pageCount(); i++)
    {
        removeScene(proxy, i);
    }
}


void UBSceneCache::moveScene(UBDocumentProxy* proxy, int sourceIndex, int targetIndex)
{
    UBSceneCacheID keySource(proxy, sourceIndex);

    UBGraphicsScene *scene = 0;

    if (QHash<UBSceneCacheID, UBGraphicsScene*>::contains(keySource))
    {
        scene = QHash<UBSceneCacheID, UBGraphicsScene*>::value(keySource);
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

    if (scene)
    {
        insert(proxy, targetIndex, scene);
        mCachedKeyFIFO.enqueue(keyTarget);
    }
    else if (QHash<UBSceneCacheID, UBGraphicsScene*>::contains(keyTarget))
    {
        scene = QHash<UBSceneCacheID, UBGraphicsScene*>::take(keyTarget);
        mCachedKeyFIFO.removeAll(keyTarget);
    }

}

void UBSceneCache::reassignDocProxy(UBDocumentProxy *newDocument, UBDocumentProxy *oldDocument)
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
        UBGraphicsScene *currentScene = value(sourceKey);
        if (currentScene) {
            currentScene->setDocument(newDocument);
        }
        mCachedKeyFIFO.removeAll(sourceKey);
        int count = QHash<UBSceneCacheID, UBGraphicsScene*>::remove(sourceKey);
        mCachedSceneCount -= count;

        insert(newDocument, i, currentScene);
    }

}


void UBSceneCache::shiftUpScenes(UBDocumentProxy* proxy, int startIncIndex, int endIncIndex)
{
    for(int i = endIncIndex; i >= startIncIndex; i--)
    {
        internalMoveScene(proxy, i, i + 1);
    }
}


void UBSceneCache::internalMoveScene(UBDocumentProxy* proxy, int sourceIndex, int targetIndex)
{
    UBSceneCacheID sourceKey(proxy, sourceIndex);

    if (QHash<UBSceneCacheID, UBGraphicsScene*>::contains(sourceKey))
    {
        UBGraphicsScene* scene = QHash<UBSceneCacheID, UBGraphicsScene*>::take(sourceKey);
        mCachedKeyFIFO.removeAll(sourceKey);

        UBSceneCacheID targetKey(proxy, targetIndex);
        QHash<UBSceneCacheID, UBGraphicsScene*>::insert(targetKey, scene);
        mCachedKeyFIFO.enqueue(targetKey);

    }
    else
    {
        UBSceneCacheID targetKey(proxy, targetIndex);
        if (QHash<UBSceneCacheID, UBGraphicsScene*>::contains(targetKey))
        {
            /*UBGraphicsScene* scene = */QHash<UBSceneCacheID, UBGraphicsScene*>::take(targetKey);

            mCachedKeyFIFO.removeAll(targetKey);
        }
    }
}

void UBSceneCache::dumpCacheContent()
{
    foreach(UBSceneCacheID key, keys())
    {
        UBGraphicsScene *scene = 0;

        if (QHash<UBSceneCacheID, UBGraphicsScene*>::contains(key))
            scene = QHash<UBSceneCacheID, UBGraphicsScene*>::value(key);

        int index = key.pageIndex;

        qDebug() << "UBSceneCache::dumpCacheContent:" << index << " : " << scene;
    }
}
