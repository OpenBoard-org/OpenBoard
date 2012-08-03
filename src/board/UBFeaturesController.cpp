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
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsVideoItem.h"
#include "domain/UBGraphicsWidgetItem.h"

#include "gui/UBFeaturesWidget.h"

const QString UBFeaturesController::virtualRootName = "root";

UBFeature::UBFeature(const QString &url, const QPixmap &icon, const QString &name, const QUrl &realPath, UBFeatureElementType type)
: virtualDir(url), mThumbnail(icon), mName(name), mPath(realPath), elementType(type)
{
}

UBFeature::~UBFeature()
{
    for (int i = 0; i < mChildren.count(); i++) {
        delete mChildren[i];
    }

    for (int i = 0; i < mParents.count(); i++) {
        mParents[i]->mChildren.removeAll(this);
    }
}

QString UBFeature::getUrl() const
{
	if ( elementType == FEATURE_INTERNAL )
        return getFullPath().toString();

    return getFullPath().toLocalFile();
}

bool UBFeature::operator ==( const UBFeature &f )const
{
    return virtualDir == f.getVirtualPath() && mName == f.getName() && mPath == f.getFullPath() && elementType == f.getType();
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
    return elementType == FEATURE_ITEM
            || elementType == FEATURE_FOLDER;
}

bool UBFeature::inTrash() const
{
    return getFullPath().toLocalFile().startsWith(QUrl::fromLocalFile(UBSettings::userTrashDirPath()).toLocalFile() );
}

UBFeaturesController::UBFeaturesController(QWidget *pParentWidget) :
    QObject(pParentWidget)
    ,featuresList(0)
    ,mLastItemOffsetIndex(0)
{
    //Initializing virtual structure of the list
    rootPath = "/" + virtualRootName;

    appPath = rootPath + "/Applications";
    audiosPath = rootPath + "/Audios";
    moviesPath = rootPath + "/Movies";
    picturesPath = rootPath + "/Pictures";
    flashPath = rootPath + "/Animations";
    interactPath = rootPath + "/Interactivities";
    shapesPath = rootPath + "/Shapes";
    trashPath = rootPath + "/Trash";
    favoritePath = rootPath + "/Favorites";

    //Initializing physical directories from UBSettings
    mUserAudioDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->userAudioDirectory());
    mUserVideoDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->userVideoDirectory());
    mUserPicturesDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->userImageDirectory());
    mUserInteractiveDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->userInteractiveDirectory());
    mUserAnimationDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->userAnimationDirectory());

    mLibPicturesDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->applicationImageLibraryDirectory());
    mLibAudiosDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->applicationAudiosLibraryDirectory());
    mLibVideosDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->applicationVideosLibraryDirectory());
    mLibAnimationsDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->applicationAnimationsLibraryDirectory());
    mLibInteractiveDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->applicationInteractivesDirectory());
    mLibApplicationsDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->applicationApplicationsLibraryDirectory());
    mLibShapesDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->applicationShapeLibraryDirectory());
    mLibSearchDirectoryPath =QUrl::fromLocalFile(UBSettings::settings()->userSearchDirectory());
    trashDirectoryPath = QUrl::fromLocalFile(UBSettings::userTrashDirPath());

    rootElement = UBFeature(QString(), QPixmap( ":images/libpalette/home.png" ), "root", QUrl());
    audiosElement = UBFeature( rootPath, QPixmap(":images/libpalette/AudiosCategory.svg"), "Audios" , mUserAudioDirectoryPath, FEATURE_CATEGORY);
    moviesElement = UBFeature( rootPath, QPixmap(":images/libpalette/MoviesCategory.svg"), "Movies" , mUserVideoDirectoryPath, FEATURE_CATEGORY);
    picturesElement = UBFeature( rootPath, QPixmap(":images/libpalette/PicturesCategory.svg"), "Pictures" , mUserPicturesDirectoryPath, FEATURE_CATEGORY);
    flashElement = UBFeature( rootPath, QPixmap(":images/libpalette/FlashCategory.svg"), "Animations" , mUserAnimationDirectoryPath, FEATURE_CATEGORY);
    interactElement = UBFeature( rootPath, QPixmap(":images/libpalette/InteractivesCategory.svg"), "Interactivities" ,  mLibInteractiveDirectoryPath, FEATURE_CATEGORY);
    applicationsElement = UBFeature( rootPath, QPixmap(":images/libpalette/ApplicationsCategory.svg"), "Applications" , mUserInteractiveDirectoryPath, FEATURE_CATEGORY);
    shapesElement = UBFeature( rootPath, QPixmap(":images/libpalette/ShapesCategory.svg"), "Shapes" , mLibShapesDirectoryPath, FEATURE_CATEGORY );
    favoriteElement = UBFeature( rootPath, QPixmap(":images/libpalette/FavoritesCategory.svg"), "Favorites", QUrl("favorites"), FEATURE_FAVORITE );
    webSearchElement = UBFeature( rootPath, QPixmap(":images/libpalette/WebSearchCategory.svg"), "Web search", mLibSearchDirectoryPath, FEATURE_CATEGORY);

    trashElement = UBFeature( rootPath, QPixmap(":images/libpalette/TrashCategory.svg"), "Trash", trashDirectoryPath, FEATURE_TRASH );

    featuresList = new QList <UBFeature>();

    scanFS();

    featuresModel = new UBFeaturesModel(featuresList, this);
    featuresModel->setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);

    featuresProxyModel = new UBFeaturesProxyModel(this);
    featuresProxyModel->setFilterFixedString(rootPath);
    featuresProxyModel->setSourceModel(featuresModel);
    featuresProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

    featuresSearchModel = new UBFeaturesSearchProxyModel(this);
    featuresSearchModel->setSourceModel(featuresModel);
    featuresSearchModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

    featuresPathModel = new UBFeaturesPathProxyModel(this);
    featuresPathModel->setPath(rootPath);
    featuresPathModel->setSourceModel(featuresModel);

    connect(featuresModel, SIGNAL(dataRestructured()), featuresProxyModel, SLOT(invalidate()));

}

