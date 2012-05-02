#include <QGraphicsItem>
#include <QPointF>

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "UBFeaturesController.h"
#include "core/UBSettings.h"
#include "tools/UBToolsManager.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"


#include "core/UBDownloadManager.h"
#include "domain/UBAbstractWidget.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsVideoItem.h"
#include "domain/UBGraphicsWidgetItem.h"

UBFeature::UBFeature(const QString &url, const QPixmap &icon, const QString &name, const QString &realPath, UBFeatureElementType type)
: virtualPath(url), mThumbnail(icon), mName(name), mPath(realPath), elementType(type)
{
	
}



bool UBFeature::isFolder() const
{
	return elementType == FEATURE_CATEGORY || elementType == FEATURE_TRASH || elementType == FEATURE_FAVORITE
		|| elementType == FEATURE_FOLDER;
}


UBFeaturesController::UBFeaturesController(QWidget *pParentWidget) :
        QObject(pParentWidget),
		mLastItemOffsetIndex(0)
{
	rootPath = "/root";
	initDirectoryTree();
}

void UBFeaturesController::initDirectoryTree()
{
	mUserAudioDirectoryPath = UBSettings::settings()->userAudioDirectory();
	mUserVideoDirectoryPath = UBSettings::settings()->userVideoDirectory();
    mUserPicturesDirectoryPath = UBSettings::settings()->userImageDirectory();
    mUserInteractiveDirectoryPath = UBSettings::settings()->userInteractiveDirectory();
    mUserAnimationDirectoryPath = UBSettings::settings()->userAnimationDirectory();

	mLibPicturesDirectoryPath = UBSettings::settings()->applicationImageLibraryDirectory();
	mLibInteractiveDirectoryPath = UBSettings::settings()->applicationInteractivesDirectory();
	mLibApplicationsDirectoryPath = UBSettings::settings()->applicationApplicationsLibraryDirectory();
	mLibShapesDirectoryPath = UBSettings::settings()->applicationShapeLibraryDirectory() ;
	trashDirectoryPath = UBSettings::userTrashDirPath();

	featuresList = new QList <UBFeature>();

	QList <UBToolsManager::UBToolDescriptor> tools = UBToolsManager::manager()->allTools();

	featuresList->append( UBFeature( QString(), QPixmap( ":images/libpalette/home.png" ), "root", QString() ) );
	currentElement = featuresList->at(0);
	
	appPath = rootPath + "/Applications";
	audiosPath = rootPath + "/Audios";
	moviesPath = rootPath + "/Movies";
	picturesPath = rootPath + "/Pictures";
	flashPath = rootPath + "/Animations";
	interactPath = rootPath + "/Interactivities";
	shapesPath = rootPath + "/Shapes";
	trashPath = rootPath + "/Trash";
	favoritePath = rootPath + "/Favorites";

	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/AudiosCategory.svg"), "Audios" , mUserAudioDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/MoviesCategory.svg"), "Movies" , mUserVideoDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/PicturesCategory.svg"), "Pictures" , mUserPicturesDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/ApplicationsCategory.svg"), "Applications" , mUserInteractiveDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/FlashCategory.svg"), "Animations" , mUserAnimationDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/InteractivesCategory.svg"), "Interactivities" ,  mLibInteractiveDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/ShapesCategory.svg"), "Shapes" , mLibShapesDirectoryPath ) );
	trashElement = UBFeature( rootPath, QPixmap(":images/libpalette/TrashCategory.svg"), "Trash", trashDirectoryPath, FEATURE_TRASH );
	featuresList->append( trashElement );
	favoriteElement = UBFeature( rootPath, QPixmap(":images/libpalette/FavoritesCategory.svg"), "Favorites", "favorites", FEATURE_FAVORITE );
	featuresList->append( favoriteElement );

	loadFavoriteList();

	foreach (UBToolsManager::UBToolDescriptor tool, tools)
	{
		featuresList->append( UBFeature( appPath, tool.icon, tool.label, tool.id, FEATURE_INTERNAL ) );
		if ( favoriteSet->find( tool.id ) != favoriteSet->end() )
		{
			featuresList->append( UBFeature( favoritePath, tool.icon, tool.label, tool.id, FEATURE_INTERNAL ) );
		}
	}
	fileSystemScan( mUserInteractiveDirectoryPath, appPath  );
	fileSystemScan( mUserAudioDirectoryPath, audiosPath  );
	fileSystemScan( mUserPicturesDirectoryPath, picturesPath  );
	fileSystemScan( mUserVideoDirectoryPath, moviesPath  );
	fileSystemScan( mUserAnimationDirectoryPath, flashPath  );

	fileSystemScan( mLibApplicationsDirectoryPath, appPath  );
	fileSystemScan( mLibPicturesDirectoryPath, picturesPath  );
	fileSystemScan( mLibShapesDirectoryPath, shapesPath  );
	fileSystemScan( mLibInteractiveDirectoryPath, interactPath  );
	fileSystemScan( trashDirectoryPath, trashPath );

	

}

