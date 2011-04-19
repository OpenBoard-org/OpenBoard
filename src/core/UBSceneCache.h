#ifndef UBSCENECACHE_H
#define UBSCENECACHE_H

#include <QtCore>

#include "domain/UBGraphicsScene.h"

class UBDocumentProxy;
class UBGraphicsScene;
class UBGraphicsScene::SceneViewState;

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

        UBGraphicsScene* createScene(UBDocumentProxy* proxy, int pageIndex);

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

        void shiftUpScenes(UBDocumentProxy* proxy, int startIncIndex, int endIncIndex);


    private:

        void internalMoveScene(UBDocumentProxy* proxy, int sourceIndex, int targetIndex);

        void dumpCacheContent();

        void compactCache();

        int mCachedSceneCount;

        QQueue<UBSceneCacheID> mCachedKeyFIFO;

        QHash<UBSceneCacheID, UBGraphicsScene::SceneViewState> mViewStates;

};



#endif // UBSCENECACHE_H