void UBFeaturesController::scanFS()
{
    featuresList->clear();
    featuresList->append(rootElement);

    *featuresList << audiosElement
                    << moviesElement
                    << picturesElement
                    << flashElement
                    << interactElement
                    << applicationsElement
                    << shapesElement
                    << favoriteElement
                    << webSearchElement
                    << trashElement;

    loadFavoriteList();

    QList <UBToolsManager::UBToolDescriptor> tools = UBToolsManager::manager()->allTools();

    foreach (UBToolsManager::UBToolDescriptor tool, tools) {
        featuresList->append(UBFeature(appPath, tool.icon, tool.label, QUrl(tool.id), FEATURE_INTERNAL));
        if (favoriteSet->find(QUrl(tool.id)) != favoriteSet->end()) {
            featuresList->append(UBFeature(favoritePath, tool.icon, tool.label, QUrl(tool.id), FEATURE_INTERNAL));
		}
	}
        //Claudio:
        // don't change the order of the scans
        fileSystemScan( mLibAudiosDirectoryPath, audiosPath);
        fileSystemScan( mLibVideosDirectoryPath, moviesPath);
        fileSystemScan( mLibAnimationsDirectoryPath, flashPath);
        fileSystemScan( mLibPicturesDirectoryPath, picturesPath  );

        fileSystemScan( mUserInteractiveDirectoryPath, appPath  );
        fileSystemScan( mUserAudioDirectoryPath, audiosPath  );
        fileSystemScan( mUserPicturesDirectoryPath, picturesPath  );
        fileSystemScan( mUserVideoDirectoryPath, moviesPath  );
        fileSystemScan( mUserAnimationDirectoryPath, flashPath  );

        fileSystemScan( mLibApplicationsDirectoryPath, appPath  );
        fileSystemScan( mLibShapesDirectoryPath, shapesPath  );
        fileSystemScan( mLibInteractiveDirectoryPath, interactPath  );
        fileSystemScan( trashDirectoryPath, trashPath );
        fileSystemScan( mLibSearchDirectoryPath, rootPath + "/" + "Web search" );
}