void UBFeaturesController::fileSystemScan(const QString & currentPath, const QString & currVirtualPath)
{
	QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(currentPath);

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1)
	{
		UBFeatureElementType fileType = fileInfo->isDir() ? FEATURE_FOLDER : FEATURE_ITEM;

        QString fileName = fileInfo->fileName();
        if ( UBFileSystemUtils::mimeTypeFromFileName(fileName).contains("application") ) {
            fileType = FEATURE_INTERACTIVE;
        }
		QString itemName = (fileType != FEATURE_ITEM) ? fileName : fileInfo->completeBaseName();
		QPixmap icon = QPixmap(":images/libpalette/soundIcon.svg");
		QString fullFileName = fileInfo->filePath();

		if ( fileType == FEATURE_FOLDER ) 
		{
            icon = QPixmap(":images/libpalette/folder.svg");
        }
        else if ( fileType == FEATURE_INTERACTIVE )
		{
			icon = QPixmap( UBAbstractWidget::iconFilePath( QUrl::fromLocalFile(fullFileName) ) );
		}
		else
		{
			if ( fullFileName.contains(".thumbnail.") )
                continue;
			icon = thumbnailForFile( fullFileName );
			/*QString thumbnailPath = UBFileSystemUtils::thumbnailPath( fullFileName );

			if (QFileInfo( thumbnailPath).exists() )
				icon = QPixmap( thumbnailPath );
			else icon = createThumbnail( fullFileName );*/
		}
		featuresList->append( UBFeature( currVirtualPath, icon, fileName, fullFileName, fileType ) );
		if ( favoriteSet->find( fullFileName ) != favoriteSet->end() )
		{
			featuresList->append( UBFeature( favoritePath, icon, fileName, fullFileName, fileType ) );
		}

		if ( fileType == FEATURE_FOLDER )
		{
			fileSystemScan( fullFileName, currVirtualPath + "/" + fileName );
		}

	}
}

void UBFeaturesController::loadFavoriteList()
{
	favoriteSet = new QSet<QString>();
	QFile file( UBSettings::userDataDirectory() + "/favorites.dat" );
	if ( file.exists() )
	{
		file.open(QIODevice::ReadOnly);
		QDataStream in(&file);
		int elementsNumber;
		in >> elementsNumber;
		for ( int i = 0; i < elementsNumber; ++i)
		{
			QString path;
			in >> path;
			/*QFileInfo fileInfo( path );
			QString fileName = fileInfo.fileName();

			UBFeature elem( favoritePath, thumbnailForFile( path ), fileName, path, fileTypeFromUrl(path) );
			featuresList->append( elem );*/
			favoriteSet->insert( path );
		}
	}
}

void UBFeaturesController::saveFavoriteList()
{
	QFile file( UBSettings::userDataDirectory() + "/favorites.dat" );
	file.resize(0);
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	out << favoriteSet->size();
	for ( QSet<QString>::iterator it = favoriteSet->begin(); it != favoriteSet->end(); ++it )
	{
		out << (*it);
	}
	file.close();
}

UBFeature UBFeaturesController::addToFavorite( const QUrl &path )
{
	QString filePath = fileNameFromUrl( path );
	if ( favoriteSet->find( filePath ) == favoriteSet->end() )
	{
		QFileInfo fileInfo( filePath );
		QString fileName = fileInfo.fileName();
		UBFeature elem( favoritePath, thumbnailForFile( filePath ), fileName, filePath, fileTypeFromUrl(filePath) );
		favoriteSet->insert( filePath );
		saveFavoriteList();
		return elem;
	}
	return UBFeature();
}

void UBFeaturesController::removeFromFavorite( const QUrl &path )
{
	QString filePath = fileNameFromUrl( path );
	if ( favoriteSet->find( filePath ) != favoriteSet->end() )
	{
		favoriteSet->erase( favoriteSet->find( filePath ) );
		saveFavoriteList();
	}
}

QString UBFeaturesController::fileNameFromUrl( const QUrl &url )
{
	QString fileName = url.toString();
	if ( fileName.contains( "uniboardTool://" ) )
		return fileName;
	return url.toLocalFile();
}

UBFeatureElementType UBFeaturesController::fileTypeFromUrl( const QString &path )
{
	QFileInfo fileInfo( path );
	QString fileName = fileInfo.fileName();

	UBFeatureElementType fileType = fileInfo.isDir() ? FEATURE_FOLDER : FEATURE_ITEM;
	if ( UBFileSystemUtils::mimeTypeFromFileName(fileName).contains("application") ) 
	{
		fileType = FEATURE_INTERACTIVE;
	}
	else if ( path.contains("uniboardTool://")  )
	{
		fileType = FEATURE_INTERNAL;
	}
	return fileType;
}

