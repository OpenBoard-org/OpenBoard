/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "UBLibraryController.h"

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBDesktopServices.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplicationController.h"

#include "domain/UBAbstractWidget.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsVideoItem.h"
#include "domain/UBGraphicsWidgetItem.h"

#include "tools/UBToolsManager.h"

#include "board/UBBoardView.h"

#include "UBBoardController.h"

#include "gui/UBThumbnailWidget.h"
#include "../core/UBSettings.h"

#include "core/memcheck.h"

static quint32 magicNumber = 0xACDCAFE0;
static QString favoriteVirtualPath = "$favorite$";

UBLibraryController::UBLibraryController(QWidget *pParentWidget) :
        QObject(pParentWidget),
        mParentWidget(pParentWidget),
        mBoardController(UBApplication::boardController),
        mLastItemOffsetIndex(0)
{
    readFavoriteList();

    mAudioStandardDirectoryPath = QUrl::fromLocalFile(UBDesktopServices::storageLocation(QDesktopServices::MusicLocation));
    userPath(mAudioStandardDirectoryPath);

    mVideoStandardDirectoryPath = QUrl::fromLocalFile(UBDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
    userPath(mVideoStandardDirectoryPath);

    mPicturesStandardDirectoryPath = QUrl::fromLocalFile(UBDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    userPath(mPicturesStandardDirectoryPath);

    mInteractiveUserDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->uniboardInteractiveUserDirectory());

    mAnimationUserDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->animationUserDirectory());

    createInternalWidgetItems();

}

bool UBLibraryController::canItemsOnElementBeDeleted(UBLibElement *pElement)
{
    return !pElement->path().toLocalFile().startsWith(UBSettings::settings()->uniboardShapeLibraryDirectory()) &&
            !pElement->path().toLocalFile().startsWith(UBSettings::settings()->sankoreDistributedInteractiveDirectory()) &&
            pElement->isDeletable();
}

void UBLibraryController::createInternalWidgetItems()
{
    QStringList toolUris = UBToolsManager::manager()->allToolIDs();

    foreach(QString toolUri, toolUris)
    {
        UBToolsManager::UBToolDescriptor tool = UBToolsManager::manager()->toolByID(toolUri);
        UBLibElement *newTool = new UBLibElement(eUBLibElementType_InteractiveItem, QUrl(tool.id), tool.label);
        QImage toolImage = tool.icon.toImage();
        newTool->setThumbnail(toolImage);
        newTool->setInformation(tool.label + " " + tool.version);

        mInternalLibElements << newTool;
    }
}

void UBLibraryController::createNewFolder(QString name, UBLibElement *parentElem)
{
    Q_ASSERT(parentElem);
    Q_ASSERT(parentElem->type() == eUBLibElementType_Folder);

    QUrl qsPath = QUrl::fromLocalFile(parentElem->path().toLocalFile() + "/" + name);
    createDirectory(qsPath);
}

void UBLibraryController::createDirectory(QUrl& pDirPath)
{
    if(!QFileInfo(pDirPath.toLocalFile()).exists())
        QDir().mkpath(pDirPath.toLocalFile());
}

void UBLibraryController::routeItem(QString& pItem, QString pMiddleDirectory)
{
    QFileInfo itemToRoute(pItem);
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName(itemToRoute.fileName());
    QString destination("");

    if(mimetype.contains("audio"))
        destination = mAudioStandardDirectoryPath.toLocalFile();
    else if (mimetype.contains("video"))
        destination = mVideoStandardDirectoryPath.toLocalFile();
    else if (mimetype.contains("image"))
        destination = mPicturesStandardDirectoryPath.toLocalFile();
    else if (mimetype.contains("application")){
        if (mimetype.contains("x-shockwave-flash"))
            destination = mAnimationUserDirectoryPath.toLocalFile();
        else
            destination = mInteractiveUserDirectoryPath.toLocalFile();
    }
    else{
        return;
    }

    if(!destination.isEmpty()){
        if(!pMiddleDirectory.isEmpty()){
            destination = destination + "/" + pMiddleDirectory;
            QUrl url = QUrl::fromLocalFile(destination);
            createDirectory(url);
        }
        destination = UBFileSystemUtils::normalizeFilePath(destination + "/" + itemToRoute.fileName());
        QFile::copy(pItem, destination);
    }
}

