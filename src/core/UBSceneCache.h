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




#ifndef UBSCENECACHE_H
#define UBSCENECACHE_H

#include <QtCore>

#include <variant>

#include "adaptors/UBSvgSubsetAdaptor.h"
#include "domain/UBGraphicsScene.h"

class UBDocumentProxy;
class UBGraphicsScene;
class UBGraphicsScene;

class UBSceneCacheID
{
public:

    UBSceneCacheID()
        : documentProxy(0)
        , pageIndex(-1)
    {
        // NOOP
    }

    UBSceneCacheID(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int pPageIndex)
    {
        documentProxy = pDocumentProxy;
        pageIndex = pPageIndex;
    }

    std::shared_ptr<UBDocumentProxy> documentProxy;
    int pageIndex;
};

inline bool operator==(const UBSceneCacheID &id1, const UBSceneCacheID &id2)
{
    return id1.documentProxy == id2.documentProxy
        && id1.pageIndex == id2.pageIndex;
}

inline uint qHash(const UBSceneCacheID &id)
{
    return qHash(id.pageIndex);
}


class UBSceneCache
{
public:
    UBSceneCache();
    virtual ~UBSceneCache();

    std::shared_ptr<UBGraphicsScene> createScene(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex, bool useUndoRedoStack);

    std::shared_ptr<UBGraphicsScene> prepareLoading(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex);

    void insert (std::shared_ptr<UBDocumentProxy> proxy, int pageIndex, std::shared_ptr<UBGraphicsScene> scene);

    bool contains(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex) const;

    std::shared_ptr<UBGraphicsScene> value(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex);

    void removeScene(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex);

    void removeAllScenes(std::shared_ptr<UBDocumentProxy> proxy);

    void moveScene(std::shared_ptr<UBDocumentProxy> proxy, int sourceIndex, int targetIndex);

    void reassignDocProxy(std::shared_ptr<UBDocumentProxy> newDocument, std::shared_ptr<UBDocumentProxy> oldDocument);

    void shiftUpScenes(std::shared_ptr<UBDocumentProxy> proxy, int startIncIndex, int endIncIndex);


private:
    class SceneCacheEntry
    {
    public:
        SceneCacheEntry(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex);
        SceneCacheEntry(std::shared_ptr<UBGraphicsScene> scene);
        ~SceneCacheEntry();
        void startLoading();
        bool isSceneAvailable() const;
        std::shared_ptr<UBGraphicsScene> scene();

    private:
        std::shared_ptr<UBSvgSubsetAdaptor::UBSvgReaderContext> mContext = nullptr;
        std::shared_ptr<UBGraphicsScene> mScene = nullptr;
        QTimer* mTimer = nullptr;
    };

//    typedef QFuture<std::shared_ptr<UBGraphicsScene>> FutureScene;
//    typedef std::variant<std::shared_ptr<UBGraphicsScene>, FutureScene> CacheEntry;

    void internalMoveScene(std::shared_ptr<UBDocumentProxy> proxy, int sourceIndex, int targetIndex);

    void insertEntry(UBSceneCacheID key, std::shared_ptr<SceneCacheEntry> entry);

    QHash<UBSceneCacheID, std::shared_ptr<SceneCacheEntry>> mSceneCache;

    QQueue<UBSceneCacheID> mCachedKeyFIFO;

    QHash<UBSceneCacheID, UBGraphicsScene::SceneViewState> mViewStates;
};



#endif // UBSCENECACHE_H
