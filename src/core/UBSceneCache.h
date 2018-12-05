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




#ifndef UBSCENECACHE_H
#define UBSCENECACHE_H

#include <QtCore>

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

        UBSceneCacheID(UBDocumentProxy* pDocumentProxy, int pPageIndex)
        {
            documentProxy = pDocumentProxy;
            pageIndex = pPageIndex;
        }

        UBDocumentProxy* documentProxy;
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

class UBSceneCache : public QHash<UBSceneCacheID, UBGraphicsScene*>
{
    public:

        UBSceneCache();
        virtual ~UBSceneCache();

        UBGraphicsScene* createScene(UBDocumentProxy* proxy, int pageIndex, bool useUndoRedoStack);

        void insert (UBDocumentProxy* proxy, int pageIndex, UBGraphicsScene* scene );

        bool contains(UBDocumentProxy* proxy, int pageIndex) const;

        UBGraphicsScene* value(UBDocumentProxy* proxy, int pageIndex);

        UBGraphicsScene* value(const UBSceneCacheID& key) const
        {
            return QHash<UBSceneCacheID, UBGraphicsScene*>::value(key);
        }

        void removeScene(UBDocumentProxy* proxy, int pageIndex);

        void removeAllScenes(UBDocumentProxy* proxy);

        void moveScene(UBDocumentProxy* proxy, int sourceIndex, int targetIndex);

        void reassignDocProxy(UBDocumentProxy *newDocument, UBDocumentProxy *oldDocument);

        void shiftUpScenes(UBDocumentProxy* proxy, int startIncIndex, int endIncIndex);


    private:

        void internalMoveScene(UBDocumentProxy* proxy, int sourceIndex, int targetIndex);

        void dumpCacheContent();

        int mCachedSceneCount;

        QQueue<UBSceneCacheID> mCachedKeyFIFO;

        QHash<UBSceneCacheID, UBGraphicsScene::SceneViewState> mViewStates;

};



#endif // UBSCENECACHE_H
