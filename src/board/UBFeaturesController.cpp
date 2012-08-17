#include <QGraphicsItem>
#include <QPointF>
#include <QtGui>

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

void UBFeaturesComputingThread::scanFS(const QUrl & currentPath, const QString & currVirtualPath, const QSet<QUrl> &pFavoriteSet)
{
    
    Q_ASSERT(QFileInfo(currentPath.toLocalFile()).exists());

    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(currentPath.toLocalFile());

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1) {
        if (abort) {
            return;
        }

        QString fullFileName = fileInfo->absoluteFilePath();
        UBFeatureElementType featureType = UBFeaturesController::fileTypeFromUrl(fullFileName);
        QString fileName = fileInfo->fileName();

        QImage icon = UBFeaturesController::getIcon(fullFileName, featureType);

        if ( fullFileName.contains(".thumbnail."))
            continue;

        UBFeature testFeature(currVirtualPath, icon, fileName, QUrl::fromLocalFile(fullFileName), featureType);

        emit sendFeature(testFeature);
        emit featureSent();
        emit scanPath(fullFileName);

        if ( pFavoriteSet.find(QUrl::fromLocalFile(fullFileName)) != pFavoriteSet.end()) {
            //TODO send favoritePath from the controller or make favoritePath public and static
            emit sendFeature(UBFeature( "/root/Favorites", icon, fileName, QUrl::fromLocalFile(fullFileName), featureType));
        }

        if (featureType == FEATURE_FOLDER) {
            scanFS(QUrl::fromLocalFile(fullFileName), currVirtualPath + "/" + fileName, pFavoriteSet);
        }
    }
}

void UBFeaturesComputingThread::scanAll(QList<QPair<QUrl, QString> > pScanningData, const QSet<QUrl> &pFavoriteSet)
{
    for (int i = 0; i < pScanningData.count(); i++) {
        if (abort) {
            return;
        }
        QPair<QUrl, QString> curPair = pScanningData.at(i);

        emit scanCategory(UBFeaturesController::categoryNameForVirtualPath(curPair.second));
        scanFS(curPair.first, curPair.second, pFavoriteSet);
    }
}

int UBFeaturesComputingThread::featuresCount(const QUrl &pPath)
{
    int noItems = 0;

    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(pPath.toLocalFile());

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1) {
        QString fullFileName = fileInfo->absoluteFilePath();
        UBFeatureElementType featureType = UBFeaturesController::fileTypeFromUrl(fullFileName);

        if (featureType != FEATURE_INVALID && !fullFileName.contains(".thumbnail.")) {
            noItems++;
        }

        if (featureType == FEATURE_FOLDER) {
            noItems += featuresCount(QUrl::fromLocalFile(fullFileName));
        }
    }

    return noItems;
}

int UBFeaturesComputingThread::featuresCountAll(QList<QPair<QUrl, QString> > pScanningData)
{
    int noItems = 0;
    for (int i = 0; i < pScanningData.count(); i++) {
        QPair<QUrl, QString> curPair = pScanningData.at(i);
        noItems += featuresCount(curPair.first);
    }

    return noItems;
}

UBFeaturesComputingThread::UBFeaturesComputingThread(QObject *parent) :
QThread(parent)
{
    restart = false;
    abort = false;
}

void UBFeaturesComputingThread::compute(const QList<QPair<QUrl, QString> > &pScanningData, QSet<QUrl> *pFavoritesSet)
{
    QMutexLocker curLocker(&mMutex);

    mScanningData = pScanningData;
    mFavoriteSet = *pFavoritesSet;

    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        mWaitCondition.wakeOne();
    }
}

void UBFeaturesComputingThread::run()
{
    forever {
        qDebug() << "Custom thread started execution";

        mMutex.lock();
        QList<QPair<QUrl, QString> > searchData = mScanningData;
        QSet<QUrl> favoriteSet = mFavoriteSet;
        mMutex.unlock();

        if (abort) {
            return;
        }
        if (restart) {
            break;
        }

        QTime curTime = QTime::currentTime();
        int fsCnt = featuresCountAll(searchData);
        int msecsto = curTime.msecsTo(QTime::currentTime());
        qDebug() << "time on evaluation" << msecsto;

        emit maxFilesCountEvaluated(fsCnt);

        emit scanStarted();
        curTime = QTime::currentTime();
        scanAll(searchData, favoriteSet);
        qDebug() << "Time on finishing" << curTime.msecsTo(QTime::currentTime());
        emit scanFinished();

        mMutex.lock();
        if (!restart) {
            mWaitCondition.wait(&mMutex);
        }
        restart = false;
        mMutex.unlock();

    }
}

UBFeaturesComputingThread::~UBFeaturesComputingThread()
{
    qDebug() <<  "thread destructor catched";

    mMutex.lock();
    abort = true;
    mWaitCondition.wakeOne();
    mMutex.unlock();

    wait();
}

