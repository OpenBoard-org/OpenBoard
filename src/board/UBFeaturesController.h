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




#ifndef UBFEATURESCONTROLLER_H
#define UBFEATURESCONTROLLER_H

#include <QMetaType>
#include <QObject>
#include <QWidget>
#include <QSet>
#include <QVector>
#include <QString>
#include <QPixmap>
#include <QMap>
#include <QUrl>
#include <QByteArray>
#include <QtGui>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QListView>

class UBFeaturesModel;
class UBFeaturesItemDelegate;
class UBFeaturesPathItemDelegate;
class UBFeaturesProxyModel;
class UBFeaturesSearchProxyModel;
class UBFeaturesPathProxyModel;
class UBFeaturesListView;
class UBFeature;


class UBFeaturesComputingThread : public QThread
{
    Q_OBJECT
public:
    explicit UBFeaturesComputingThread(QObject *parent = 0);
    virtual ~UBFeaturesComputingThread();
        void compute(const QList<QPair<QUrl, UBFeature> > &pScanningData, QSet<QUrl> *pFavoritesSet);

protected:
    void run();

signals:
    void sendFeature(UBFeature pFeature);
    void featureSent();
    void scanStarted();
    void scanFinished();
    void maxFilesCountEvaluated(int max);
    void scanCategory(const QString &str);
    void scanPath(const QString &str);

public slots:

private:
    void scanFS(const QUrl & currentPath, const QString & currVirtualPath, const QSet<QUrl> &pFavoriteSet);
    void scanAll(QList<QPair<QUrl, UBFeature> > pScanningData, const QSet<QUrl> &pFavoriteSet);
    int featuresCount(const QUrl &pPath);
    int featuresCountAll(QList<QPair<QUrl, UBFeature> > pScanningData);

private:
    QMutex mMutex;
    QWaitCondition mWaitCondition;
    QUrl mScanningPath;
    QString mScanningVirtualPath;
    QList<QPair<QUrl, UBFeature> > mScanningData;
    QSet<QUrl> mFavoriteSet;
    bool restart;
    bool abort;
};


enum UBFeatureElementType
{
    FEATURE_CATEGORY,
    FEATURE_VIRTUALFOLDER,
    FEATURE_FOLDER,
    FEATURE_INTERACTIVE,
    FEATURE_INTERNAL,
    FEATURE_ITEM,
    FEATURE_AUDIO,
    FEATURE_VIDEO,
    FEATURE_IMAGE,
    FEATURE_FLASH,
    FEATURE_TRASH,
    FEATURE_FAVORITE,
    FEATURE_SEARCH,
    FEATURE_INVALID
};

class UBFeature
{
public:
    UBFeature() {;}
    UBFeature(const QString &url, const QImage &icon, const QString &name, const QUrl &realPath, UBFeatureElementType type = FEATURE_CATEGORY);
    virtual ~UBFeature();
    QString getName() const { return mName; }
    QString getDisplayName() const {return mDisplayName;}
    QImage getThumbnail() const {return mThumbnail;}
    QString getVirtualPath() const { return virtualDir; }
    QUrl getFullPath() const { return mPath; }
    QString getFullVirtualPath() const { return  virtualDir + "/" + mName; }
    QString getUrl() const;
    void setFullPath(const QUrl &newPath) {mPath = newPath;}
    void setFullVirtualPath(const QString &newVirtualPath) {virtualDir = newVirtualPath;}
    UBFeatureElementType getType() const { return elementType; }

    bool isFolder() const;
    bool allowedCopy() const;
    bool isDeletable() const;
    bool inTrash() const;
    bool operator ==( const UBFeature &f )const;
    bool operator !=( const UBFeature &f )const;
    const QMap<QString,QString> & getMetadata() const { return metadata; }
    void setMetadata( const QMap<QString,QString> &data ) { metadata = data; }


private:
    QString getNameFromVirtualPath(const QString &pVirtPath);
    QString getVirtualDirFromVirtualPath(const QString &pVirtPath);

private:
    QString virtualDir;
    QString virtualPath;
    QImage mThumbnail;
    QString mName;
    QString mDisplayName;
    QUrl mPath;
    UBFeatureElementType elementType;
    QMap<QString,QString> metadata;
};
Q_DECLARE_METATYPE( UBFeature )

class UBFeaturesController : public QObject
{
friend class UBFeaturesWidget;

Q_OBJECT

public:
    UBFeaturesController(QWidget *parentWidget);
    virtual ~UBFeaturesController();

    QList <UBFeature>* getFeatures() const {return featuresList;}

    const QString& getRootPath()const {return rootPath;}
    void scanFS();

    void addItemToPage(const UBFeature &item);
    void addItemAsBackground(const UBFeature &item);
    const UBFeature& getCurrentElement()const {return currentElement;}
    void setCurrentElement( const UBFeature &elem ) {currentElement = elem;}
    const UBFeature & getTrashElement () const { return trashElement; }

