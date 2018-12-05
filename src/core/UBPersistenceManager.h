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



#ifndef UBPERSISTENCEMANAGER_H_
#define UBPERSISTENCEMANAGER_H_

#include <QtCore>

#include "UBSceneCache.h"

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

        virtual UBDocumentProxy* createDocument(const QString& pGroupName = ""
                , const QString& pName = ""
                , bool withEmptyPage = true
                , QString directory =QString()
                , int pageCount = 0
                , bool promptDialogIfExists = false);

        virtual UBDocumentProxy *createNewDocument(const QString& pGroupName = ""
                , const QString& pName = ""
                , bool withEmptyPage = true
                , QString directory =QString()
                , int pageCount = 0
                , bool promptDialogIfExists = false);

        virtual UBDocumentProxy* createDocumentFromDir(const QString& pDocumentDirectory
                                                       , const QString& pGroupName = ""
                , const QString& pName = ""
                , bool withEmptyPage = false
                , bool addTitlePage = false
                , bool promptDialogIfExists = false);

        virtual UBDocumentProxy* persistDocumentMetadata(UBDocumentProxy* pDocumentProxy);

        virtual UBDocumentProxy* duplicateDocument(UBDocumentProxy* pDocumentProxy);

        virtual void deleteDocument(UBDocumentProxy* pDocumentProxy);

        virtual void deleteDocumentScenes(UBDocumentProxy* pDocumentProxy, const QList<int>& indexes);

        virtual void duplicateDocumentScene(UBDocumentProxy* pDocumentProxy, int index);

        virtual void copyDocumentScene(UBDocumentProxy *from, int fromIndex, UBDocumentProxy *to, int toIndex);

        virtual void persistDocumentScene(UBDocumentProxy* pDocumentProxy,
                UBGraphicsScene* pScene, const int pSceneIndex);

        virtual UBGraphicsScene* createDocumentSceneAt(UBDocumentProxy* pDocumentProxy, int index, bool useUndoRedoStack = true);

        virtual void insertDocumentSceneAt(UBDocumentProxy* pDocumentProxy, UBGraphicsScene* scene, int index, bool persist = true);

        virtual void moveSceneToIndex(UBDocumentProxy* pDocumentProxy, int source, int target);

        virtual UBGraphicsScene* loadDocumentScene(UBDocumentProxy* pDocumentProxy, int sceneIndex);
        UBGraphicsScene *getDocumentScene(UBDocumentProxy* pDocumentProxy, int sceneIndex) {return mSceneCache.value(pDocumentProxy, sceneIndex);}
        void reassignDocProxy(UBDocumentProxy *newDocument, UBDocumentProxy *oldDocument);

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

        bool addDirectoryContentToDocument(const QString& documentRootFolder, UBDocumentProxy* pDocument);

        void createDocumentProxiesStructure(bool interactive = false);
        void createDocumentProxiesStructure(const QFileInfoList &contentInfo, bool interactive = false);
        QDialog::DialogCode processInteractiveReplacementDialog(UBDocumentProxy *pProxy);

        QStringList documentSubDirectories()
        {
            return mDocumentSubDirectories;
        }

        virtual bool isEmpty(UBDocumentProxy* pDocumentProxy);
        virtual void purgeEmptyDocuments();

        bool addGraphicsWidgetToDocument(UBDocumentProxy *mDocumentProxy, QString path, QUuid objectUuid, QString& destinationPath);
        bool addFileToDocument(UBDocumentProxy* pDocumentProxy, QString path, const QString& subdir,  QUuid objectUuid, QString& destinationPath, QByteArray* data = NULL);

        bool mayHaveVideo(UBDocumentProxy* pDocumentProxy);
        bool mayHaveAudio(UBDocumentProxy* pDocumentProxy);
        bool mayHavePDF(UBDocumentProxy* pDocumentProxy);
        bool mayHaveSVGImages(UBDocumentProxy* pDocumentProxy);
        bool mayHaveWidget(UBDocumentProxy* pDocumentProxy);

        QString adjustDocumentVirtualPath(const QString &str);

        void closing();
        bool isSceneInCached(UBDocumentProxy *proxy, int index) const;

    signals:

        void proxyListChanged();

        void documentCreated(UBDocumentProxy* pDocumentProxy);
        void documentMetadataChanged(UBDocumentProxy* pDocumentProxy);
        void documentWillBeDeleted(UBDocumentProxy* pDocumentProxy);

        void documentSceneCreated(UBDocumentProxy* pDocumentProxy, int pIndex);
        void documentSceneWillBeDeleted(UBDocumentProxy* pDocumentProxy, int pIndex);

private:
        int sceneCount(const UBDocumentProxy* pDocumentProxy);
        static QStringList getSceneFileNames(const QString& folder);
        void renamePage(UBDocumentProxy* pDocumentProxy,
                        const int sourceIndex, const int targetIndex);
        void copyPage(UBDocumentProxy* pDocumentProxy,
                      const int sourceIndex, const int targetIndex);
        void generatePathIfNeeded(UBDocumentProxy* pDocumentProxy);
        void checkIfDocumentRepositoryExists();

        void saveFoldersTreeToXml(QXmlStreamWriter &writer, const QModelIndex &parentIndex);
        void loadFolderTreeFromXml(const QString &path, const QDomElement &element);

        QString xmlFolderStructureFilename;

        UBSceneCache mSceneCache;
        QStringList mDocumentSubDirectories;
        QMutex mDeletedListMutex;
        bool mHasPurgedDocuments;
        QString mDocumentRepositoryPath;
        QString mFoldersXmlStorageName;

    private slots:
        void documentRepositoryChanged(const QString& path);

};


#endif /* UBPERSISTENCEMANAGER_H_ */