void UBFeaturesController::fileSystemScan(const QUrl & currentPath, const QString & currVirtualPath)
{
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(currentPath.toLocalFile());

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1) {
        QString fullFileName = fileInfo->absoluteFilePath();
        UBFeatureElementType featureType = fileTypeFromUrl(fullFileName);
        QString fileName = fileInfo->fileName();

        QPixmap icon(getIcon(fullFileName, featureType));

        if ( fullFileName.contains(".thumbnail."))
            continue;
 
        UBFeature testFeature(currVirtualPath, icon, fileName, QUrl::fromLocalFile(fullFileName), featureType);

        featuresList->append(testFeature);

        if ( favoriteSet->find( QUrl::fromLocalFile( fullFileName ) ) != favoriteSet->end() ) {
            featuresList->append( UBFeature( favoritePath, icon, fileName, QUrl::fromLocalFile( fullFileName ), featureType ) );
		}

        if (featureType == FEATURE_FOLDER) {
            fileSystemScan(QUrl::fromLocalFile(fullFileName), currVirtualPath + "/" + fileName);
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

void UBFeaturesController::addToFavorite( const QUrl &path )
{
	QString filePath = fileNameFromUrl( path );
	if ( favoriteSet->find( path ) == favoriteSet->end() )
	{
		QFileInfo fileInfo( filePath );
		QString fileName = fileInfo.fileName();
        UBFeature elem( favoritePath, getIcon( filePath, FEATURE_CATEGORY ), fileName, path, fileTypeFromUrl(filePath) );
		favoriteSet->insert( path );
		saveFavoriteList();

        if ( !elem.getVirtualPath().isEmpty() && !elem.getVirtualPath().isNull())
        featuresModel->addItem( elem );
	}
}

void UBFeaturesController::removeFromFavorite( const QUrl &path, bool deleteManualy)
{
//	QString filePath = fileNameFromUrl( path );
	if ( favoriteSet->find( path ) != favoriteSet->end() )
	{
		favoriteSet->erase( favoriteSet->find( path ) );
		saveFavoriteList();
	}

    if (deleteManualy) {
        featuresModel->deleteFavoriteItem(path.toString());
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
    QFileInfo fileInfo(path);

    if (!fileInfo.exists()) {
        return FEATURE_INVALID;
    }

    QString fileName = fileInfo.fileName();
    QString mimeString = UBFileSystemUtils::mimeTypeFromFileName(fileName);

    UBFeatureElementType fileType = fileInfo.isDir() ? FEATURE_FOLDER : FEATURE_ITEM;

    if ( mimeString.contains("application")) {
        if (mimeString.contains("application/search")) {
            fileType = FEATURE_SEARCH;
        } else {
            fileType = FEATURE_INTERACTIVE;
        }
    } else if ( path.contains("uniboardTool://")) {
		fileType = FEATURE_INTERNAL;
	}

	return fileType;
}

QPixmap UBFeaturesController::getIcon(const QString &path, UBFeatureElementType pFType = FEATURE_INVALID)
{
    if ( pFType == FEATURE_FOLDER )
    {
        return QPixmap(":images/libpalette/folder.svg");
    }
    else if ( pFType == FEATURE_INTERACTIVE )
    {
        return QPixmap( UBGraphicsWidgetItem::iconFilePath( QUrl::fromLocalFile(path) ) );
    }


    if ( path.contains("uniboardTool://") )
    {
            return QPixmap( UBToolsManager::manager()->iconFromToolId(path) );
    }
    if ( UBFileSystemUtils::mimeTypeFromFileName(path).contains("application") )
    {
        return QPixmap( UBGraphicsWidgetItem::iconFilePath( QUrl::fromLocalFile(path) ) );
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

void UBFeaturesController::importImage(const QImage &image, const QString &fileName)
{
    importImage(image, currentElement, fileName);
}

void UBFeaturesController::importImage( const QImage &image, const UBFeature &destination, const QString &fileName )
{
    QString mFileName = fileName;
    if (mFileName.isNull()) {
        QDateTime now = QDateTime::currentDateTime();
        mFileName  = tr("ImportedImage") + "-" + now.toString("dd-MM-yyyy hh-mm-ss") + ".png";
    }

    UBFeature dest = destination;

    if ( !destination.getFullVirtualPath().startsWith( picturesElement.getFullVirtualPath(), Qt::CaseInsensitive ) )
    {
	    dest = picturesElement;
    }

    QString filePath = dest.getFullPath().toLocalFile() + "/" + mFileName;
    image.save(filePath);

    QPixmap thumb = createThumbnail( filePath );
    UBFeature resultItem =  UBFeature( dest.getFullVirtualPath(), thumb, mFileName,
        QUrl::fromLocalFile( filePath ), FEATURE_ITEM );

    featuresModel->addItem(resultItem);

}

void UBFeaturesController::addNewFolder(const QString &name)
{
    QString path = currentElement.getFullPath().toLocalFile() + "/" + name;

    if ("/root" == currentElement.getFullVirtualPath())
    {
        return;
    }

    if(!QFileInfo(path).exists()) {
        QDir().mkpath(path);
	}
    UBFeature newFeatureFolder = UBFeature( currentElement.getFullVirtualPath(), QPixmap(":images/libpalette/folder.svg"),
                                            name, QUrl::fromLocalFile( path ), FEATURE_FOLDER );

    featuresModel->addItem( newFeatureFolder );
    featuresProxyModel->invalidate();
}

void UBFeaturesController::addItemToPage(const UBFeature &item)
{
    UBApplication::boardController->downloadURL( item.getFullPath() );
}

void UBFeaturesController::addItemAsBackground(const UBFeature &item)
{
    UBApplication::boardController->downloadURL( item.getFullPath(), QPointF(), QSize(), true );
}

UBFeature UBFeaturesController::getDestinationFeatureForUrl( const QUrl &url )
{
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName( url.toString() );

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

void UBFeaturesController::addDownloadedFile(const QUrl &sourceUrl, const QByteArray &pData)
{
    UBFeature dest = getDestinationFeatureForUrl( sourceUrl );

    if ( dest == UBFeature() )
        return;

    QString fileName = QFileInfo( sourceUrl.toString() ).fileName();
    QString filePath = dest.getFullPath().toLocalFile() + "/" + fileName;

    QFile file( filePath );
    if ( file.open(QIODevice::WriteOnly ))
    {
        file.write(pData);
        file.close();

        UBFeature downloadedFeature = UBFeature( dest.getFullVirtualPath(), getIcon( filePath ),
                                                 fileName, QUrl::fromLocalFile(filePath), FEATURE_ITEM);
        if (downloadedFeature != UBFeature()) {
            featuresModel->addItem(downloadedFeature);
        }
    }

}

UBFeature UBFeaturesController::moveItemToFolder( const QUrl &url, const UBFeature &destination )
{
	/*UBFeature newElement = copyItemToFolder( url, destination );
	deleteItem( url );
	return newElement;*/
    QString sourcePath = url.toLocalFile();

	Q_ASSERT( QFileInfo( sourcePath ).exists() );

    UBFeature possibleDest = getDestinationFeatureForUrl( url );

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
    if ( sourcePath.compare( newFullPath, Qt::CaseInsensitive ) )
    {
	    QFile( sourcePath ).copy( newFullPath );
        deleteItem( url );
    }

    QPixmap thumb = getIcon( newFullPath );
	
    UBFeatureElementType type = FEATURE_ITEM;
	if ( UBFileSystemUtils::mimeTypeFromFileName( newFullPath ).contains("application") ) 
        type = FEATURE_INTERACTIVE;
    UBFeature newElement( destVirtualPath, thumb, name, QUrl::fromLocalFile( newFullPath ), type );
    return newElement;
}

void UBFeaturesController::rescanModel()
{
    featuresModel->removeRows(0, featuresList->count());

    //Could implement infolder scanning for better perfomance
    scanFS();
    refreshModels();
}

void UBFeaturesController::siftElements(const QString &pSiftValue)
{
    featuresProxyModel->setFilterFixedString(pSiftValue);
    featuresProxyModel->invalidate();

    featuresPathModel->setPath(pSiftValue);
    featuresPathModel->invalidate();
}

UBFeature UBFeaturesController::getFeature(const QModelIndex &index, QListView *pOnView)
{
    return qobject_cast<QSortFilterProxyModel *>(pOnView->model())->data(index, Qt::UserRole + 1).value<UBFeature>();
}

void UBFeaturesController::searchStarted(const QString &pattern, QListView *pOnView)
{
    if (pattern.isEmpty()) {

        pOnView->setModel(featuresProxyModel);
        featuresProxyModel->invalidate();
    } else if ( pattern.size() > 2 ) {

        featuresSearchModel->setFilterWildcard( "*" + pattern + "*" );
        pOnView->setModel(featuresSearchModel );
        featuresSearchModel->invalidate();
    }
}

void UBFeaturesController::refreshModels()
{
    featuresProxyModel->invalidate();
    featuresSearchModel->invalidate();
    featuresPathModel->invalidate();
}

UBFeature UBFeaturesController::copyItemToFolder( const QUrl &url, const UBFeature &destination )
{
    QString sourcePath = url.toLocalFile();

    Q_ASSERT( QFileInfo( sourcePath ).exists() );

    UBFeature possibleDest = getDestinationFeatureForUrl( url );

    UBFeature dest = destination;

    if ( destination != trashElement &&
        !destination.getFullVirtualPath().startsWith( possibleDest.getFullVirtualPath(), Qt::CaseInsensitive ) )
    {
        dest = possibleDest;
    }

    QString name = QFileInfo(sourcePath).fileName();
    QString destPath = dest.getFullPath().toLocalFile();
    QString destVirtualPath = dest.getFullVirtualPath();
    QString newFullPath = destPath + "/" + name;
    if (!sourcePath.compare(newFullPath, Qt::CaseInsensitive))
        QFile(sourcePath).copy(newFullPath);

    QPixmap thumb = getIcon(newFullPath);

    UBFeatureElementType type = FEATURE_ITEM;
    if (UBFileSystemUtils::mimeTypeFromFileName(newFullPath).contains("application"))
        type = FEATURE_INTERACTIVE;
    UBFeature newElement( destVirtualPath, thumb, name, QUrl::fromLocalFile( newFullPath ), type );
    return newElement;
}

void UBFeaturesController::moveExternalData(const QUrl &url, const UBFeature &destination)
{
    QString sourcePath = url.toLocalFile();

    Q_ASSERT( QFileInfo( sourcePath ).exists() );

    UBFeature possibleDest = getDestinationFeatureForUrl(url);

    UBFeature dest = destination;

    if ( destination != trashElement &&
        !destination.getFullVirtualPath().startsWith( possibleDest.getFullVirtualPath(), Qt::CaseInsensitive ) )
    {
        dest = possibleDest;
    }

    UBFeatureElementType type = fileTypeFromUrl(sourcePath);

    if (type == FEATURE_FOLDER) {
        return;
    }

    QString name = QFileInfo(sourcePath).fileName();
    QString destPath = dest.getFullPath().toLocalFile();
    QString destVirtualPath = dest.getFullVirtualPath();
    QString newFullPath = destPath + "/" + name;

    if (!sourcePath.compare(newFullPath, Qt::CaseInsensitive) || !UBFileSystemUtils::copy(sourcePath, newFullPath)) {
        return;
    }

    Q_ASSERT(QFileInfo(newFullPath).exists());

    QPixmap thumb = getIcon(newFullPath, type);
    UBFeature newElement(destVirtualPath, thumb, name, QUrl::fromLocalFile(newFullPath), type);

    featuresModel->addItem(newElement);
}

void UBFeaturesController::deleteItem(const QUrl &url)
{
    QString path = url.toLocalFile();
    Q_ASSERT( QFileInfo( path ).exists() );

	QString thumbnailPath = UBFileSystemUtils::thumbnailPath( path );
    if ( thumbnailPath.length() && QFileInfo( thumbnailPath ).exists()) {
        if (QFileInfo(thumbnailPath).isFile()) {
            QFile::remove(thumbnailPath);
        } else if (QFileInfo(thumbnailPath).isDir()){
            if (!UBFileSystemUtils::deleteDir(thumbnailPath)) {
                qDebug() << "Not able to delete directory";
             }
        }
    }
    if (QFileInfo(path).isFile()) {
        QFile::remove( path );

    } else if (QFileInfo(path).isDir()) {
        UBFileSystemUtils::deleteDir(path);
    }
}

void UBFeaturesController::deleteItem(const UBFeature &pFeature)
{
    QUrl sourceUrl = pFeature.getFullPath();
    featuresModel->deleteItem(pFeature);
    deleteItem(sourceUrl);
}

bool UBFeaturesController::isTrash( const QUrl &url )
{
    return url.toLocalFile().startsWith( trashDirectoryPath.toLocalFile() );
}
void UBFeaturesController::moveToTrash(UBFeature feature, bool deleteManualy)
{
    featuresModel->moveData(feature, trashElement, Qt::MoveAction, deleteManualy);
    removeFromFavorite(feature.getFullPath());
    featuresModel->deleteFavoriteItem(UBFeaturesController::fileNameFromUrl(feature.getFullPath()));
}

UBFeaturesController::~UBFeaturesController()
{
    if (featuresList) {
        delete featuresList;
    }
}

void UBFeaturesController::assignFeaturesListVeiw(UBFeaturesListView *pList)
{
    pList->setDragDropMode( QAbstractItemView::DragDrop );
    pList->setSelectionMode( QAbstractItemView::ContiguousSelection );

    pList->setResizeMode( QListView::Adjust );
    pList->setViewMode( QListView::IconMode );

    pList->setIconSize(QSize(UBFeaturesWidget::defaultThumbnailSize, UBFeaturesWidget::defaultThumbnailSize));
    pList->setGridSize(QSize(UBFeaturesWidget::defaultThumbnailSize + 20, UBFeaturesWidget::defaultThumbnailSize + 20));

    itemDelegate = new UBFeaturesItemDelegate(this, pList);
    pList->setItemDelegate(itemDelegate);

    pList->setModel(featuresProxyModel);
}

void UBFeaturesController::assignPathListView(UBFeaturesListView *pList)
{
    pList->setViewMode(QListView::IconMode );
    pList->setIconSize(QSize(UBFeaturesWidget::defaultThumbnailSize - 10, UBFeaturesWidget::defaultThumbnailSize - 10));
    pList->setGridSize(QSize(UBFeaturesWidget::defaultThumbnailSize + 10, UBFeaturesWidget::defaultThumbnailSize - 10));
    pList->setFixedHeight(60);
    pList->setSelectionMode(QAbstractItemView::NoSelection);
    pList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    pList->setFlow(QListView::LeftToRight);
    pList->setWrapping(false);
    pList->setDragDropMode(QAbstractItemView::DropOnly);

    pList->setModel( featuresPathModel);

    pathItemDelegate = new UBFeaturesPathItemDelegate(this);
    pList->setItemDelegate(pathItemDelegate);
}


