/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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
#include "domain/UBGraphicsWidgetItem.h"

#include "gui/UBFeaturesWidget.h"

const QString UBFeaturesController::virtualRootName = "root";
const QString UBFeaturesController::rootPath  = "/" + virtualRootName;

const QString UBFeaturesController::appPath = rootPath + "/Applications";
const QString UBFeaturesController::audiosPath = rootPath + "/Audios";
const QString UBFeaturesController::moviesPath = rootPath + "/Movies";
const QString UBFeaturesController::picturesPath = rootPath + "/Pictures";
const QString UBFeaturesController::flashPath = rootPath + "/Animations";
const QString UBFeaturesController::interactPath = rootPath + "/Interactivities";
const QString UBFeaturesController::shapesPath = rootPath + "/Shapes";
const QString UBFeaturesController::trashPath = rootPath + "/Trash";
const QString UBFeaturesController::favoritePath = rootPath + "/Favorites";
const QString UBFeaturesController::webSearchPath = rootPath + "/Web search";


void UBFeaturesComputingThread::scanFS(const QUrl & currentPath, const QString & currVirtualPath, const QSet<QUrl> &pFavoriteSet)
{
//    Q_ASSERT(QFileInfo(currentPath.toLocalFile()).exists());
//    if(QFileInfo(currentPath.toLocalFile()).exists())
//        return;

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

        UBFeature testFeature(currVirtualPath + "/" + fileName, icon, fileName, QUrl::fromLocalFile(fullFileName), featureType);

        emit sendFeature(testFeature);
        emit featureSent();
        emit scanPath(fullFileName);

        if ( pFavoriteSet.find(QUrl::fromLocalFile(fullFileName)) != pFavoriteSet.end()) {
            //TODO send favoritePath from the controller or make favoritePath public and static
            emit sendFeature(UBFeature( UBFeaturesController::favoritePath + "/" + fileName, icon, fileName, QUrl::fromLocalFile(fullFileName), featureType));
        }

        if (featureType == FEATURE_FOLDER) {
            scanFS(QUrl::fromLocalFile(fullFileName), currVirtualPath + "/" + fileName, pFavoriteSet);
        }
    }
}