    void addDownloadedFile( const QUrl &sourceUrl, const QByteArray &pData, const QString pContentSource, const QString pTitle );

    UBFeature moveItemToFolder( const QUrl &url, const UBFeature &destination );
    UBFeature copyItemToFolder( const QUrl &url, const UBFeature &destination );
    void moveExternalData(const QUrl &url, const UBFeature &destination);

    void rescanModel();
    void siftElements(const QString &pSiftValue);
    //TODO make less complicated for betteer maintainence
    UBFeature getFeature(const QModelIndex &index, const QString &listName);
    void searchStarted(const QString &pattern, QListView *pOnView);
    void refreshModels();

    void deleteItem( const QUrl &url );
    void deleteItem(const UBFeature &pFeature);
    bool isTrash( const QUrl &url );
    void moveToTrash(UBFeature feature, bool deleteManualy = false);
    void addToFavorite( const QUrl &path );
    void removeFromFavorite(const QUrl &path, bool deleteManualy = false);
    void importImage(const QImage &image, const QString &fileName = QString());
    void importImage( const QImage &image, const UBFeature &destination, const QString &fileName = QString() );
    QStringList getFileNamesInFolders();

    void fileSystemScan(const QUrl &currPath, const QString & currVirtualPath);
    int featuresCount(const QUrl &currPath);
    static UBFeatureElementType fileTypeFromUrl( const QString &path );

    static QString fileNameFromUrl( const QUrl &url );
    static QImage getIcon( const QString &path, UBFeatureElementType pFType );
    static bool isDeletable( const QUrl &url );
    static char featureTypeSplitter() {return ':';}
    static QString categoryNameForVirtualPath(const QString &str);

    static const QString virtualRootName;

    void assignFeaturesListView(UBFeaturesListView *pList);
    void assignPathListView(UBFeaturesListView *pList);

public:
    static const QString rootPath;
    static const QString audiosPath;
    static const QString moviesPath;
    static const QString picturesPath;
    static const QString appPath;
    static const QString flashPath;
    static const QString shapesPath;
    static const QString interactPath;
    static const QString trashPath;
    static const QString favoritePath;
    static const QString webSearchPath;

signals:
    void maxFilesCountEvaluated(int pLimit);
    void scanStarted();
    void scanFinished();
    void featureAddedFromThread();
    void scanCategory(const QString &);
    void scanPath(const QString &);

private slots:
    void addNewFolder(QString name);
    void startThread();
    void createNpApiFeature(const QString &str);

private:

    UBFeaturesItemDelegate *itemDelegate;
    UBFeaturesPathItemDelegate *pathItemDelegate;

    UBFeaturesModel *featuresModel;
    UBFeaturesProxyModel *featuresProxyModel;
    UBFeaturesSearchProxyModel *featuresSearchModel;
    UBFeaturesPathProxyModel *featuresPathModel;

    QAbstractItemModel *curListModel;
    UBFeaturesComputingThread mCThread;

private:

    static QImage createThumbnail(const QString &path);
    //void addImageToCurrentPage( const QString &path );
    void loadFavoriteList();
    void saveFavoriteList();
    QString uniqNameForFeature(const UBFeature &feature, const QString &pName = "Imported", const QString &pExtention = "") const;
    QString adjustName(const QString &str);

    QList <UBFeature> *featuresList;

    QUrl mUserAudioDirectoryPath;
    QUrl mUserVideoDirectoryPath;
    QUrl mUserPicturesDirectoryPath;
    QUrl mUserInteractiveDirectoryPath;
    QUrl mUserAnimationDirectoryPath;

    QString libraryPath;
    QUrl mLibPicturesDirectoryPath;
    QUrl mLibAudiosDirectoryPath;
    QUrl mLibVideosDirectoryPath;
    QUrl mLibInteractiveDirectoryPath;
    QUrl mLibAnimationsDirectoryPath;
    QUrl mLibApplicationsDirectoryPath;
    QUrl mLibShapesDirectoryPath;

    QUrl trashDirectoryPath;
    QUrl mLibSearchDirectoryPath;



    int mLastItemOffsetIndex;
    UBFeature currentElement;

    UBFeature rootElement;
    UBFeature favoriteElement;
    UBFeature audiosElement;
    UBFeature moviesElement;
    UBFeature picturesElement;
    UBFeature interactElement;
    UBFeature applicationsElement;
    UBFeature flashElement;
    UBFeature shapesElement;
    UBFeature webSearchElement;

    QSet <QUrl> *favoriteSet;

public:
    UBFeature trashElement;
    UBFeature getDestinationFeatureForUrl( const QUrl &url );
    UBFeature getDestinationFeatureForMimeType(const QString &pMmimeType);

};

#endif
