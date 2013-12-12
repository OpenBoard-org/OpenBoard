/*
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

class UBDocument;
class UBDocumentProxy;
class UBGraphicsScene;

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
        static const QString teacherGuideDirectory;
        static void shiftPagesToStartWithTheZeroOne(QString persistencePath);

        static UBPersistenceManager* persistenceManager();
        static void destroy();

        virtual UBDocumentProxy* createDocument(const QString& pGroupName = "", const QString& pName = "", bool withEmptyPage = true);
        virtual UBDocumentProxy* createDocumentFromDir(const QString& pDocumentDirectory, const QString& pGroupName = "", const QString& pName = "");

        virtual UBDocumentProxy* persistDocumentMetadata(UBDocumentProxy* pDocumentProxy);

        virtual UBDocumentProxy* duplicateDocument(UBDocumentProxy* pDocumentProxy);

        virtual void deleteDocument(UBDocumentProxy* pDocumentProxy);

        virtual void deleteDocumentScenes(UBDocumentProxy* pDocumentProxy, const QList<int>& indexes);

        virtual void duplicateDocumentScene(UBDocumentProxy* pDocumentProxy, int index);

        virtual void persistDocumentScene(UBDocumentProxy* pDocumentProxy,
                UBGraphicsScene* pScene, const int pSceneIndex, bool isAnAutomaticBackup = false);

        virtual UBGraphicsScene* createDocumentSceneAt(UBDocumentProxy* pDocumentProxy, int index, bool useUndoRedoStack = true);

        virtual void insertDocumentSceneAt(UBDocumentProxy* pDocumentProxy, UBGraphicsScene* scene, int index);

        virtual void moveSceneToIndex(UBDocumentProxy* pDocumentProxy, int source, int target);

        virtual UBGraphicsScene* loadDocumentScene(UBDocumentProxy* pDocumentProxy, int sceneIndex);
        UBGraphicsScene *getDocumentScene(UBDocumentProxy* pDocumentProxy, int sceneIndex) {return mSceneCache.value(pDocumentProxy, sceneIndex);}

        QList<QPointer<UBDocumentProxy> > documentProxies;

        virtual QStringList allShapes();
        virtual QStringList allGips();
        virtual QStringList allImages(const QDir& dir);
        virtual QStringList allVideos(const QDir& dir);
        virtual QStringList allWidgets(const QDir& dir);

        QString generateUniqueDocumentPath();
        QString generateUniqueDocumentPath(const QString& baseFolder);

        bool addDirectoryContentToDocument(const QString& documentRootFolder, UBDocumentProxy* pDocument);

        virtual UBDocumentProxy* documentByUuid(const QUuid& pUuid);

        QStringList documentSubDirectories()
        {
            return mDocumentSubDirectories;
        }

        virtual bool isEmpty(UBDocumentProxy* pDocumentProxy);
        virtual void purgeEmptyDocuments();

        bool addGraphicsWidgetToDocument(UBDocumentProxy *mDocumentProxy, QString path, QUuid objectUuid, QString& destinationPath);
        bool addFileToDocument(UBDocumentProxy* pDocumentProxy, QString path, const QString& subdir,  QUuid objectUuid, QString& destinationPath, QByteArray* data = NULL);

    signals:
        void documentCreated(UBDocumentProxy* pDocumentProxy);
        void documentMetadataChanged(UBDocumentProxy* pDocumentProxy);
        void documentWillBeDeleted(UBDocumentProxy* pDocumentProxy);

        void documentSceneCreated(UBDocumentProxy* pDocumentProxy, int pIndex);
        void documentSceneWillBeDeleted(UBDocumentProxy* pDocumentProxy, int pIndex);

    private:

        int sceneCount(const UBDocumentProxy* pDocumentProxy);

        static QStringList getSceneFileNames(const QString& folder);

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

        QHash<int,QString>teacherBarNodeString;

    private slots:
        void documentRepositoryChanged(const QString& path);

};


#endif /* UBPERSISTENCEMANAGER_H_ */