void UBLibraryController::importImageOnLibrary(QImage& pImage)
{
    if(!pImage.isNull()){
        QDateTime now = QDateTime::currentDateTime();
        QString filePath = mPicturesStandardDirectoryPath.toLocalFile() + "/" + tr("ImportedImage") + "-" + now.toString("dd-MM-yyyy hh-mm-ss") + ".png";
        filePath = UBFileSystemUtils::normalizeFilePath(filePath);
        pImage.save(filePath);
        UBApplication::showMessage(tr("Added 1 Image to Library"));
    }
}

void UBLibraryController::importItemOnLibrary(QString& pItemString)
{
    pItemString.replace("\r","");
    pItemString.replace("\n","");

    QFileInfo itemToImport(pItemString);

    bool isZip = false;
    if(itemToImport.isDir() || (isZip = UBFileSystemUtils::isAZipFile(pItemString))){
        if(pItemString.contains(".wgt",Qt::CaseInsensitive) || pItemString.contains(".wdgt",Qt::CaseInsensitive)){
            QString destination = UBSettings::settings()->uniboardInteractiveUserDirectory() + "/" + itemToImport.fileName();
            if(isZip)
                UBFileSystemUtils::expandZipToDir(pItemString,destination);
            else{
                if(pItemString.endsWith("/")) pItemString.chop(1);
                UBFileSystemUtils::copyDir(pItemString, destination + QFileInfo(pItemString).fileName() + "/");
            }
            return;
        }

        QStringList allFiles = UBFileSystemUtils::allFiles(pItemString);
        QString pathBeforeCurrentDir = pItemString;
        pathBeforeCurrentDir.replace(UBFileSystemUtils::lastPathComponent(pathBeforeCurrentDir), "");
        if(pathBeforeCurrentDir.endsWith("//"))
            pathBeforeCurrentDir.chop(1);

        foreach(QString eachFile, allFiles){
            QString intermediateDirectory = eachFile;
            intermediateDirectory = intermediateDirectory.remove(pathBeforeCurrentDir);
            intermediateDirectory = intermediateDirectory.remove(QFileInfo(eachFile).fileName());
            routeItem(eachFile,intermediateDirectory);
        }
    }
    else{
        routeItem(pItemString);
    }

}

void UBLibraryController::userPath(QUrl& pPath)
{
    pPath = QUrl::fromLocalFile(pPath.toLocalFile() + "/Sankore");
    createDirectory(pPath);
}

