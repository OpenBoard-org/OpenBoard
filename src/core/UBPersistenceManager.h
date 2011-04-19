/*
 * UBPersistenceManager.h
 *
 *  Created on: Nov 6, 2008
 *      Author: luc
 */

#ifndef UBPERSISTENCEMANAGER_H_
#define UBPERSISTENCEMANAGER_H_

#include <QtCore>

#include "UBSceneCache.h"

class UBDocument;
class UBDocumentProxy;
class UBGraphicsScene;

class UBPersistenceManager : public QObject
{
    Q_OBJECT;

    private:
        UBPersistenceManager(QObject *pParent = 0);
        static UBPersistenceManager* sSingleton;

    public:

        virtual ~UBPersistenceManager();

        static const QString imageDirectory;
        static const QString objectDirectory;
        static const QString videoDirectory;
        static const QString audioDirectory;
        static const QString widgetDirectory;

        static UBPersistenceManager* persistenceManager();

        virtual UBDocumentProxy* createDocument(const QString& pGroupName = "", const QString& pName = "");
        virtual UBDocumentProxy* createDocumentFromDir(const QString& pDocumentDirectory);

        virtual UBDocumentProxy* persistDocumentMetadata(UBDocumentProxy* pDocumentProxy);

        virtual UBDocumentProxy* duplicateDocument(UBDocumentProxy* pDocumentProxy);

        virtual void deleteDocument(UBDocumentProxy* pDocumentProxy);

        virtual void deleteDocumentScenes(UBDocumentProxy* pDocumentProxy, const QList<int>& indexes);

        virtual void duplicateDocumentScene(UBDocumentProxy* pDocumentProxy, int index);

        virtual void persistDocumentScene(UBDocumentProxy* pDocumentProxy,
                UBGraphicsScene* pScene, const int pSceneIndex);

        virtual UBGraphicsScene* createDocumentSceneAt(UBDocumentProxy* pDocumentProxy, int index);

        virtual void insertDocumentSceneAt(UBDocumentProxy* pDocumentProxy, UBGraphicsScene* scene, int index);

        virtual void moveSceneToIndex(UBDocumentProxy* pDocumentProxy, int source, int target);

        virtual UBGraphicsScene* loadDocumentScene(UBDocumentProxy* pDocumentProxy, int sceneIndex);

        QList<QPointer<UBDocumentProxy> > documentProxies;

        virtual QStringList allShapes();
        virtual QStringList allGips();
        virtual QStringList allSounds();
        virtual QStringList allImages(const QDir& dir);
        virtual QStringList allVideos(const QDir& dir);
        virtual QStringList allWidgets(const QDir& dir);

        virtual QString generateUniqueDocumentPath();

        virtual void addDirectoryContentToDocument(const QString& documentRootFolder, UBDocumentProxy* pDocument);

        virtual void upgradeDocumentIfNeeded(UBDocumentProxy* pDocumentProxy);

        virtual void upgradeAllDocumentsIfNeeded();

        virtual UBDocumentProxy* documentByUuid(const QUuid& pUuid);

        QStringList documentSubDirectories()
        {
            return mDocumentSubDirectories;
        }

        virtual bool isEmpty(UBDocumentProxy* pDocumentProxy);
        virtual void purgeEmptyDocuments();

        virtual QString addVideoFileToDocument(UBDocumentProxy* pDocumentProxy, QString path, QUuid objectUuid);
        virtual QString addVideoFileToDocument(UBDocumentProxy* pDocumentProxy, QUrl sourceUrl, QByteArray pPayload, QUuid objectUuid);
        virtual QString addAudioFileToDocument(UBDocumentProxy* pDocumentProxy, QString path, QUuid objectUuid);
        virtual QString addAudioFileToDocument(UBDocumentProxy* pDocumentProxy, QUrl sourceUrl, QByteArray pPayload, QUuid objectUuid);
        virtual QString addPdfFileToDocument(UBDocumentProxy* pDocumentProxy, QString path, QUuid objectUuid);

        bool mayHaveVideo(UBDocumentProxy* pDocumentProxy);
        bool mayHaveAudio(UBDocumentProxy* pDocumentProxy);
        bool mayHavePDF(UBDocumentProxy* pDocumentProxy);
        bool mayHaveSVGImages(UBDocumentProxy* pDocumentProxy);
        bool mayHaveWidget(UBDocumentProxy* pDocumentProxy);

    signals:

        void proxyListChanged();

        void documentCreated(UBDocumentProxy* pDocumentProxy);
        void documentMetadataChanged(UBDocumentProxy* pDocumentProxy);
        void documentCommitted(UBDocumentProxy* pDocumentProxy);
        void documentWillBeDeleted(UBDocumentProxy* pDocumentProxy);

        void documentSceneCreated(UBDocumentProxy* pDocumentProxy, int pIndex);
        void documentSceneMoved(UBDocumentProxy* pDocumentProxy, int pIndex);
        void documentSceneWillBeDeleted(UBDocumentProxy* pDocumentProxy, int pIndex);
        void documentSceneDeleted(UBDocumentProxy* pDocumentProxy, int pDeletedIndex);

    private:

        int sceneCount(const UBDocumentProxy* pDocumentProxy);

        int sceneCountInDir(const QString& pPath);

        QList<QPointer<UBDocumentProxy> > allDocumentProxies();

        void renamePage(UBDocumentProxy* pDocumentProxy,
                const int sourceIndex, const int targetIndex);

        void copyPage(UBDocumentProxy* pDocumentProxy,
                const int sourceIndex, const int targetIndex);

        void generatePathIfNeeded(UBDocumentProxy* pDocumentProxy);

        void checkIfDocumentRepositoryExists();

        UBSceneCache mSceneCache;

        QStringList mDocumentSubDirectories;

        QMutex mDeletedListMutex;

        bool mHasPurgedDocuments;

        QList<UBDocumentProxy*> mDocumentCreatedDuringSession;

        QString mDocumentRepositoryPath;

    private slots:
        void documentRepositoryChanged(const QString& path);

};


#endif /* UBPERSISTENCEMANAGER_H_ */
