#ifndef UBFEATURESCONTROLLER_H
#define UBFEATURESCONTROLLER_H

#include <QMetaType>
#include <QObject>
#include <QWidget>
#include <QVector>
#include <QString>
#include <QPixmap>

//#include "UBDockPaletteWidget.h"

enum UBFeatureElementType
{
    FEATURE_CATEGORY,
    FEATURE_VIRTUALFOLDER,
    FEATURE_FOLDER,
    FEATURE_INTERACTIVE,
	FEATURE_INTERNAL,
    FEATURE_ITEM,
	FEATURE_TRASH
};

class UBFeature
{
public:
    UBFeature() {;}
	UBFeature(const UBFeature &f);
    UBFeature(const QString &url, const QPixmap &icon, const QString &name, const QString &realPath, UBFeatureElementType type = FEATURE_CATEGORY);
    virtual ~UBFeature() {;}
    QString getName() const { return mName; }
    QPixmap getThumbnail() const {return mThumbnail;}
    QString getUrl() const { return virtualPath; }
	//QString getPath() const { return mPath; };
    QString getFullPath() const { return mPath; }
    UBFeatureElementType getType() const { return elementType; }
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

    QVector <UBFeature>* getFeatures()const { return featuresList; }
	
    const QString& getRootPath()const { return rootPath; }

	void addItemToPage(const UBFeature &item);
	const UBFeature& getCurrentElement()const { return currentElement; }
	void setCurrentElement( const UBFeature &elem ) { currentElement = elem; }
	const UBFeature & getTrashElement () const { return trashElement; }

	static QPixmap thumbnailForFile( const QString &path );
	static UBFeature moveItemToFolder( const QUrl &url, const UBFeature &destination );
	static UBFeature copyItemToFolder( const QUrl &url, const UBFeature &destination );
	static void deleteItem( const QUrl &url );
	bool isTrash( const QUrl &url );
	UBFeature newFolder( const QString &name );
private:
	void initDirectoryTree();
	void fileSystemScan(const QString &currPath, const QString & currVirtualPath);
	static QPixmap createThumbnail(const QString &path);
	//void addImageToCurrentPage( const QString &path );

	QVector <UBFeature> *featuresList;
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

	QString rootPath;
	QString audiosPath;
	QString moviesPath;
	QString picturesPath;
	QString appPath;
	QString flashPath;
	QString shapesPath;
	QString interactPath;
	QString trashPath;

	int mLastItemOffsetIndex;
	UBFeature currentElement;
	UBFeature trashElement;
};



#endif