QList<UBLibElement*> UBLibraryController::rootCategoriesList()
{
    QList<UBLibElement*> categories;

    UBLibElement* element = new UBLibElement(eUBLibElementType_Folder, mAudioStandardDirectoryPath, tr("Audios", "Audio category element"));
    element->setThumbnail(QImage(":images/libpalette/AudiosCategory.svg"));
    element->setMoveable(false);
    categories << element;

    element = new UBLibElement(eUBLibElementType_Folder, mVideoStandardDirectoryPath, tr("Movies", "Movies category element"));
    element->setThumbnail(QImage(":images/libpalette/MoviesCategory.svg"));
    element->setMoveable(false);
    categories << element;

    element = new UBLibElement(eUBLibElementType_Folder, mPicturesStandardDirectoryPath, tr("Pictures", "Pictures category element"));
    element->setThumbnail(QImage(":images/libpalette/PicturesCategory.svg"));
    element->setMoveable(false);
    categories << element;

    QString path = UBSettings::settings()->uniboardShapeLibraryDirectory();
    element = new UBLibElement(eUBLibElementType_Folder, QUrl::fromLocalFile(path), tr("Shapes", "Shapes category element"));
    element->setThumbnail(QImage(":images/libpalette/ShapesCategory.svg"));
    element->setMoveable(false);
    categories << element;

    element = new UBLibElement(eUBLibElementType_Folder, mInteractiveUserDirectoryPath, tr("Applications", "Applications category element"));
    element->setThumbnail(QImage(":images/libpalette/ApplicationsCategory.svg"));
    element->setMoveable(false);
    categories << element;

    element = new UBLibElement(eUBLibElementType_VirtualFolder, favoriteVirtualPath, tr("Favorite", "Favorite category element"));
    element->setThumbnail(QImage(":images/libpalette/FavoritesCategory.svg"));
    element->setMoveable(false);
    categories << element;

    mInteractiveCategoryPath = QUrl::fromLocalFile(UBSettings::settings()->uniboardGipLibraryDirectory());
    element = new UBLibElement(eUBLibElementType_Folder, mInteractiveCategoryPath, tr("Interactivities", "Interactives category element"));
    element->setThumbnail(QImage(":images/libpalette/InteractivesCategory.svg"));
    element->setMoveable(false);
    categories << element;

//  Note : FEATURE IN DEVELOPMENT, DO NOT ERASE (or you will get problems) !!!!
    mSearchCategoryPath = QUrl::fromLocalFile(UBSettings::settings()->uniboardSearchDirectory());
    element = new UBLibElement(eUBLibElementType_Folder, mSearchCategoryPath, tr("Web Search", "Web search category element"));
    element->setThumbnail(QImage(":images/libpalette/WebSearchCategory.svg"));
    element->setMoveable(false);
    categories << element;

    element = new UBLibElement(eUBLibElementType_Folder, mAnimationUserDirectoryPath, tr("Animations", "Animations category element"));
    element->setThumbnail(QImage(":images/libpalette/FlashCategory.svg"));
    element->setMoveable(false);
    categories << element;



    categories << UBLibElement::trashElement();

    return categories;
}

QImage UBLibraryController::createThumbnail(UBLibElement* pElement)
{
    QString thumbnailPath = UBFileSystemUtils::thumbnailPath(pElement->path().toLocalFile());
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName(pElement->path().toLocalFile());
    UBApplication::showMessage(tr("Creating image thumbnail for %1.").arg(pElement->name()));

    if (mimetype.contains("audio"))
        thumbnailPath = ":images/libpalette/soundIcon.svg";
    else if (mimetype.contains("video"))
        thumbnailPath = ":images/libpalette/movieIcon.svg";
    else {
        if (pElement->extension().startsWith("svg", Qt::CaseInsensitive) || pElement->extension().startsWith("svgz", Qt::CaseInsensitive)) {
            thumbnailPath = pElement->path().toLocalFile();
            UBThumbnailSvg(pElement->path().toLocalFile());
        }
        else {
            QPixmap pix(pElement->path().toLocalFile());
            if (!pix.isNull()) {
                pix = pix.scaledToWidth(qMin(UBSettings::maxThumbnailWidth, pix.width()), Qt::SmoothTransformation);
                pix.save(thumbnailPath);
                UBThumbnailPixmap pixmap(pix);
                UBPlatformUtils::hideFile(thumbnailPath);
            }
            else{
                thumbnailPath = ":images/libpalette/notFound.png";
            }
        }
    }

    return QImage(thumbnailPath);
}

QImage UBLibraryController::thumbnailForFile(UBLibElement* pElement)
{
    if (pElement->path().toString().contains("uniboardTool://")){
            QImage image = QImage(UBToolsManager::manager()->iconFromToolId(pElement->path().toString()));
            return image;
    }
    if (pElement->type() == eUBLibElementType_InteractiveItem){
        QImage image = QImage(UBAbstractWidget::iconFilePath(pElement->path()));
        return image;
    }

    QString thumbnailPath = UBFileSystemUtils::thumbnailPath(pElement->path().toLocalFile());

    if (!thumbnailPath.length())
        qWarning() << "thumbnailForFile impossible to create thumbnail path for the element " + pElement->path().toLocalFile();

    if (QFileInfo(thumbnailPath).exists())
        return QImage(thumbnailPath);
    else
        return createThumbnail(pElement);
}

