#ifndef UBFEATURESCONTROLLER_H
#define UBFEATURESCONTROLLER_H

#include <QMetaType>
#include <QObject>
#include <QWidget>
#include <QSet>
#include <QVector>
#include <QString>
#include <QPixmap>

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
		UBFeature(const QString &url, const QPixmap &icon, const QString &name, const QString &realPath, UBFeatureElementType type = FEATURE_CATEGORY);
		virtual ~UBFeature() {;}
		QString getName() const { return mName; }
		QPixmap getThumbnail() const {return mThumbnail;}
		QString getUrl() const { return virtualPath; }
		//QString getPath() const { return mPath; };
		QString getFullPath() const { return mPath; }
		QString getVirtualPath() const { return  virtualPath + "/" + mName; }
		UBFeatureElementType getType() const { return elementType; }
		bool isFolder() const;
		bool isDeletable() const;
		bool operator ==( const UBFeature &f )const;
		bool operator !=( const UBFeature &f )const;
	private:
		QString virtualPath;
		QPixmap mThumbnail;
		QString mName;
		QString mPath;
		UBFeatureElementType elementType;
};
Q_DECLARE_METATYPE( UBFeature )


	class UBFeaturesController : public QObject
{
	Q_OBJECT
	public:
		UBFeaturesController(QWidget *parentWidget);
		virtual ~UBFeaturesController();

		QList <UBFeature>* getFeatures()const { return featuresList; }

		const QString& getRootPath()const { return rootPath; }

		void addItemToPage(const UBFeature &item);
		const UBFeature& getCurrentElement()const { return currentElement; }
		void setCurrentElement( const UBFeature &elem ) { currentElement = elem; }
		const UBFeature & getTrashElement () const { return trashElement; }
		UBFeature moveItemToFolder( const QUrl &url, const UBFeature &destination );
		UBFeature copyItemToFolder( const QUrl &url, const UBFeature &destination );
		void deleteItem( const QUrl &url );
		bool isTrash( const QUrl &url );
		UBFeature newFolder( const QString &name );
		UBFeature addToFavorite( const QUrl &path );
		void removeFromFavorite( const QUrl &path );

		static QString fileNameFromUrl( const QUrl &url );
		static QPixmap thumbnailForFile( const QString &path );
		static bool isDeletable( const QUrl &url );
	private:
		void initDirectoryTree();
		void fileSystemScan(const QString &currPath, const QString & currVirtualPath);
		static QPixmap createThumbnail(const QString &path);
		//void addImageToCurrentPage( const QString &path );
		void loadFavoriteList();
		void saveFavoriteList();
		UBFeature getDestinationForItem( const QUrl &url );

		static UBFeatureElementType fileTypeFromUrl( const QString &path );

		QList <UBFeature> *featuresList;
		UBFeature *rootElement;

		QString mUserAudioDirectoryPath;
		QString mUserVideoDirectoryPath;
		QString mUserPicturesDirectoryPath;
		QString mUserInteractiveDirectoryPath;
		QString mUserAnimationDirectoryPath;

		QString libraryPath;
		QString mLibAudioDirectoryPath;
		QString mLibVideoDirectoryPath;
		QString mLibPicturesDirectoryPath;
		QString mLibInteractiveDirectoryPath;
		QString mLibAnimationDirectoryPath;
		QString mLibApplicationsDirectoryPath;
		QString mLibShapesDirectoryPath;
		QString trashDirectoryPath;
		QString mLibSearchDirectoryPath;

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
		UBFeature trashElement;
		UBFeature favoriteElement;
		UBFeature audiosElement;
		UBFeature moviesElement;
		UBFeature picturesElement;
		UBFeature interactElement;
		UBFeature flashElement;
		UBFeature shapesElement;
		UBFeature searchElement;

		QSet <QString> *favoriteSet;
};



#endif