QPixmap UBFeaturesController::thumbnailForFile(const QString &path)
{
	if ( path.contains("uniboardTool://") )
	{
            return QPixmap( UBToolsManager::manager()->iconFromToolId(path) );
    }
    if ( UBFileSystemUtils::mimeTypeFromFileName(path).contains("application") )
	{
        return QPixmap( UBAbstractWidget::iconFilePath( QUrl::fromLocalFile(path) ) );
    }

	QPixmap thumb;
	QString thumbnailPath = UBFileSystemUtils::thumbnailPath( path );

	if ( QFileInfo( thumbnailPath ).exists() )
		thumb = QPixmap( thumbnailPath );
	else thumb = createThumbnail( path );
	return thumb;
}

QPixmap UBFeaturesController::createThumbnail(const QString &path)
{
    QString thumbnailPath = UBFileSystemUtils::thumbnailPath(path);
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName(path);
	QString extension = QFileInfo(path).completeSuffix();
    //UBApplication::showMessage(tr("Creating image thumbnail for %1.").arg(pElement->name()));

    if ( mimetype.contains("audio" ))
        thumbnailPath = ":images/libpalette/soundIcon.svg";
    else if ( mimetype.contains("video") )
        thumbnailPath = ":images/libpalette/movieIcon.svg";
    else 
	{
        if ( extension.startsWith("svg", Qt::CaseInsensitive) || extension.startsWith("svgz", Qt::CaseInsensitive) ) 
		{
            thumbnailPath = path;
        }
        else 
		{
            QPixmap pix(path);
            if (!pix.isNull()) 
			{
                pix = pix.scaledToWidth(qMin(UBSettings::maxThumbnailWidth, pix.width()), Qt::SmoothTransformation);
                pix.save(thumbnailPath);
                UBPlatformUtils::hideFile(thumbnailPath);
            }
            else{
                thumbnailPath = ":images/libpalette/notFound.png";
            }
        }
    }

    return QPixmap(thumbnailPath);
}

UBFeature UBFeaturesController::newFolder( const QString &name )
{
	QString path = currentElement.getFullPath() + "/" + name;
	if(!QFileInfo(path).exists())
	{
        QDir().mkpath(path);
	}
	return UBFeature( currentElement.getUrl() + "/" + currentElement.getName(), QPixmap(":images/libpalette/folder.svg"), name, path, FEATURE_FOLDER );
}

void UBFeaturesController::addItemToPage(const UBFeature &item)
{
	if ( item.getType() == FEATURE_INTERNAL )
	{
		UBApplication::boardController->downloadURL( QUrl( item.getFullPath() ) );
	}
	else
	{
		UBApplication::boardController->downloadURL( QUrl::fromLocalFile( item.getFullPath() ) );
	}
}

UBFeature UBFeaturesController::moveItemToFolder( const QUrl &url, const UBFeature &destination )
{
	UBFeature newElement = copyItemToFolder( url, destination );
	deleteItem( url );
	return newElement;
}

UBFeature UBFeaturesController::copyItemToFolder( const QUrl &url, const UBFeature &destination )
{
	QString sourcePath = url.toLocalFile();

	Q_ASSERT( QFileInfo( sourcePath ).exists() );

	QString name = QFileInfo( sourcePath ).fileName();
	QString destPath = destination.getFullPath();
	QString destVirtualPath = destination.getUrl() + "/" + destination.getName();
	QString newFullPath = destPath + "/" + name;
	QFile( sourcePath ).copy( newFullPath );

	QPixmap thumb = thumbnailForFile( newFullPath );
	
    UBFeatureElementType type = FEATURE_ITEM;
	if ( UBFileSystemUtils::mimeTypeFromFileName( newFullPath ).contains("application") ) 
        type = FEATURE_INTERACTIVE;
	UBFeature newElement( destVirtualPath, thumb, name, newFullPath, type );
	return newElement;
}

void UBFeaturesController::deleteItem( const QUrl &url )
{
	QString path = url.toLocalFile();
	Q_ASSERT( QFileInfo( path ).exists() );

	QString thumbnailPath = UBFileSystemUtils::thumbnailPath( path );
    if (thumbnailPath.length() && QFileInfo( thumbnailPath ).exists()) 
	{
        QFile::remove(thumbnailPath);
    }
	QFile::remove( path );
}

bool UBFeaturesController::isTrash( const QUrl &url )
{
	return url.toLocalFile().startsWith( trashDirectoryPath );
}

UBFeaturesController::~UBFeaturesController()
{
}
