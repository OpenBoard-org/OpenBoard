/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

        static UBPersistenceManager* persistenceManager();
        static void destroy();

        virtual UBDocumentProxy* createDocument(const QString& pGroupName = "", const QString& pName = "", bool withEmptyPage = true);
        virtual UBDocumentProxy* createDocumentFromDir(const QString& pDocumentDirectory, const QString& pGroupName = "", const QString& pName = "", bool withEmptyPage = false);

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
        UBGraphicsScene *getDocumentScene(UBDocumentProxy* pDocumentProxy, int sceneIndex) {return mSceneCache.value(pDocumentProxy, sceneIndex);}

        QList<QPointer<UBDocumentProxy> > documentProxies;

        virtual QStringList allShapes();
        virtual QStringList allGips();
        virtual QStringList allImages(const QDir& dir);
        virtual QStringList allVideos(const QDir& dir);
        virtual QStringList allWidgets(const QDir& dir);

        virtual QString generateUniqueDocumentPath();

        QString teacherGuideAbsoluteObjectPath(UBDocumentProxy* pDocumentProxy);
        QString addObjectToTeacherGuideDirectory(UBDocumentProxy* proxy, QString pPath);

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
        virtual QString addGraphicsWidgteToDocument(UBDocumentProxy *mDocumentProxy, QString path, QUuid objectUuid);

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

        QHash<int,QString>teacherBarNodeString;

    private slots:
        void documentRepositoryChanged(const QString& path);

};


#endif /* UBPERSISTENCEMANAGER_H_ */