QList<UBLibElement*> UBLibraryController::addVirtualElementsForItemPath(const QString& pPath)
{
    QList<UBLibElement*> content;
    if (pPath == mInteractiveUserDirectoryPath.toLocalFile()){
        content << listElementsInPath(UBSettings::settings()->uniboardInteractiveLibraryDirectory());
        content << listElementsInPath(UBSettings::settings()->uniboardInteractiveFavoritesDirectory());
        foreach(UBLibElement* eachElement, mInternalLibElements)
            content << new UBLibElement(eachElement);
    }
    else if (pPath == mPicturesStandardDirectoryPath.toLocalFile()){
        QUrl path = QUrl::fromLocalFile(UBSettings::settings()->uniboardImageLibraryDirectory());
        userPath(path);
        content << listElementsInPath(path.toLocalFile());
        content << listElementsInPath(UBSettings::settings()->uniboardDefaultUserImageLibraryDirectory());
    }
    else if (pPath == mInteractiveCategoryPath.toLocalFile()){
        content << listElementsInPath(UBSettings::settings()->sankoreDistributedInteractiveDirectory());
    }

    return content;
}

QList<UBLibElement*> UBLibraryController::listElementsInPath(const QString& pPath)
{
    QList<UBLibElement*> content;
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(pPath);

    QFileInfoList::iterator fileInfo;
    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo += 1) {
        eUBLibElementType fileType = fileInfo->isDir() ? eUBLibElementType_Folder : eUBLibElementType_Item;

        QString fileName = fileInfo->fileName();
        if (UBFileSystemUtils::mimeTypeFromFileName(fileName).contains("application")) {
            fileType = eUBLibElementType_InteractiveItem;
        }

        // This is necessary because of the w3c widget directory (xxxx.wgt).
        QString itemName = (fileType != eUBLibElementType_Item) ? fileName : fileInfo->completeBaseName();

        UBLibElement *element = new UBLibElement(fileType, QUrl::fromLocalFile(fileInfo->absoluteFilePath()), itemName);

        if (fileType == eUBLibElementType_Folder) {
            element->setThumbnail(QImage(":images/libpalette/folder.svg"));
        }
        else if (fileType == eUBLibElementType_Item) {
            if (element->path().toLocalFile().contains(".thumbnail."))
                continue;
            element->setThumbnail(thumbnailForFile(element));
        }
        content << element;
    }
    content << addVirtualElementsForItemPath(pPath);

    return content;
}



QList<UBLibElement*> UBLibraryController::listElementsInVirtualForlder(UBLibElement* pElement)
{
    Q_UNUSED(pElement);
    QList<UBLibElement*> copyOfTheFavoriteList;
    foreach(UBLibElement* eachElement, mFavoriteList)
        copyOfTheFavoriteList << new UBLibElement(eachElement);
    return copyOfTheFavoriteList;
}

void UBLibraryController::moveContent(QList<UBLibElement*> sourceList, UBLibElement *pDestination)
{
    if (pDestination->type() != eUBLibElementType_Folder)
        qWarning() << "moveContent the destination element isn't a directory";

    foreach (UBLibElement* eachSource, sourceList) {
        if (!eachSource || pDestination->path().toLocalFile() == eachSource->path().toLocalFile() || eachSource->type() == eUBLibElementType_VirtualFolder)
            continue;

        if (QFileInfo(eachSource->path().toLocalFile()).isDir())
            UBFileSystemUtils::moveDir(eachSource->path().toLocalFile(), pDestination->path().toLocalFile() + "/" + eachSource->name());
        else {
            QFile(eachSource->path().toLocalFile()).copy(pDestination->path().toLocalFile() + "/" + eachSource->name() + "." + eachSource->extension());
            QFile::remove(eachSource->path().toLocalFile());
            QString thumbnailPath = UBFileSystemUtils::thumbnailPath(eachSource->path().toLocalFile());
            if (thumbnailPath.length() && QFileInfo(thumbnailPath).exists()) {
                QString thumbnailFileName = UBFileSystemUtils::lastPathComponent(thumbnailPath);
                QFile(thumbnailPath).copy(pDestination->path().toLocalFile() + "/" + thumbnailFileName);
                QFile::remove(thumbnailPath);
            }
        }
    }
}

