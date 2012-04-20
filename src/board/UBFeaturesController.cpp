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

UBFeature::UBFeature(const UBFeature &f)
{
	virtualPath = f.getUrl();
	mPath = f.getFullPath();
	mThumbnail = f.getThumbnail();
	mName = f.getName();
	elementType = f.getType();
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

	featuresList = new QVector <UBFeature>();

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

	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/AudiosCategory.svg"), "Audios" , mUserAudioDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/MoviesCategory.svg"), "Movies" , mUserVideoDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/PicturesCategory.svg"), "Pictures" , mUserPicturesDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/ApplicationsCategory.svg"), "Applications" , mUserInteractiveDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/FlashCategory.svg"), "Animations" , mUserAnimationDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/InteractivesCategory.svg"), "Interactivities" ,  mLibInteractiveDirectoryPath ) );
	featuresList->append( UBFeature( rootPath, QPixmap(":images/libpalette/ShapesCategory.svg"), "Shapes" , mLibShapesDirectoryPath ) );

	foreach (UBToolsManager::UBToolDescriptor tool, tools)
	{
		featuresList->append( UBFeature( appPath, tool.icon, tool.label, tool.id, FEATURE_INTERNAL ) );
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

		if ( fileType == FEATURE_FOLDER )
		{
			fileSystemScan( fullFileName, currVirtualPath + "/" + fileName );
		}

	}
}

QPixmap UBFeaturesController::thumbnailForFile(const QString &path)
{
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
	UBApplication::boardController->downloadURL( QUrl::fromLocalFile( item.getFullPath() ) );
}

UBFeature UBFeaturesController::moveItemToFolder( const QUrl &url, const UBFeature &destination )
{
	UBFeature newElement = copyItemToFolder( url, destination );
	QString sourcePath = url.toLocalFile();
	QFile::remove( sourcePath );
	QString thumbnailPath = UBFileSystemUtils::thumbnailPath( sourcePath );
    if (thumbnailPath.length() && QFileInfo( thumbnailPath ).exists()) 
	{
        QFile::remove(thumbnailPath);
    }
	return newElement;
	/*QString sourcePath = url.toLocalFile();

	Q_ASSERT( QFileInfo( sourcePath ).exists() );

	QString name = QFileInfo( sourcePath ).fileName();
	QString destPath = destination.getFullPath();
	QString destVirtualPath = destination.getUrl() + "/" + destination.getName();
	QString newFullPath = destPath + "/" + name;
	QFile( sourcePath ).copy( newFullPath );
	QFile::remove( sourcePath );

	QString thumbnailPath = UBFileSystemUtils::thumbnailPath( sourcePath );
    if (thumbnailPath.length() && QFileInfo( thumbnailPath ).exists()) 
	{
        QFile::remove(thumbnailPath);
    }

	QPixmap thumb = thumbnailForFile( newFullPath );
	
	UBFeatureElementType type = UBFeatureElementType::FEATURE_ITEM;
	if ( UBFileSystemUtils::mimeTypeFromFileName( newFullPath ).contains("application") ) 
        type = UBFeatureElementType::FEATURE_INTERACTIVE;
	UBFeature newElement( destVirtualPath, thumb, name, destPath, type );
	return newElement;*/
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

/*
void UBFeaturesController::addImageToCurrentPage( const QString &path )
{
	QPointF pos = UBApplication::boardController->activeScene()->normalizedSceneRect().center();
	mLastItemOffsetIndex = qMin(mLastItemOffsetIndex, 5);

    QGraphicsItem* itemInScene = 0;

    if ( UBApplication::boardController->activeScene() ) 
	{
        QString mimeType = UBFileSystemUtils::mimeTypeFromFileName( path );

        pos = QPointF( pos.x() + 50 * mLastItemOffsetIndex, pos.y() + 50 * mLastItemOffsetIndex );
        mLastItemOffsetIndex++;
        //TODO UB 4.x move this logic to the scene ..
        if (mimeType == "image/svg+xml") 
		{
			itemInScene = UBApplication::boardController->activeScene()->addSvg( QUrl::fromLocalFile(path), pos );
        } 
		else 
		{
            itemInScene = UBApplication::boardController->activeScene()->addPixmap( QPixmap(path), pos );
        }
    }

    if (itemInScene) 
	{
        itemInScene = UBApplication::boardController->activeScene()->scaleToFitDocumentSize(itemInScene, false, UBSettings::objectInControlViewMargin);
    }
}
*/

UBFeaturesController::~UBFeaturesController()
{
}
