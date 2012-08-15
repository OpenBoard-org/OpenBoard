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
        void compute(const QList<QPair<QUrl, QString> > &pScanningData, QSet<QUrl> *pFavoritesSet);

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
    void scanAll(QList<QPair<QUrl, QString> > pScanningData, const QSet<QUrl> &pFavoriteSet);
    int featuresCount(const QUrl &pPath);
    int featuresCountAll(QList<QPair<QUrl, QString> > pScanningData);

private:
    QMutex mMutex;
    QWaitCondition mWaitCondition;
    QUrl mScanningPath;
    QString mScanningVirtualPath;
    QList<QPair<QUrl, QString> > mScanningData;
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
	FEATURE_TRASH,
	FEATURE_FAVORITE,
    FEATURE_SEARCH,
    FEATURE_INVALID
};

class UBFeature
{
public:
    UBFeature() {;}
//    UBFeature(const UBFeature &f);
    UBFeature(const QString &url, const QImage &icon, const QString &name, const QUrl &realPath, UBFeatureElementType type = FEATURE_CATEGORY);
//    UBFeature();
    virtual ~UBFeature();
    QString getName() const { return mName; }
    QImage getThumbnail() const {return mThumbnail;}
    QString getVirtualPath() const { return virtualDir; }
	//QString getPath() const { return mPath; };
    QUrl getFullPath() const { return mPath; }
    QString getFullVirtualPath() const { return  virtualDir + "/" + mName; }
	QString getUrl() const;
    void setFullPath(const QUrl &newPath) {mPath = newPath;}
    void setFullVirtualPath(const QString &newVirtualPath) {virtualDir = newVirtualPath;}
    UBFeatureElementType getType() const { return elementType; }

	bool isFolder() const;
	bool isDeletable() const;
    bool inTrash() const;
	bool operator ==( const UBFeature &f )const;
	bool operator !=( const UBFeature &f )const;
	const QMap<QString,QString> & getMetadata() const { return metadata; }
	void setMetadata( const QMap<QString,QString> &data ) { metadata = data; }

private:
    QString virtualDir;
    QString virtualPath;
    QImage mThumbnail;
    QString mName;
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

    void addDownloadedFile( const QUrl &sourceUrl, const QByteArray &pData );

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

    void assignFeaturesListVeiw(UBFeaturesListView *pList);
    void assignPathListView(UBFeaturesListView *pList);

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

	QString rootPath;
	QString audiosPath;
	QString moviesPath;
	QString picturesPath;
	QString appPath;
	QString flashPath;
	QString shapesPath;
	QString interactPath;
	QString trashPath;
	QString favoritePath;
    QString webSearchPath;

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

};

#endif
