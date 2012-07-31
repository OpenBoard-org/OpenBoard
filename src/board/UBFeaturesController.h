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

class UBFeaturesModel;
class UBFeaturesItemDelegate;
class UBFeaturesPathItemDelegate;
class UBFeaturesProxyModel;
class UBFeaturesSearchProxyModel;
class UBFeaturesPathProxyModel;
class UBFeaturesListView;

//#include "UBDockPaletteWidget.h"

enum UBFeatureElementType
{
    FEATURE_CATEGORY,
    FEATURE_VIRTUALFOLDER,
    FEATURE_FOLDER,
    FEATURE_INTERACTIVE,
	FEATURE_INTERNAL,
    FEATURE_ITEM,
	FEATURE_TRASH,
	FEATURE_FAVORITE,
	FEATURE_SEARCH
};

class UBFeature
{
public:
    UBFeature() {;}
	//UBFeature(const UBFeature &f);
    UBFeature(const QString &url, const QPixmap &icon, const QString &name, const QUrl &realPath, UBFeatureElementType type = FEATURE_CATEGORY);
//    UBFeature();
    virtual ~UBFeature();
    QString getName() const { return mName; }
    QPixmap getThumbnail() const {return mThumbnail;}
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

    bool hasChildren() const {return mChildren.count();}
    bool hasParents() const {return mParents.count();}
    bool hasRelationships() const {return mChildren.count() && mParents.count();}

private:
    QString virtualDir;
    QPixmap mThumbnail;
    QString mName;
	QUrl mPath;
    UBFeatureElementType elementType;
    QMap<QString,QString> metadata;

    QList<UBFeature*> mChildren;
    QList<UBFeature*> mParents;
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

    UBFeature addDownloadedFile( const QUrl &sourceUrl, const QByteArray &pData );

	UBFeature moveItemToFolder( const QUrl &url, const UBFeature &destination );
	UBFeature copyItemToFolder( const QUrl &url, const UBFeature &destination );

    void rescanModel();
    void siftElements(const QString &pSiftValue);
    //TODO make less complicated for betteer maintainence
    UBFeature getFeature(const QModelIndex &index, QListView *pOnView);
    void searchStarted(const QString &pattern, QListView *pOnView);
    void refreshModels();

	void deleteItem( const QUrl &url );
    void deleteItem(const UBFeature &pFeature);
	bool isTrash( const QUrl &url );
    void moveToTrash(UBFeature feature, bool deleteManualy = false);
    void addNewFolder(const QString &name);
    void addToFavorite( const QUrl &path );
    void removeFromFavorite(const QUrl &path, bool deleteManualy = false);
    UBFeature importImage( const QImage &image, const UBFeature &destination );

    void fileSystemScan(const QUrl &currPath, const QString & currVirtualPath);


	static QString fileNameFromUrl( const QUrl &url );
	static QPixmap thumbnailForFile( const QString &path );
	static bool isDeletable( const QUrl &url );
    static char featureTypeSplitter() {return ':';}

    static const QString virtualRootName;

    void assignFeaturesListVeiw(UBFeaturesListView *pList);
    void assignPathListView(UBFeaturesListView *pList);

private:

    UBFeaturesItemDelegate *itemDelegate;
    UBFeaturesPathItemDelegate *pathItemDelegate;

    UBFeaturesModel *featuresModel;
    UBFeaturesProxyModel *featuresProxyModel;
    UBFeaturesSearchProxyModel *featuresSearchModel;
    UBFeaturesPathProxyModel *featuresPathModel;


private:

    static QPixmap createThumbnail(const QString &path);
	//void addImageToCurrentPage( const QString &path );
	void loadFavoriteList();
	void saveFavoriteList();

	static UBFeatureElementType fileTypeFromUrl( const QString &path );

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
    UBFeature getParentFeatureForUrl( const QUrl &url );

};

#endif