void UBLibraryController::trashElements(QList<UBLibElement*> trashList)
{
    moveContent(trashList, UBLibElement::trashElement());
    removeFromFavorite(trashList);
}

void UBLibraryController::emptyElementsOnTrash( QList<UBLibElement*> elementsList)
{
    foreach(UBLibElement* eachElement, elementsList) {
        if (QFileInfo(eachElement->path().toLocalFile()).isDir())
            UBFileSystemUtils::deleteDir(eachElement->path().toLocalFile());
        else
            QFile::remove(eachElement->path().toLocalFile());
    }
}

void UBLibraryController::cleanElementsList()
{
    qDeleteAll(mElementsList);
    mElementsList.clear();
}

QList<UBLibElement*> UBLibraryController::getContent(UBLibElement *element)
{
    cleanElementsList();

    switch (element->type()) {
    case eUBLibElementType_Category: {
            mElementsList = rootCategoriesList();
            break;
        }
    case eUBLibElementType_VirtualFolder: {
            mElementsList = listElementsInVirtualForlder(element);
            break;
        }
    case eUBLibElementType_Folder: {
            mElementsList = listElementsInPath(element->path().toLocalFile());
            break;
        }
    case eUBLibElementType_Item: {
            qWarning() << "You are browsing a file";
            break;
        }
    default:
        // We should never come here...
        Q_ASSERT(false);
        break;
    }

    return mElementsList;
}

UBLibraryController::~UBLibraryController()
{
    cleanElementsList();
	//NOOP
}

void UBLibraryController::setItemAsBackground(UBLibElement* image)
{
    if (!activeScene()){
        qDebug() << "No active scene";
        return;
    }
    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(image->path().toLocalFile());

    if(mimeType == "image/svg+xml"){
        QUrl url(QUrl::fromLocalFile(image->path().toLocalFile()));
        QGraphicsSvgItem* boardItem = activeScene()->addSvg(url, QPointF(0, 0));
        activeScene()->setAsBackgroundObject(boardItem, true, true);
    }
    else{
        QPixmap pix(image->path().toLocalFile());
        UBGraphicsPixmapItem* boardItem = activeScene()->addPixmap(pix, QPointF(0, 0));
        activeScene()->setAsBackgroundObject(boardItem, true);
    }
}

void UBLibraryController::addItemToPage(UBLibElement* item)
{
    QList<QUrl> list;
    list << QUrl::fromLocalFile(item->path().toLocalFile());
    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(item->path().toLocalFile());

    if (mimeType.contains("image")){
        addImagesToCurrentPage(list);
    }
    else if (mimeType.contains("audio")){
        addAudiosToCurrentPage(list);
    }
    else if (mimeType.contains("video")){
        addVideosToCurrentPage(list);
    }
    else if (mimeType.contains("application")){
        addInteractivesToCurrentPage(list);
    }
    else if (mimeType.isEmpty() && item->type() == eUBLibElementType_InteractiveItem){
        // Those conditions allow us to detect internal app like
        // mask, ruler, compass and protractor
        list.clear();
        list << item->path().toString().replace("file:","");
        addInteractivesToCurrentPage(list);
    }
    else{
        UBApplication::showMessage(tr("Adding to page failed for item %1.").arg(item->name()));
    }
}

void UBLibraryController::removeBackground()
{
    if (activeScene())
        activeScene()->setAsBackgroundObject(0);

}

UBGraphicsScene* UBLibraryController::activeScene()
{
    if (mBoardController->activeDocument())
        return mBoardController->activeScene();

    return 0;
}


void UBLibraryController::persistFavoriteList()
{
    QFile file(UBSettings::libraryPaletteFavoriteListFilePath());
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    //magic number
    out << magicNumber;
    out << (quint32)mFavoriteList.count();
    foreach(UBLibElement* eachElement, mFavoriteList)
    {
        out << (quint32)eachElement->type();
        out << eachElement->path().toString();
        out << eachElement->information();
        out << eachElement->name();
        out << eachElement->extension();
    }
    file.close();
}