void UBFeaturesComputingThread::scanAll(QList<QPair<QUrl, UBFeature> > pScanningData, const QSet<QUrl> &pFavoriteSet)
{
    for (int i = 0; i < pScanningData.count(); i++) {
        if (abort) {
            return;
        }
        QPair<QUrl, UBFeature> curPair = pScanningData.at(i);

        emit scanCategory(curPair.second.getDisplayName());
        scanFS(curPair.first, curPair.second.getFullVirtualPath(), pFavoriteSet);
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

int UBFeaturesComputingThread::featuresCountAll(QList<QPair<QUrl, UBFeature> > pScanningData)
{
    int noItems = 0;
    for (int i = 0; i < pScanningData.count(); i++) {
        QPair<QUrl, UBFeature> curPair = pScanningData.at(i);
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

void UBFeaturesComputingThread::compute(const QList<QPair<QUrl, UBFeature> > &pScanningData, QSet<QUrl> *pFavoritesSet)
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
//        qDebug() << "Custom thread started execution";

        mMutex.lock();
        QList<QPair<QUrl, UBFeature> > searchData = mScanningData;
        QSet<QUrl> favoriteSet = mFavoriteSet;
        mMutex.unlock();

        if (abort) {
            return;
        }
        if (restart) {
            break;
        }

//        QTime curTime = QTime::currentTime();
        int fsCnt = featuresCountAll(searchData);
//        int msecsto = curTime.msecsTo(QTime::currentTime());
//        qDebug() << "time on evaluation" << msecsto;

        emit maxFilesCountEvaluated(fsCnt);

        emit scanStarted();
//        curTime = QTime::currentTime();
        scanAll(searchData, favoriteSet);
//        qDebug() << "Time on finishing" << curTime.msecsTo(QTime::currentTime());
        emit scanFinished();

        mMutex.lock();
        if (!abort) {
            mWaitCondition.wait(&mMutex);
        }
        restart = false;
        mMutex.unlock();

    }
}

UBFeaturesComputingThread::~UBFeaturesComputingThread()
{
//    qDebug() <<  "thread destructor catched";

    mMutex.lock();
    abort = true;
    mWaitCondition.wakeOne();
    mMutex.unlock();

    wait();
}

UBFeature::UBFeature(const QString &url, const QImage &icon, const QString &name, const QUrl &realPath, UBFeatureElementType type)
    : mThumbnail(icon), mDisplayName(name), mPath(realPath), elementType(type)
{
    mName = getNameFromVirtualPath(url);
    virtualDir = getVirtualDirFromVirtualPath(url);
}

UBFeature::~UBFeature()
{
}

QString UBFeature::getNameFromVirtualPath(const QString &pVirtPath)
{
    QString result;
    int slashPos = pVirtPath.lastIndexOf("/");
    if (slashPos != -1) {
        result = pVirtPath.right(pVirtPath.count() - slashPos - 1);
    } else {
        qDebug() << "UBFeature: incorrect virtual path parameter specified";
    }

    return result;
}

QString UBFeature::getVirtualDirFromVirtualPath(const QString &pVirtPath)
{
    QString result;
    int slashPos = pVirtPath.lastIndexOf("/");
    if (slashPos != -1) {
        result = pVirtPath.left(slashPos);
    } else {
        qDebug() << "UBFeature: incorrect virtual path parameter specified";
    }

    return result;
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
        || elementType == FEATURE_FOLDER || elementType == FEATURE_SEARCH;
}

bool UBFeature::allowedCopy() const
{
    return isFolder()
            && elementType != FEATURE_CATEGORY
            && elementType != FEATURE_SEARCH;
}

bool UBFeature::isDeletable() const
{
    return elementType == FEATURE_ITEM
            || elementType == FEATURE_AUDIO
            || elementType == FEATURE_VIDEO
            || elementType == FEATURE_IMAGE
            || elementType == FEATURE_FLASH
            || elementType == FEATURE_INTERACTIVE
            || elementType == FEATURE_FOLDER
    //Ilia. Just a hotfix. Permission mechanism for UBFeatures should be reworked
            || getVirtualPath().startsWith("/root/Applications/Web");
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

    rootElement = UBFeature(rootPath, QImage( ":images/libpalette/home.png" ), "root", QUrl());
    audiosElement = UBFeature( audiosPath, QImage(":images/libpalette/AudiosCategory.svg"), tr("Audios") , mUserAudioDirectoryPath, FEATURE_CATEGORY);
    moviesElement = UBFeature( moviesPath, QImage(":images/libpalette/MoviesCategory.svg"), tr("Movies") , mUserVideoDirectoryPath, FEATURE_CATEGORY);
    picturesElement = UBFeature( picturesPath, QImage(":images/libpalette/PicturesCategory.svg"), tr("Pictures") , mUserPicturesDirectoryPath, FEATURE_CATEGORY);
    //flashElement = UBFeature( flashPath, QImage(":images/libpalette/FlashCategory.svg"), tr("Animations") , mUserAnimationDirectoryPath, FEATURE_CATEGORY);
    interactElement = UBFeature( interactPath, QImage(":images/libpalette/InteractivesCategory.svg"), tr("Interactivities") ,  mLibInteractiveDirectoryPath, FEATURE_CATEGORY);
    applicationsElement = UBFeature( appPath, QImage(":images/libpalette/ApplicationsCategory.svg"), tr("Applications") , mUserInteractiveDirectoryPath, FEATURE_CATEGORY);
    shapesElement = UBFeature( shapesPath, QImage(":images/libpalette/ShapesCategory.svg"), tr("Shapes") , mLibShapesDirectoryPath, FEATURE_CATEGORY );
    favoriteElement = UBFeature( favoritePath, QImage(":images/libpalette/FavoritesCategory.svg"), tr("Favorites"), QUrl("favorites"), FEATURE_FAVORITE );
    webSearchElement = UBFeature( webSearchPath, QImage(":images/libpalette/WebSearchCategory.svg"), tr("Web search"), mLibSearchDirectoryPath, FEATURE_CATEGORY);

    trashElement = UBFeature( trashPath, QImage(":images/libpalette/TrashCategory.svg"), tr("Trash"), trashDirectoryPath, FEATURE_TRASH);

    featuresList = new QList <UBFeature>();

    scanFS();

    featuresModel = new UBFeaturesModel(featuresList, this);
    //featuresModel->setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);

    featuresProxyModel = new UBFeaturesProxyModel(this);
    featuresProxyModel->setFilterRegularExpression(QRegularExpression::anchoredPattern(rootPath));
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
    QList<QPair<QUrl, UBFeature> > computingData;

    computingData << QPair<QUrl, UBFeature>(mLibAudiosDirectoryPath, audiosElement)
            <<  QPair<QUrl, UBFeature>(mLibVideosDirectoryPath, moviesElement)
            <<  QPair<QUrl, UBFeature>(mLibAnimationsDirectoryPath, flashElement)
            <<  QPair<QUrl, UBFeature>(mLibPicturesDirectoryPath, picturesElement)

            <<  QPair<QUrl, UBFeature>(mUserInteractiveDirectoryPath, applicationsElement)
            <<  QPair<QUrl, UBFeature>(mUserAudioDirectoryPath, audiosElement)
            <<  QPair<QUrl, UBFeature>(mUserPicturesDirectoryPath, picturesElement)
            <<  QPair<QUrl, UBFeature>(mUserVideoDirectoryPath, moviesElement)
            <<  QPair<QUrl, UBFeature>(mUserAnimationDirectoryPath, flashElement)

            <<  QPair<QUrl, UBFeature>(mLibApplicationsDirectoryPath, applicationsElement)
            <<  QPair<QUrl, UBFeature>(mLibShapesDirectoryPath, shapesElement)
            <<  QPair<QUrl, UBFeature>(mLibInteractiveDirectoryPath, interactElement)
            <<  QPair<QUrl, UBFeature>(trashDirectoryPath, trashElement)
            <<  QPair<QUrl, UBFeature>(mLibSearchDirectoryPath, webSearchElement);

    mCThread.compute(computingData, favoriteSet);
}

void UBFeaturesController::createNpApiFeature(const QString &str)
{
    Q_ASSERT(QFileInfo(str).exists() && QFileInfo(str).isDir());

    QString widgetName = QFileInfo(str).fileName();

    featuresModel->addItem(UBFeature(QString(appPath + "/Web/" + widgetName), QImage(UBGraphicsWidgetItem::iconFilePath(QUrl::fromLocalFile(str))), widgetName, QUrl::fromLocalFile(str), FEATURE_INTERACTIVE));
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
        featuresList->append(UBFeature(appPath + "/" + tool.label, tool.icon.toImage(), tool.label, QUrl(tool.id), FEATURE_INTERNAL));
        if (favoriteSet->find(QUrl(tool.id)) != favoriteSet->end()) {
            featuresList->append(UBFeature(favoritePath + "/" + tool.label, tool.icon.toImage(), tool.label, QUrl(tool.id), FEATURE_INTERNAL));
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
 
        UBFeature testFeature(currVirtualPath + "/" + fileName, icon, fileName, QUrl::fromLocalFile(fullFileName), featureType);

        featuresList->append(testFeature);

        if ( favoriteSet->find( QUrl::fromLocalFile( fullFileName ) ) != favoriteSet->end() ) {
            featuresList->append( UBFeature( favoritePath + "/" + fileName, icon, fileName, QUrl::fromLocalFile( fullFileName ), featureType ) );
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

QString UBFeaturesController::uniqNameForFeature(const UBFeature &feature, const QString &pName, const QString &pExtention) const
{
    Q_ASSERT(featuresList);

    QStringList resultList;
    QString parentVirtualPath = feature.getFullVirtualPath();
    QString resultName;

    qDebug() << "start";
    foreach (UBFeature curFeature, *featuresList) {

        if (curFeature.getFullVirtualPath().startsWith(feature.getFullVirtualPath())) {

            QString curResultName = curFeature.getFullVirtualPath();
            if (!parentVirtualPath.endsWith("/")) {
                parentVirtualPath.append("/");
            }
            //Cut virtual path prevfix
            int i = curResultName.indexOf(feature.getFullVirtualPath());
            if (i != -1) {
                curResultName = curResultName.right(curFeature.getFullVirtualPath().count() - i - parentVirtualPath.count());
            }
            //if directory has children, emptying the name;
            i = curResultName.indexOf("/");
            if (i != -1) {
                curResultName = "";
            }

            if (!curResultName.isEmpty()) {
                resultList.append(curResultName);
            }

            qDebug() << curResultName;
        }
    }

    if (!resultList.contains(pName + pExtention, Qt::CaseInsensitive)) {
        resultName = pName + pExtention;

    } else {
        for (int i = 0; i < 16777215; i++) {
            QString probeName = pName + "_" + QString::number(i) + pExtention;
            if (!resultList.contains(probeName, Qt::CaseInsensitive)) {
                resultName = probeName;
                break;
            }
        }
    }
    qDebug() << "result name is " << resultName;

    return resultName;
}

QString UBFeaturesController::adjustName(const QString &str)
{
    if (str.isNull()) {
        return QString();
    }

    QString resultStr = str;
    QRegularExpression invalidSymbols("[\\/\\s\\:\\?\\*\\|\\<\\>\\\"]+");

    return resultStr.replace(invalidSymbols, "_");
}

void UBFeaturesController::addToFavorite( const QUrl &path )
{
    QString filePath = fileNameFromUrl( path );
    if ( favoriteSet->find( path ) == favoriteSet->end() )
    {
        QFileInfo fileInfo( filePath );
        QString fileName = fileInfo.fileName();
        UBFeatureElementType type = fileTypeFromUrl(filePath);
        UBFeature elem(favoritePath + "/" + fileName, getIcon(filePath, type), fileName, path, fileTypeFromUrl(filePath) );
        favoriteSet->insert( path );
        saveFavoriteList();

        if ( !elem.getVirtualPath().isEmpty() && !elem.getVirtualPath().isNull())
        featuresModel->addItem( elem );
    }
}

void UBFeaturesController::removeFromFavorite( const QUrl &path, bool deleteManualy)
{
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
    if ( fileName.contains( "openboardtool://"))
        return fileName;
    return url.toLocalFile();
}


UBFeatureElementType UBFeaturesController::fileTypeFromUrl(const QString &path)
{
    QFileInfo fileInfo(path);

    if ( path.contains("openboardtool://"))
        return FEATURE_INTERNAL;

    if (!fileInfo.exists()) {
        return FEATURE_INVALID;
    }

    UBFeatureElementType fileType = FEATURE_INVALID;
    QString fileName = fileInfo.fileName();
    QString mimeString = UBFileSystemUtils::mimeTypeFromFileName(fileName);

    if ( mimeString.contains("application")) {
        if (mimeString.contains("application/search")) {
            fileType = FEATURE_SEARCH;
        } else if (mimeString.contains("application/x-shockwave-flash")) {
            fileType = FEATURE_FLASH;
        } else {
            fileType = FEATURE_INTERACTIVE;
        }
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
    QString filePath;
    UBFeature dest = destination;
    if (mFileName.isNull()) {
        QDateTime now = QDateTime::currentDateTime();
        static int imageCounter = 0;
        mFileName  = tr("ImportedImage") + "-" + now.toString("dd-MM-yyyy hh-mm-ss");
        
        filePath = dest.getFullPath().toLocalFile() + "/" + mFileName;

        if (QFile::exists(filePath+".png"))
            mFileName += QString("-[%1]").arg(++imageCounter);
        else
            imageCounter = 0;
        
        mFileName += ".png";
    }
    

    if ( !destination.getFullVirtualPath().startsWith( picturesElement.getFullVirtualPath(), Qt::CaseInsensitive ) )
    {
        dest = picturesElement;
    }

    filePath = dest.getFullPath().toLocalFile() + "/" + mFileName;
    image.save(filePath);

    QImage thumb = createThumbnail( filePath );
    UBFeature resultItem =  UBFeature( dest.getFullVirtualPath() + "/" + mFileName, thumb, mFileName,
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
    UBFeature newFeatureFolder = UBFeature( currentElement.getFullVirtualPath() + "/" + name, QImage(":images/libpalette/folder.svg"),
                                            name, QUrl::fromLocalFile( path ), FEATURE_FOLDER );

    featuresModel->addItem(newFeatureFolder);
    featuresProxyModel->invalidate();
}

void UBFeaturesController::addItemToPage(const UBFeature &item)
{
    UBApplication::boardController->downloadURL( item.getFullPath() );
}

void UBFeaturesController::addItemAsBackground(const UBFeature &item)
{
    UBApplication::boardController->downloadURL( item.getFullPath(), QString(), QPointF(), QSize(), true );
}

UBFeature UBFeaturesController::getDestinationFeatureForUrl( const QUrl &url )
{
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName( url.toString() );
    return getDestinationFeatureForMimeType(mimetype);
}

UBFeature UBFeaturesController::getDestinationFeatureForMimeType(const QString &pMmimeType)
{
    if ( pMmimeType.contains("audio") )
        return audiosElement;
    if ( pMmimeType.contains("video") )
        return moviesElement;
    else if ( pMmimeType.contains("image") || pMmimeType.isEmpty())
        return picturesElement;
    else if ( pMmimeType.contains("application") )
    {
        if ( pMmimeType.contains( "x-shockwave-flash") )
            return flashElement;
        else
            return interactElement;
    }
    return UBFeature();
}

void UBFeaturesController::addDownloadedFile(const QUrl &sourceUrl, const QByteArray &pData, const QString pContentSource, const QString pTitle)
{
    UBFeature dest = getDestinationFeatureForMimeType(pContentSource);

    if (dest == UBFeature())
        return;

    QString fileName;
    QString filePath;
    
    //Audio item
    if(dest == picturesElement) {

        QString UniqName = uniqNameForFeature(dest, adjustName(pTitle), ".jpg");
        fileName =  !UniqName.isNull()
                ? UniqName
                : tr("ImportedImage") + "-" + QDateTime::currentDateTime().toString("dd-MM-yyyy hh-mm-ss")+ ".jpg";

        filePath = dest.getFullPath().toLocalFile() + "/" + fileName;

        QImage::fromData(pData).save(filePath);

        UBFeature downloadedFeature = UBFeature(dest.getFullVirtualPath() + "/" + fileName, getIcon( filePath, fileTypeFromUrl(filePath)),
                                                 fileName, QUrl::fromLocalFile(filePath), FEATURE_ITEM);
        if (downloadedFeature != UBFeature()) {
            featuresModel->addItem(downloadedFeature);
        }

    } else {
        fileName = QFileInfo( sourceUrl.toString() ).fileName();
        filePath = dest.getFullPath().toLocalFile() + "/" + fileName;

        QFile file( filePath );
        if ( file.open(QIODevice::WriteOnly ))
        {
            file.write(pData);
            file.close();

            UBFeature downloadedFeature = UBFeature(dest.getFullVirtualPath() + "/" + fileName, getIcon( filePath, fileTypeFromUrl(filePath)),
                                                    fileName, QUrl::fromLocalFile(filePath), FEATURE_ITEM);
            if (downloadedFeature != UBFeature()) {
                featuresModel->addItem(downloadedFeature);
            }
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

    UBFeature possibleDest = getDestinationFeatureForUrl(url);

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
    UBFeature newElement( destVirtualPath + "/" + name, thumb, name, QUrl::fromLocalFile( newFullPath ), type );
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
    featuresProxyModel->setFilterRegularExpression(QRegularExpression::anchoredPattern(pSiftValue));
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
        featuresSearchModel->setFilterRegularExpression(QRegularExpression::wildcardToRegularExpression("*" + pattern + "*"));
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
    UBFeature newElement( destVirtualPath + "/" + name, thumb, name, QUrl::fromLocalFile( newFullPath ), type );
    return newElement;
}

void UBFeaturesController::moveExternalData(const QUrl &url, const UBFeature &destination)
{
    QString sourcePath = url.toLocalFile();

    Q_ASSERT( QFileInfo( sourcePath ).exists() );

    UBFeature possibleDest = getDestinationFeatureForUrl(url);

    UBFeature dest = destination;

    if ( destination != trashElement && destination != UBFeature()
       /*&& !destination.getFullVirtualPath().startsWith( possibleDest.getFullVirtualPath(), Qt::CaseInsensitive )*/ )
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
    UBFeature newElement(destVirtualPath + "/" + name, thumb, name, QUrl::fromLocalFile(newFullPath), type);

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

void UBFeaturesController::assignFeaturesListView(UBFeaturesListView *pList)
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