UBFeature::UBFeature(const QString &url, const QImage &icon, const QString &name, const QUrl &realPath, UBFeatureElementType type)
: virtualDir(url), mThumbnail(icon), mName(name), mPath(realPath), elementType(type)
{
}

UBFeature::~UBFeature()
{
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

bool UBFeature::isDeletable() const
{
    return elementType == FEATURE_ITEM
            || elementType == FEATURE_AUDIO
            || elementType == FEATURE_VIDEO
            || elementType == FEATURE_IMAGE
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
    webSearchPath = rootPath + "/Web search";

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

    rootElement = UBFeature(QString(), QImage( ":images/libpalette/home.png" ), "root", QUrl());
    audiosElement = UBFeature( rootPath, QImage(":images/libpalette/AudiosCategory.svg"), "Audios" , mUserAudioDirectoryPath, FEATURE_CATEGORY);
    moviesElement = UBFeature( rootPath, QImage(":images/libpalette/MoviesCategory.svg"), "Movies" , mUserVideoDirectoryPath, FEATURE_CATEGORY);
    picturesElement = UBFeature( rootPath, QImage(":images/libpalette/PicturesCategory.svg"), "Pictures" , mUserPicturesDirectoryPath, FEATURE_CATEGORY);
    flashElement = UBFeature( rootPath, QImage(":images/libpalette/FlashCategory.svg"), "Animations" , mUserAnimationDirectoryPath, FEATURE_CATEGORY);
    interactElement = UBFeature( rootPath, QImage(":images/libpalette/InteractivesCategory.svg"), "Interactivities" ,  mLibInteractiveDirectoryPath, FEATURE_CATEGORY);
    applicationsElement = UBFeature( rootPath, QImage(":images/libpalette/ApplicationsCategory.svg"), "Applications" , mUserInteractiveDirectoryPath, FEATURE_CATEGORY);
    shapesElement = UBFeature( rootPath, QImage(":images/libpalette/ShapesCategory.svg"), "Shapes" , mLibShapesDirectoryPath, FEATURE_CATEGORY );
    favoriteElement = UBFeature( rootPath, QImage(":images/libpalette/FavoritesCategory.svg"), "Favorites", QUrl("favorites"), FEATURE_FAVORITE );
    webSearchElement = UBFeature( rootPath, QImage(":images/libpalette/WebSearchCategory.svg"), "Web search", mLibSearchDirectoryPath, FEATURE_CATEGORY);

    trashElement = UBFeature( rootPath, QImage(":images/libpalette/TrashCategory.svg"), "Trash", trashDirectoryPath, FEATURE_TRASH );

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
    connect(&mCThread, SIGNAL(sendFeature(UBFeature)), featuresModel, SLOT(addItem(UBFeature)));
    connect(&mCThread, SIGNAL(featureSent()), this, SIGNAL(featureAddedFromThread()));
    connect(&mCThread, SIGNAL(scanStarted()), this, SIGNAL(scanStarted()));
    connect(&mCThread, SIGNAL(scanFinished()), this, SIGNAL(scanFinished()));
    connect(&mCThread, SIGNAL(maxFilesCountEvaluated(int)), this, SIGNAL(maxFilesCountEvaluated(int)));
    connect(&mCThread, SIGNAL(scanCategory(QString)), this, SIGNAL(scanCategory(QString)));
    connect(&mCThread, SIGNAL(scanPath(QString)), this, SIGNAL(scanPath(QString)));
    connect(UBApplication::boardController, SIGNAL(npapiWidgetCreated(QString)), this, SLOT(createNpApiFeature(QString)));

    QTimer::singleShot(0, this, SLOT(startThread()));
}

void UBFeaturesController::startThread()
{
    QList<QPair<QUrl, QString> > computingData;

    computingData << QPair<QUrl, QString>(mLibAudiosDirectoryPath, audiosPath)
            <<  QPair<QUrl, QString>(mLibVideosDirectoryPath, moviesPath)
            <<  QPair<QUrl, QString>(mLibAnimationsDirectoryPath, flashPath)
            <<  QPair<QUrl, QString>(mLibPicturesDirectoryPath, picturesPath)

            <<  QPair<QUrl, QString>(mUserInteractiveDirectoryPath, appPath)
            <<  QPair<QUrl, QString>(mUserAudioDirectoryPath, audiosPath)
            <<  QPair<QUrl, QString>(mUserPicturesDirectoryPath, picturesPath)
            <<  QPair<QUrl, QString>(mUserVideoDirectoryPath, moviesPath)
            <<  QPair<QUrl, QString>(mUserAnimationDirectoryPath, flashPath)

            <<  QPair<QUrl, QString>(mLibApplicationsDirectoryPath, appPath)
            <<  QPair<QUrl, QString>(mLibShapesDirectoryPath, shapesPath)
            <<  QPair<QUrl, QString>(mLibInteractiveDirectoryPath, interactPath)
            <<  QPair<QUrl, QString>(trashDirectoryPath, trashPath)
            <<  QPair<QUrl, QString>(mLibSearchDirectoryPath, rootPath + "/" + "Web search");

    mCThread.compute(computingData, favoriteSet);
}

void UBFeaturesController::createNpApiFeature(const QString &str)
{
    Q_ASSERT(QFileInfo(str).exists() && QFileInfo(str).isDir());

    QString widgetName = QFileInfo(str).fileName();

    featuresModel->addItem(UBFeature(QString(appPath+"/Web"), QImage(UBGraphicsWidgetItem::iconFilePath(QUrl::fromLocalFile(str))), widgetName, QUrl::fromLocalFile(str), FEATURE_INTERACTIVE));
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

    //filling favoriteList
    loadFavoriteList();

    QList <UBToolsManager::UBToolDescriptor> tools = UBToolsManager::manager()->allTools();

    foreach (UBToolsManager::UBToolDescriptor tool, tools) {
        featuresList->append(UBFeature(appPath, tool.icon.toImage(), tool.label, QUrl(tool.id), FEATURE_INTERNAL));
        if (favoriteSet->find(QUrl(tool.id)) != favoriteSet->end()) {
            featuresList->append(UBFeature(favoritePath, tool.icon.toImage(), tool.label, QUrl(tool.id), FEATURE_INTERNAL));
        }
    }
}
void UBFeaturesController::fileSystemScan(const QUrl & currentPath, const QString & currVirtualPath)
{
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(currentPath.toLocalFile());

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1) {
        QString fullFileName = fileInfo->absoluteFilePath();
        UBFeatureElementType featureType = fileTypeFromUrl(fullFileName);
        QString fileName = fileInfo->fileName();

        QImage icon = getIcon(fullFileName, featureType);

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

int UBFeaturesController::featuresCount(const QUrl &currPath)
{
    int noItems = 0;

    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(currPath.toLocalFile());

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1) {
        QString fullFileName = fileInfo->absoluteFilePath();
        UBFeatureElementType featureType = fileTypeFromUrl(fullFileName);

        if (featureType != FEATURE_INVALID && !fullFileName.contains(".thumbnail.")) {
            noItems++;
        } else {
            continue;
        }

        if (featureType == FEATURE_FOLDER) {
            noItems += featuresCount(QUrl::fromLocalFile(fullFileName));
        }
    }

    return noItems;
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
        UBFeatureElementType type = fileTypeFromUrl(fileInfo.absoluteFilePath());
        UBFeature elem(favoritePath, getIcon(filePath, type), fileName, path, fileTypeFromUrl(filePath) );
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


UBFeatureElementType UBFeaturesController::fileTypeFromUrl(const QString &path)
{
    QFileInfo fileInfo(path);

    if (!fileInfo.exists()) {
        return FEATURE_INVALID;
    }

    QString fileName = fileInfo.fileName();
    QString mimeString = UBFileSystemUtils::mimeTypeFromFileName(fileName);

    UBFeatureElementType fileType = FEATURE_INVALID;

    if ( mimeString.contains("application")) {
        if (mimeString.contains("application/search")) {
            fileType = FEATURE_SEARCH;
        } else if (mimeString.contains("application/x-shockwave-flash")) {
            fileType = FEATURE_FLASH;
        } else {
            fileType = FEATURE_INTERACTIVE;
        }
    } else if ( path.contains("uniboardTool://")) {
		fileType = FEATURE_INTERNAL;
    } else if (mimeString.contains("audio")) {
        fileType = FEATURE_AUDIO;
    } else if (mimeString.contains("video")) {
        fileType = FEATURE_VIDEO;
    } else if (mimeString.contains("image")) {
        fileType = FEATURE_IMAGE;
    } else if (fileInfo.isDir()) {
        fileType = FEATURE_FOLDER;
    } else {
        fileType = FEATURE_INVALID;
    }

	return fileType;
}

QImage UBFeaturesController::getIcon(const QString &path, UBFeatureElementType pFType = FEATURE_INVALID)
{
    if (pFType == FEATURE_FOLDER) {
        return QImage(":images/libpalette/folder.svg");
    } else if (pFType == FEATURE_INTERACTIVE || pFType == FEATURE_SEARCH) {
        return QImage(UBGraphicsWidgetItem::iconFilePath(QUrl::fromLocalFile(path)));
    } else if (pFType == FEATURE_INTERNAL) {
        return QImage(UBToolsManager::manager()->iconFromToolId(path));
    } else if (pFType == FEATURE_FLASH) {
        return QImage(":images/libpalette/FlashIcon.svg");
    } else if (pFType == FEATURE_AUDIO) {
        return QImage(":images/libpalette/soundIcon.svg");
    } else if (pFType == FEATURE_VIDEO) {
        return QImage(":images/libpalette/movieIcon.svg");
    } else if (pFType == FEATURE_IMAGE) {
        QImage pix(path);
        if (pix.isNull()) {
            pix = QImage(":images/libpalette/notFound.png");
        } else {
            pix = pix.scaledToWidth(qMin(UBSettings::maxThumbnailWidth, pix.width()));
        }
        return pix;
    }

    return QImage(":images/libpalette/notFound.png");
}

bool UBFeaturesController::isDeletable( const QUrl &url )
{
    UBFeatureElementType type = fileTypeFromUrl(fileNameFromUrl(url));
    return type == FEATURE_AUDIO
            || type == FEATURE_VIDEO
            || type == FEATURE_IMAGE
            || type == FEATURE_ITEM;
}

QString UBFeaturesController::categoryNameForVirtualPath(const QString &str)
{
    QString result;
    int ind = str.lastIndexOf("/");
    if (ind != -1) {
        result = str.right(str.count() - ind - 1);
    }
    return result;
}

QImage UBFeaturesController::createThumbnail(const QString &path)
{
    QString thumbnailPath = path;
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName(path);

    if ( mimetype.contains("audio" )) {
        thumbnailPath = ":images/libpalette/soundIcon.svg";
    } else if ( mimetype.contains("video")) {
        thumbnailPath = ":images/libpalette/movieIcon.svg";
    } else {
        QImage pix(path);
        if (!pix.isNull()) {
            pix = pix.scaledToWidth(qMin(UBSettings::maxThumbnailWidth, pix.width()), Qt::SmoothTransformation);
            return pix;

        } else {
            thumbnailPath = ":images/libpalette/notFound.png";
        }

    }

    return QImage(thumbnailPath);
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

    QImage thumb = createThumbnail( filePath );
    UBFeature resultItem =  UBFeature( dest.getFullVirtualPath(), thumb, mFileName,
        QUrl::fromLocalFile( filePath ), FEATURE_ITEM );

    featuresModel->addItem(resultItem);

}

QStringList UBFeaturesController::getFileNamesInFolders()
{
    QStringList strList;

    Q_ASSERT(curListModel);

    for (int i = 0; i < curListModel->rowCount(QModelIndex()); i++) {
        QModelIndex ind = curListModel->index(i, 0);
        if (!ind.isValid()) {
            qDebug() << "incorrect model index catched";
            continue;
        }
        UBFeature curFeature = curListModel->data(ind, Qt::UserRole + 1).value<UBFeature>();
        if (curFeature.getType() == FEATURE_FOLDER) {
            strList << QFileInfo(curFeature.getFullPath().toLocalFile()).fileName();
        }
    }

    return strList;
}

void UBFeaturesController::addNewFolder(QString name)
{
    QString path = currentElement.getFullPath().toLocalFile() + "/" + name;

    if ("/root" == currentElement.getFullVirtualPath())
    {
        return;
    }

    if(!QFileInfo(path).exists()) {
        QDir().mkpath(path);
	}
    UBFeature newFeatureFolder = UBFeature( currentElement.getFullVirtualPath(), QImage(":images/libpalette/folder.svg"),
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

    QImage thumb = getIcon( newFullPath );
	
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

UBFeature UBFeaturesController::getFeature(const QModelIndex &index, const QString &listName)
{
//    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(pOnView->model());
    QAbstractItemModel *model = 0;
    if (listName == UBFeaturesWidget::objNamePathList) {
        model = featuresPathModel;
    } else if (listName == UBFeaturesWidget::objNameFeatureList) {
        model = curListModel;
    }

    if (model) {
        return model->data(index, Qt::UserRole + 1).value<UBFeature>();
    }

    return UBFeature();


//    return pOnView->model()->data(index, Qt::UserRole + 1).value<UBFeature>();  /*featuresSearchModel->data(index, Qt::UserRole + 1).value<UBFeature>()*/;
}

void UBFeaturesController::searchStarted(const QString &pattern, QListView *pOnView)
{
    if (pattern.isEmpty()) {

        pOnView->setModel(featuresProxyModel);
        featuresProxyModel->invalidate();
        curListModel = featuresProxyModel;
    } else if ( pattern.size() > 1 ) {

        //        featuresSearchModel->setFilterPrefix(currentElement.getFullVirtualPath());
        featuresSearchModel->setFilterWildcard( "*" + pattern + "*" );
        pOnView->setModel(featuresSearchModel );
        featuresSearchModel->invalidate();
        curListModel = featuresSearchModel;
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

    QImage thumb = getIcon(newFullPath);

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

    QImage thumb = getIcon(newFullPath, type);
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
    curListModel = featuresProxyModel;
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