void UBLibraryController::readFavoriteList()
{
    QFile file(UBSettings::libraryPaletteFavoriteListFilePath());
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);    // read the data serialized from the file
    quint32 magicNumber;
    quint32 numberOfElements;

    in >> magicNumber >> numberOfElements;

    for(quint32 i = 0; i < numberOfElements; i += 1){
        quint32 type;
        QString path;
        QString info;
        QString name;
        QString extension;

        in >> type >> path >> info >> name >> extension;

        UBLibElement* eachElement = new UBLibElement((eUBLibElementType)type, QUrl(path), name);
        eachElement->setInformation(info);
        eachElement->setExtension(extension);
        eachElement->setThumbnail(thumbnailForFile(eachElement));
        if(!isOnFavoriteList(eachElement))
            mFavoriteList << eachElement;
    }

    file.close();
    persistFavoriteList();
}

UBLibElement* UBLibraryController::isOnFavoriteList(UBLibElement * element)
{
    foreach(UBLibElement* eachElement, mFavoriteList)
        if(eachElement->path() == element->path())
            return eachElement;

    return 0;
}
void UBLibraryController::addToFavorite(QList<UBLibElement*> elementList)
{

    foreach(UBLibElement* eachElement, elementList)
        if(!isOnFavoriteList(eachElement))
            mFavoriteList << new UBLibElement(eachElement);

    persistFavoriteList();
}


void UBLibraryController::removeFromFavorite(QList<UBLibElement*> elementList)
{
    foreach(UBLibElement* eachElement, elementList){
        UBLibElement* elementToRemove = isOnFavoriteList(eachElement);
        if(elementToRemove)
            mFavoriteList.removeOne(elementToRemove);
    }

    persistFavoriteList();
}

QRectF UBLibraryController::visibleSceneRect()
{
    QRectF visibleSceneRect(0, 0, 0, 0);

    if (activeScene() && mBoardController && mBoardController->controlView()) {
        QPointF topLeftCorner = mBoardController->controlGeometry().topLeft();
        QPointF bottomRightCorner =
                mBoardController->controlGeometry().bottomRight();

        QPointF sceneTopLeft = mBoardController->controlView()->mapToScene(
                topLeftCorner.toPoint());
        QPointF sceneBottomRight = mBoardController->controlView()->mapToScene(
                bottomRightCorner.toPoint());

        visibleSceneRect.setTopLeft(sceneTopLeft);
        visibleSceneRect.setBottomRight(sceneBottomRight);
    }

    return visibleSceneRect;
}

void UBLibraryController::addImagesToCurrentPage(const QList<QUrl>& images)
{
    QPointF pos = visibleSceneRect().topLeft();

    foreach(const QUrl url, images)
    {
        mLastItemOffsetIndex++;
        mLastItemOffsetIndex = qMin(mLastItemOffsetIndex, 5);

        QGraphicsItem* itemInScene = 0;

        if (activeScene()) {
            QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(
                    url.toString());

            //TODO UB 4.x move this logic to the scene ..
            if (mimeType == "image/svg+xml") {
                itemInScene = activeScene()->addSvg(url, pos);
            } else {
                itemInScene = activeScene()->addPixmap(QPixmap(
                        url.toLocalFile()), pos);
            }
        }

        if (itemInScene) {
            itemInScene = activeScene()->scaleToFitDocumentSize(itemInScene, false, UBSettings::objectInControlViewMargin);

            itemInScene->setPos(QPoint(pos.x() + 50 * mLastItemOffsetIndex, pos.y() + 50 * mLastItemOffsetIndex));
        }
    }
}

