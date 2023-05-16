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



#ifndef UBPERSISTENCEMANAGER_H_
#define UBPERSISTENCEMANAGER_H_

#include <QtCore>

#include "UBSceneCache.h"
#include "UBPersistenceWorker.h"

class QDomNode;
class QDomElement;
class UBDocument;
class UBDocumentProxy;
class UBGraphicsScene;
class UBDocumentTreeNode;
class UBDocumentTreeModel;

class UBPersistenceManager : public QObject
{
    Q_OBJECT

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
        static const QString fileDirectory; // Issue 1683 (Evolution) - AOU - 20131206

        static const QString myDocumentsName;
        static const QString modelsName;
        static const QString untitledDocumentsName;
        static const QString fFolders;
        static const QString tFolder;
        static const QString aName;

        static UBPersistenceManager* persistenceManager();
        static void destroy();

        virtual std::shared_ptr<UBDocumentProxy> createDocument(const QString& pGroupName = ""
                , const QString& pName = ""
                , bool withEmptyPage = true
                , QString directory =QString()
                , int pageCount = 0
                , bool promptDialogIfExists = false);

        virtual std::shared_ptr<UBDocumentProxy> createNewDocument(const QString& pGroupName = ""
                , const QString& pName = ""
                , bool withEmptyPage = true
                , QString directory =QString()
                , int pageCount = 0
                , bool promptDialogIfExists = false);

        virtual std::shared_ptr<UBDocumentProxy> createDocumentFromDir(const QString& pDocumentDirectory
                                                       , const QString& pGroupName = ""
                , const QString& pName = ""
                , bool withEmptyPage = false
                , bool addTitlePage = false
                , bool promptDialogIfExists = false);

        virtual std::shared_ptr<UBDocumentProxy> persistDocumentMetadata(std::shared_ptr<UBDocumentProxy> pDocumentProxy, bool forceImmediateSaving = false);

        virtual std::shared_ptr<UBDocumentProxy> duplicateDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy);

        virtual void deleteDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy);

        virtual void deleteDocumentScenes(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const QList<int>& indexes);

        virtual void duplicateDocumentScene(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int index);

        virtual void copyDocumentScene(std::shared_ptr<UBDocumentProxy>from, int fromIndex, std::shared_ptr<UBDocumentProxy>to, int toIndex);

        virtual void persistDocumentScene(std::shared_ptr<UBDocumentProxy> pDocumentProxy, std::shared_ptr<UBGraphicsScene> pScene, const int pSceneIndex, bool isAnAutomaticBackup = false, bool forceImmediateSaving = false);

        virtual std::shared_ptr<UBGraphicsScene> createDocumentSceneAt(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int index, bool useUndoRedoStack = true);

        virtual void insertDocumentSceneAt(std::shared_ptr<UBDocumentProxy> pDocumentProxy, std::shared_ptr<UBGraphicsScene> scene, int index, bool persist = true, bool deleting = false);

        virtual void moveSceneToIndex(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int source, int target);

        virtual std::shared_ptr<UBGraphicsScene> loadDocumentScene(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int sceneIndex, bool cacheNeighboringScenes = true);
        std::shared_ptr<UBGraphicsScene> getDocumentScene(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int sceneIndex) {return mSceneCache.value(pDocumentProxy, sceneIndex);}
        void reassignDocProxy(std::shared_ptr<UBDocumentProxy> newDocument, std::shared_ptr<UBDocumentProxy> oldDocument);

