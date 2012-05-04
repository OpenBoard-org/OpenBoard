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

UBFeature::UBFeature(const QString &url, const QPixmap &icon, const QString &name, const QUrl &realPath, UBFeatureElementType type)
: virtualPath(url), mThumbnail(icon), mName(name), mPath(realPath), elementType(type)
{
	
}

QString UBFeature::getUrl() const
{
	if ( elementType == FEATURE_INTERNAL )
        return getFullPath().toString();
	/*if ( UBApplication::isFromWeb( getFullPath() ) )
		return QUrl( getFullPath() );*/
    return getFullPath().toLocalFile();
}

bool UBFeature::operator ==( const UBFeature &f )const
{
	return virtualPath == f.getVirtualPath() && mName == f.getName() && mPath == f.getFullPath() && elementType == f.getType();
}

bool UBFeature::operator !=( const UBFeature &f )const
{
	return !(*this == f);
}

bool UBFeature::isFolder() const
{
	return elementType == FEATURE_CATEGORY || elementType == FEATURE_TRASH || elementType == FEATURE_FAVORITE
		|| elementType == FEATURE_FOLDER;
}

bool UBFeature::isDeletable()const
{
	return elementType == FEATURE_ITEM;
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
    mUserAudioDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->userAudioDirectory() );
	mUserVideoDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->userVideoDirectory() );
    mUserPicturesDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->userImageDirectory() );
    mUserInteractiveDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->userInteractiveDirectory() );
    mUserAnimationDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->userAnimationDirectory() );

	mLibPicturesDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->applicationImageLibraryDirectory() );
	mLibInteractiveDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->applicationInteractivesDirectory() );
	mLibApplicationsDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->applicationApplicationsLibraryDirectory() );
	mLibShapesDirectoryPath = QUrl::fromLocalFile( UBSettings::settings()->applicationShapeLibraryDirectory() );
	mLibSearchDirectoryPath =QUrl::fromLocalFile(  UBSettings::settings()->userSearchDirectory() );
	trashDirectoryPath = QUrl::fromLocalFile( UBSettings::userTrashDirPath() );

	featuresList = new QList <UBFeature>();

	QList <UBToolsManager::UBToolDescriptor> tools = UBToolsManager::manager()->allTools();

	featuresList->append( UBFeature( QString(), QPixmap( ":images/libpalette/home.png" ), "root", QUrl() ) );
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

	audiosElement = UBFeature( rootPath, QPixmap(":images/libpalette/AudiosCategory.svg"), "Audios" , mUserAudioDirectoryPath );
	featuresList->append( audiosElement );
	moviesElement = UBFeature( rootPath, QPixmap(":images/libpalette/MoviesCategory.svg"), "Movies" , mUserVideoDirectoryPath );
	featuresList->append( moviesElement );
	picturesElement = UBFeature( rootPath, QPixmap(":images/libpalette/PicturesCategory.svg"), "Pictures" , mUserPicturesDirectoryPath );
	featuresList->append( picturesElement );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/ApplicationsCategory.svg"), "Applications" , mUserInteractiveDirectoryPath ) );
	flashElement = UBFeature( rootPath, QPixmap(":images/libpalette/FlashCategory.svg"), "Animations" , mUserAnimationDirectoryPath );
	featuresList->append( flashElement );
	interactElement = UBFeature( rootPath, QPixmap(":images/libpalette/InteractivesCategory.svg"), "Interactivities" ,  mLibInteractiveDirectoryPath );
	featuresList->append( interactElement );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/ShapesCategory.svg"), "Shapes" , mLibShapesDirectoryPath ) );
	trashElement = UBFeature( rootPath, QPixmap(":images/libpalette/TrashCategory.svg"), "Trash", trashDirectoryPath, FEATURE_TRASH );
	featuresList->append( trashElement );
	favoriteElement = UBFeature( rootPath, QPixmap(":images/libpalette/FavoritesCategory.svg"), "Favorites", QUrl("favorites"), FEATURE_FAVORITE );
	featuresList->append( favoriteElement );
	webSearchElement = UBFeature( rootPath, QPixmap(":images/libpalette/WebSearchCategory.svg"), "Web search", mLibSearchDirectoryPath );
	featuresList->append( webSearchElement );
	loadFavoriteList();

	foreach (UBToolsManager::UBToolDescriptor tool, tools)
	{
		featuresList->append( UBFeature( appPath, tool.icon, tool.label, QUrl( tool.id ), FEATURE_INTERNAL ) );
		if ( favoriteSet->find( QUrl( tool.id ) ) != favoriteSet->end() )
		{
			featuresList->append( UBFeature( favoritePath, tool.icon, tool.label, QUrl( tool.id ), FEATURE_INTERNAL ) );
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
	fileSystemScan( mLibSearchDirectoryPath, rootPath + "/" + "Web search" );
	

}

void UBFeaturesController::fileSystemScan(const QUrl & currentPath, const QString & currVirtualPath)
{
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(currentPath.toLocalFile());

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1)
	{
		UBFeatureElementType fileType = fileInfo->isDir() ? FEATURE_FOLDER : FEATURE_ITEM;

        QString fileName = fileInfo->fileName();
        if ( UBFileSystemUtils::mimeTypeFromFileName(fileName).contains("application") ) 
		{
			if ( UBFileSystemUtils::mimeTypeFromFileName(fileName).contains("application/search") )
			{
				fileType = FEATURE_SEARCH;
			}
			else
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
        featuresList->append( UBFeature( currVirtualPath, icon, fileName, QUrl::fromLocalFile( fullFileName ), fileType ) );
		if ( favoriteSet->find( QUrl::fromLocalFile( fullFileName ) ) != favoriteSet->end() )
		{
			featuresList->append( UBFeature( favoritePath, icon, fileName, QUrl::fromLocalFile( fullFileName ), fileType ) );
		}

		if ( fileType == FEATURE_FOLDER )
		{
			fileSystemScan( QUrl::fromLocalFile( fullFileName ), currVirtualPath + "/" + fileName );
		}

	}
}

void UBFeaturesController::loadFavoriteList()
{
	favoriteSet = new QSet<QUrl>();
	QFile file( UBSettings::userDataDirectory() + "/favorites.dat" );
	if ( file.exists() )
	{
		file.open(QIODevice::ReadOnly);
		QDataStream in(&file);
		int elementsNumber;
		in >> elementsNumber;
		for ( int i = 0; i < elementsNumber; ++i)
		{
			QUrl path;
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
	for ( QSet<QUrl>::iterator it = favoriteSet->begin(); it != favoriteSet->end(); ++it )
	{
		out << (*it);
	}
	file.close();
}

UBFeature UBFeaturesController::addToFavorite( const QUrl &path )
{
	QString filePath = fileNameFromUrl( path );
	if ( favoriteSet->find( path ) == favoriteSet->end() )
	{
		QFileInfo fileInfo( filePath );
		QString fileName = fileInfo.fileName();
		UBFeature elem( favoritePath, thumbnailForFile( filePath ), fileName, path, fileTypeFromUrl(filePath) );
		favoriteSet->insert( path );
		saveFavoriteList();
		return elem;
	}
	return UBFeature();
}

void UBFeaturesController::removeFromFavorite( const QUrl &path )
{
	QString filePath = fileNameFromUrl( path );
	if ( favoriteSet->find( path ) != favoriteSet->end() )
	{
		favoriteSet->erase( favoriteSet->find( path ) );
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

bool UBFeaturesController::isDeletable( const QUrl &url )
{
	UBFeatureElementType type = fileTypeFromUrl( fileNameFromUrl(url) );
	return type == FEATURE_ITEM;
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
    QString path = currentElement.getFullPath().toLocalFile() + "/" + name;
	if(!QFileInfo(path).exists())
	{
        QDir().mkpath(path);
	}
    return UBFeature( currentElement.getFullVirtualPath(), QPixmap(":images/libpalette/folder.svg"), 
        name, QUrl::fromLocalFile( path ), FEATURE_FOLDER );
}

void UBFeaturesController::addItemToPage(const UBFeature &item)
{
    UBApplication::boardController->downloadURL( item.getFullPath() );
}

void UBFeaturesController::addItemAsBackground(const UBFeature &item)
{
    UBApplication::boardController->downloadURL( item.getFullPath(), QPointF(), QSize(), true );
}


UBFeature UBFeaturesController::getDestinationForItem( const QUrl &url )
{
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName( fileNameFromUrl(url) );

    if ( mimetype.contains("audio") )
        return audiosElement;
    if ( mimetype.contains("video") )
        return moviesElement;
    else if ( mimetype.contains("image") )
        return picturesElement;
    else if ( mimetype.contains("application") )
	{
        if ( mimetype.contains( "x-shockwave-flash") )
            return flashElement;
        else
            return interactElement;
    }
    return UBFeature();
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

	UBFeature possibleDest = getDestinationForItem( url );

	UBFeature dest = destination;

	if ( destination != trashElement && 
		!destination.getFullVirtualPath().startsWith( possibleDest.getFullVirtualPath(), Qt::CaseInsensitive ) )
	{
		dest = possibleDest;
	}

	QString name = QFileInfo( sourcePath ).fileName();
    QString destPath = dest.getFullPath().toLocalFile();
	QString destVirtualPath = dest.getFullVirtualPath();
	QString newFullPath = destPath + "/" + name;
	QFile( sourcePath ).copy( newFullPath );

	QPixmap thumb = thumbnailForFile( newFullPath );
	
    UBFeatureElementType type = FEATURE_ITEM;
	if ( UBFileSystemUtils::mimeTypeFromFileName( newFullPath ).contains("application") ) 
        type = FEATURE_INTERACTIVE;
    UBFeature newElement( destVirtualPath, thumb, name, QUrl::fromLocalFile( newFullPath ), type );
	return newElement;
}

void UBFeaturesController::deleteItem( const QUrl &url )
{
	QString path = url.toLocalFile();
	Q_ASSERT( QFileInfo( path ).exists() );

	QString thumbnailPath = UBFileSystemUtils::thumbnailPath( path );
    if ( thumbnailPath.length() && QFileInfo( thumbnailPath ).exists() ) 
	{
        QFile::remove(thumbnailPath);
    }
	QFile::remove( path );
}

bool UBFeaturesController::isTrash( const QUrl &url )
{
    return url.toLocalFile().startsWith( trashDirectoryPath.toLocalFile() );
}

UBFeaturesController::~UBFeaturesController()
{
}