void UBLibraryController::addVideosToCurrentPage(const QList<QUrl>& videos)
{
    QPointF pos = visibleSceneRect().topLeft();

    foreach(const QUrl url, videos)
    {
        mLastItemOffsetIndex++;
        mLastItemOffsetIndex = qMin(mLastItemOffsetIndex, 5);

        UBGraphicsVideoItem* itemInScene = UBApplication::boardController->addVideo(url, false, pos);
        itemInScene->setPos(QPoint(pos.x() + 50 * mLastItemOffsetIndex, pos.y() + 50 * mLastItemOffsetIndex));
    }
}

void UBLibraryController::addAudiosToCurrentPage(const QList<QUrl>& sounds)
{
    QPointF topLeftPos = visibleSceneRect().topLeft();

    QPointF pos = topLeftPos;

    foreach(const QUrl url, sounds)
    {
        mLastItemOffsetIndex++;
        mLastItemOffsetIndex = qMin(mLastItemOffsetIndex, 5);
        pos = QPointF(topLeftPos.x() + 50 * mLastItemOffsetIndex,
                      topLeftPos.y() + 50 * mLastItemOffsetIndex);
        UBApplication::boardController->addAudio(url, false, pos);
    }
}

void UBLibraryController::addInteractivesToCurrentPage( const QList<QUrl>& widgets)
{
    foreach(const QUrl url, widgets)
        mBoardController->downloadURL(url, QPointF(0, 0));
}

QString UBLibraryController::favoritePath()
{
    return favoriteVirtualPath;
}

UBLibElement::UBLibElement() {
    mType = eUBLibElementType_Category;
    mName = QObject::tr("/Home", "Category list label on navigation tool bar");
    mbMoveable = false;
    mbDeletable = true;
}

UBLibElement::UBLibElement(UBLibElement* element)
{
    mType = element->type();
    mPath = element->path();
    mThumbnail = *element->thumbnail();
    mInfo = element->information();
    mName = element->name();
    mExtension = element->extension();
    mbMoveable = element->isMoveable();
    mbDeletable = element->isDeletable();
}


UBLibElement::UBLibElement(eUBLibElementType type, const QUrl &path, const QString &name)
{
    mType = type;
    mPath = path;
    mName = name;
    mInfo = "";
    mbMoveable = true;
    mbDeletable = true;

    if (type == eUBLibElementType_Folder)
        mThumbnail = QImage(":images/libpalette/folder.svg");

    if (type == eUBLibElementType_InteractiveItem)
        mThumbnail = QImage(UBAbstractWidget::iconFilePath(path));

    if (type == eUBLibElementType_Item)
        mExtension = QFileInfo(path.toLocalFile()).completeSuffix();

    if(type == eUBLibElementType_VirtualFolder || type == eUBLibElementType_Category)
        mbMoveable = false;
}

UBLibElement::~UBLibElement()
{
    //NOOP
}

UBChainedLibElement::UBChainedLibElement(UBLibElement *pElem, UBChainedLibElement *pNextElem)
{
    mpElem = new UBLibElement(pElem);
    mpNextElem = pNextElem;
}

UBChainedLibElement::~UBChainedLibElement()
{
    if (NULL != mpNextElem) {
        delete mpNextElem;
        mpNextElem = NULL;
    }
}

void UBChainedLibElement::setNextElement(UBChainedLibElement *nextElem)
{
    mpNextElem = nextElem;
}

UBChainedLibElement* UBChainedLibElement::lastElement()
{
    UBChainedLibElement* pElem = NULL;

    if(NULL != mpNextElem)
    {
        UBChainedLibElement* pLast = mpNextElem->lastElement();
        if(NULL != pLast)
        {
            pElem = pLast;
        }
        else
        {
            pElem = mpNextElem;
        }
    }

    return pElem;
}

QUrl UBChainedLibElement::lastItemPath()
{
    return lastElement()->element()->path();
}

UBLibElement* UBLibElement::trashElement()
{
    UBLibElement *trashElement;
	trashElement = new UBLibElement(eUBLibElementType_Folder, QUrl::fromLocalFile(UBSettings::trashLibraryPaletteDirPath()), QObject::tr("Trash", "Pictures category element"));
    trashElement->setThumbnail(QImage(":images/libpalette/TrashCategory.svg"));
    trashElement->setMoveable(false);

    return trashElement;
}