//        QList<QPointer<UBDocumentProxy> > documentProxies;
        UBDocumentTreeNode *mDocumentTreeStructure;
        UBDocumentTreeModel *mDocumentTreeStructureModel;

        virtual QStringList allShapes();
        virtual QStringList allGips();
        virtual QStringList allImages(const QDir& dir);
        virtual QStringList allVideos(const QDir& dir);
        virtual QStringList allWidgets(const QDir& dir);

        QString generateUniqueDocumentPath();
        QString generateUniqueDocumentPath(const QString& baseFolder);

        bool addDirectoryContentToDocument(const QString& documentRootFolder, std::shared_ptr<UBDocumentProxy> pDocument);

        void createDocumentProxiesStructure(bool interactive = false);
        void createDocumentProxiesStructure(const QFileInfoList &contentInfoList, bool interactive = false);
        std::shared_ptr<UBDocumentProxy> createDocumentProxyStructure(QFileInfo &contentInfo);
        QDialog::DialogCode processInteractiveReplacementDialog(std::shared_ptr<UBDocumentProxy> pProxy, bool multipleFiles = false);

        QStringList documentSubDirectories()
        {
            return mDocumentSubDirectories;
        }

        virtual bool isEmpty(std::shared_ptr<UBDocumentProxy> pDocumentProxy);
        virtual void purgeEmptyDocuments();

        bool addGraphicsWidgetToDocument(std::shared_ptr<UBDocumentProxy> mDocumentProxy, QString path, QUuid objectUuid, QString& destinationPath);
        bool addFileToDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy, QString path, const QString& subdir,  QUuid objectUuid, QString& destinationPath, QByteArray* data = NULL);

        bool mayHaveVideo(std::shared_ptr<UBDocumentProxy> pDocumentProxy);
        bool mayHaveAudio(std::shared_ptr<UBDocumentProxy> pDocumentProxy);
        bool mayHavePDF(std::shared_ptr<UBDocumentProxy> pDocumentProxy);
        bool mayHaveSVGImages(std::shared_ptr<UBDocumentProxy> pDocumentProxy);
        bool mayHaveWidget(std::shared_ptr<UBDocumentProxy> pDocumentProxy);

        QString adjustDocumentVirtualPath(const QString &str);

        void closing();
        bool isSceneInCached(std::shared_ptr<UBDocumentProxy>proxy, int index) const;

    signals:

        void proxyListChanged();

        void documentCreated(std::shared_ptr<UBDocumentProxy> pDocumentProxy);
        void documentMetadataChanged(std::shared_ptr<UBDocumentProxy> pDocumentProxy);

        void documentSceneCreated(std::shared_ptr<UBDocumentProxy> pDocumentProxy, int pIndex);

private:
        int sceneCount(const std::shared_ptr<UBDocumentProxy> pDocumentProxy);
        static QStringList getSceneFileNames(const QString& folder);
        void renamePage(std::shared_ptr<UBDocumentProxy> pDocumentProxy,
                        const int sourceIndex, const int targetIndex);
        void copyPage(std::shared_ptr<UBDocumentProxy> pDocumentProxy,
                      const int sourceIndex, const int targetIndex);
        void generatePathIfNeeded(std::shared_ptr<UBDocumentProxy> pDocumentProxy);
        void checkIfDocumentRepositoryExists();

        void saveFoldersTreeToXml(QXmlStreamWriter &writer, const QModelIndex &parentIndex);
        void loadFolderTreeFromXml(const QString &path, const QDomElement &element);

        void cleanupDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy) const;

        QString xmlFolderStructureFilename;

        UBSceneCache mSceneCache;
        QStringList mDocumentSubDirectories;
        QMutex mDeletedListMutex;
        bool mHasPurgedDocuments;
        QString mDocumentRepositoryPath;
        QString mFoldersXmlStorageName;
        QProgressDialog mProgress;
        QFutureWatcher<void> futureWatcher;
        UBPersistenceWorker* mWorker;

        QThread* mThread;
        bool mIsWorkerFinished;

        bool mIsApplicationClosing;

        bool mReplaceDialogReturnedReplaceAll;
        bool mReplaceDialogReturnedCancel;

    private slots:
        void documentRepositoryChanged(const QString& path);
        void errorString(QString error);
        void onSceneLoaded(QByteArray,std::shared_ptr<UBDocumentProxy>,int);
        void onWorkerFinished();
        void onScenePersisted(std::shared_ptr<UBGraphicsScene> scene);

};


#endif /* UBPERSISTENCEMANAGER_H_ */
