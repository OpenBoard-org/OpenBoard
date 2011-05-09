#include "UBLibraryController.h"

#include <QtSvg>
#include <QtXml>
#include <QSettings>
#include <Qt>

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBDesktopServices.h"
#include "frameworks/UBFileSystemUtils.h"

#include "pdf/XPDFRenderer.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplicationController.h"

#include "network/UBNetworkAccessManager.h"
#include "network/UBServerXMLHttpRequest.h"

#include "domain/UBAbstractWidget.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsVideoItem.h"
#include "domain/UBGraphicsWidgetItem.h"

#include "tools/UBToolsManager.h"

#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"

#include "web/UBWebPage.h"
#include "web/UBWebController.h"

#include "gui/UBMainWindow.h"

#include "api/UBLibraryAPI.h"

#include "UBBoardController.h"
#include "../frameworks/UBFileSystemUtils.h"

#include <QDataStream>

static quint32 magicNumber = 0xACDCAFE0;
static QString favoriteVirtualPath = "$favorite$";

UBLibraryController::UBLibraryController(QWidget *pParentWidget, UBBoardController *pBoardController) :
        QObject(pParentWidget),
        mParentWidget(pParentWidget),
        mBoardController(pBoardController),
        mLibraryWindow(0),
        mLibraryUI(0),
        mImageWebView(0),
        mVideoWebView(0),
        mInteractiveWebView(0),
        mImageOnlineTi(0),
        mVideoOnlineTi(0),
        mInteractiveOnlineTi(0),
        mInteractiveUniboardTi(0),
        mLastItemOffsetIndex(0),
        mNeedRefreshOnNextDisplay(false)
{
    QString dummyBackgrounds = tr("Backgrounds");
    mLibraryFileToDownload = onlineLibraries();
    readFavoriteList();

    mAudioStandardDirectoryPath = QUrl::fromLocalFile(UBDesktopServices::storageLocation(QDesktopServices::MusicLocation));
    userPath(mAudioStandardDirectoryPath);

    mVideoStandardDirectoryPath = QUrl::fromLocalFile(UBDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
    userPath(mVideoStandardDirectoryPath);

    mPicturesStandardDirectoryPath = QUrl::fromLocalFile(UBDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    userPath(mPicturesStandardDirectoryPath);

    mInteractiveUserDirectoryPath = QUrl::fromLocalFile(UBSettings::settings()->uniboardInteractiveUserDirectory());

    qDebug() << ">>  mInteractiveUserDirectoryPath : " << mInteractiveUserDirectoryPath;

    createInternalWidgetItems();

}

bool UBLibraryController::canItemsOnElementBeDeleted(UBLibElement *pElement)
{
    return !pElement->path().toLocalFile().startsWith(UBSettings::settings()->uniboardShapeLibraryDirectory());
}

void UBLibraryController::createInternalWidgetItems()
{
    QStringList toolUris = UBToolsManager::manager()->allToolIDs();

    foreach(QString toolUri, toolUris)
    {
        UBToolsManager::UBToolDescriptor tool = UBToolsManager::manager()->toolByID(toolUri);
        UBLibElement *newTool = new UBLibElement(eUBLibElementType_InteractiveItem, QUrl(tool.id), tool.label);
        QImage toolImage = tool.icon.toImage();
        newTool->setThumbnail(&toolImage);
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

void UBLibraryController::preloadFirstOnlineLibrary()
{
    new UBLibraryPreloader(UBApplication::staticMemoryCleaner);
}

QStringList UBLibraryController::onlineLibraries()
{
    QStringList libraries;
    libraries << UBSettings::settings()->softwareLibrariesUrl + "/libraries-release.xml";
    return libraries;
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
    else if (mimetype.contains("application"))
        destination = UBSettings::settings()->uniboardInteractiveUserDirectory();
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
        QString filePath = mPicturesStandardDirectoryPath.toLocalFile() + "/ImportedImage-" + now.toString("dd-MM-yyyy hh-mm-ss") + ".png";
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
    pPath = QUrl::fromLocalFile(pPath.toLocalFile() + "/" + QCoreApplication::applicationName());
    createDirectory(pPath);
}

QList<UBLibElement*> UBLibraryController::rootCategoriesList()
{
    QList<UBLibElement*> categories;

    UBLibElement* element = new UBLibElement(eUBLibElementType_Folder, mAudioStandardDirectoryPath, tr("Audios", "Audio category element"));
    QImage* categoryImage = new QImage(":images/libpalette/AudiosCategory.svg");
    element->setThumbnail(categoryImage);
    element->setMoveable(false);
    categories << element;

    element = new UBLibElement(eUBLibElementType_Folder, mVideoStandardDirectoryPath, tr("Movies", "Movies category element"));
    categoryImage = new QImage(":images/libpalette/MoviesCategory.svg");
    element->setThumbnail(categoryImage);
    element->setMoveable(false);
    categories << element;

    element = new UBLibElement(eUBLibElementType_Folder, mPicturesStandardDirectoryPath, tr("Pictures", "Pictures category element"));
    categoryImage = new QImage(":images/libpalette/PicturesCategory.svg");
    element->setThumbnail(categoryImage);
    element->setMoveable(false);
    categories << element;

    QString path = UBSettings::settings()->uniboardShapeLibraryDirectory();
    categoryImage = new QImage(":images/libpalette/ShapesCategory.svg");
    element = new UBLibElement(eUBLibElementType_Folder, QUrl::fromLocalFile(path), tr("Shapes", "Shapes category element"));
    element->setThumbnail(categoryImage);
    element->setMoveable(false);
    categories << element;


    categoryImage = new QImage(":images/libpalette/ApplicationsCategory.svg");
    element = new UBLibElement(eUBLibElementType_Folder, mInteractiveUserDirectoryPath, tr("Applications", "Applications category element"));
    element->setThumbnail(categoryImage);
    element->setMoveable(false);
    categories << element;

    categoryImage = new QImage(":images/libpalette/FavoritesCategory.svg");
    element = new UBLibElement(eUBLibElementType_VirtualFolder, favoriteVirtualPath, tr("Favorite", "Favorite category element"));
    element->setThumbnail(categoryImage);
    element->setMoveable(false);
    categories << element;

    categoryImage = new QImage(":images/libpalette/InteractivesCategory.svg");
    mInteractiveCategoryPath = QUrl::fromLocalFile(UBSettings::settings()->uniboardGipLibraryDirectory());
    element = new UBLibElement(eUBLibElementType_Folder, mInteractiveCategoryPath, tr("Interactives", "Interactives category element"));
    element->setThumbnail(categoryImage);
    element->setMoveable(false);
    categories << element;


    categories << UBLibElement::trashElement();

    return categories;
}

QImage* UBLibraryController::createThumbnail(UBLibElement* pElement)
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

    return new QImage(thumbnailPath);
}

QImage* UBLibraryController::thumbnailForFile(UBLibElement* pElement)
{
    QString thumbnailPath = UBFileSystemUtils::thumbnailPath(pElement->path().toLocalFile());

    if (!thumbnailPath.length())
        qWarning() << "thumbnailForFile impossible to create thumbnail path for the element " + pElement->path().toLocalFile();

    if (QFileInfo(thumbnailPath).exists())
        return new QImage(thumbnailPath);
    else
        return createThumbnail(pElement);
}

QList<UBLibElement*> UBLibraryController::addVirtualElementsForItemPath(const QString& pPath)
{
    QList<UBLibElement*> content;
    if (pPath == mInteractiveUserDirectoryPath.toLocalFile())
        content << mInternalLibElements;
    else if (pPath == mPicturesStandardDirectoryPath.toLocalFile()){
        QUrl path = QUrl::fromLocalFile(UBSettings::settings()->uniboardImageLibraryDirectory());
        userPath(path);
        content << listElementsInPath(path.toLocalFile());
    }
    else if (pPath == mInteractiveUserDirectoryPath.toLocalFile()){
        content << listElementsInPath(UBSettings::settings()->uniboardInteractiveLibraryDirectory());
        content << listElementsInPath(UBSettings::settings()->uniboardInteractiveFavoritesDirectory());
    }

    return content;
}

QList<UBLibElement*> UBLibraryController::listElementsInPath(const QString& pPath)
{
    QList<UBLibElement*> content = addVirtualElementsForItemPath(pPath);
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
            QImage* directoryImage = new QImage(":images/libpalette/folder.svg");
            element->setThumbnail(directoryImage);
        }
        else if (fileType == eUBLibElementType_Item) {
            if (element->path().toLocalFile().contains(".thumbnail."))
                continue;
            element->setThumbnail(thumbnailForFile(element));
        }
        content << element;
    }

    return content;
}



QList<UBLibElement*> UBLibraryController::listElementsInVirtualForlder(UBLibElement* pElement)
{
    Q_UNUSED(pElement);
    return mFavoriteList;
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
        if (eachElement->type() == eUBLibElementType_Folder)
            UBFileSystemUtils::deleteDir(eachElement->path().toLocalFile());
        else
            QFile::remove(eachElement->path().toLocalFile());
    }
}

QList<UBLibElement*> UBLibraryController::getContent(UBLibElement *element)
{
    QList<UBLibElement*> elementsList;

    switch (element->type()) {
    case eUBLibElementType_Category: {
            elementsList = rootCategoriesList();
            break;
        }
    case eUBLibElementType_VirtualFolder: {
            elementsList = listElementsInVirtualForlder(element);
            break;
        }
    case eUBLibElementType_Folder: {
            elementsList = listElementsInPath(element->path().toLocalFile());
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

    return elementsList;
}

UBLibraryController::~UBLibraryController()
{
    delete mLibraryUI;
}



void UBLibraryController::setItemAsBackground(UBLibElement* image)
{
    if (!activeScene()){
        qWarning() << "No active scene";
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
    else{
        UBApplication::showMessage(tr("Adding to page failed for item %1.").arg(item->name()));
    }



}

void UBLibraryController::removeBackground()
{
    if (activeScene()) {
        activeScene()->setAsBackgroundObject(0);
        selectionChanged();
    }
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
        out << eachElement->path();
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

        UBLibElement* eachElement = new UBLibElement((eUBLibElementType)type, path, name);
        eachElement->setInformation(info);
        eachElement->setExtension(extension);
        eachElement->setThumbnail(thumbnailForFile(eachElement));
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

    foreach(UBLibElement* eachElement, elementList){
        if(!isOnFavoriteList(eachElement))
            mFavoriteList << eachElement;
    }
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




void UBLibraryController::refreshShapeThumbnailsView() {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QStringList shapePaths =
            UBPersistenceManager::persistenceManager()->allShapes();

    mSvgItemToFilepath.clear();

    QList<QGraphicsItem*> items;
    QList<QUrl> itemsPath;

    for (int i = 0; i < shapePaths.size(); i++) {
        QString path = shapePaths.at(i);

        UBThumbnailSvg *svgItem = new UBThumbnailSvg(path);
        mSvgItemToFilepath.insert(svgItem, path);

        items.append(svgItem);
        itemsPath.append(QUrl::fromLocalFile(path));
    }

    mLibraryUI->shapeThumbnailView->setGraphicsItems(items, itemsPath);

    QApplication::restoreOverrideCursor();
}


void UBLibraryController::refreshImageThumbnailsView() {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    mSvgImageItemToFilepath.clear();
    mPixmapItemToFilepath.clear();

    QList<QGraphicsItem*> items;
    QList<QUrl> itemsPath;

    QList<QTreeWidgetItem *> tw =
            mLibraryUI->imageLibraryTreeWidget->selectedItems();

    if (tw.size() > 0) {
        UBLibraryFolderItem* fi = dynamic_cast<UBLibraryFolderItem*> (tw.last());

        if (fi) {
            QStringList
                    imagePaths =
                    UBPersistenceManager::persistenceManager()->allImages(
                            fi->dir());

            for (int i = 0; i < imagePaths.size(); i++) {
                QString path = imagePaths.at(i);

                if (!path.contains(".thumbnail.")) {
                    if (path.endsWith("svg", Qt::CaseInsensitive) || path.endsWith(
                            "svgz", Qt::CaseInsensitive)) {
                        UBThumbnailSvg *svgItem = new UBThumbnailSvg(path);

                        items.append(svgItem);
                        itemsPath.append(QUrl::fromLocalFile(path));
                        mSvgImageItemToFilepath.insert(svgItem, path);
                    } else {
                        // The image is not a thumbnail ...
                        QString thumbnailPath = UBFileSystemUtils::thumbnailPath(
                                path);
                        if (!QFileInfo(thumbnailPath).exists()) {
                            // ... and the thumbnail does not exist
                            QPixmap pix(path);
                            if (!pix.isNull()) {
                                // actually create the thumbnail
                                pix = pix.scaledToWidth(
                                        qMin(UBSettings::maxThumbnailWidth,
                                             pix.width()),
                                        Qt::SmoothTransformation);
                                pix.save(thumbnailPath);

                                QGraphicsPixmapItem *pixmapItem =
                                        new UBThumbnailPixmap(pix);

                                items.append(pixmapItem);
                                itemsPath.append(QUrl::fromLocalFile(path));
                                mPixmapItemToFilepath.insert(pixmapItem, path);
                            }
                        }
                        UBPlatformUtils::hideFile(thumbnailPath);
                    }
                } else {
                    // The image is a thumbnail
                    QString originalImagePath(path);
                    originalImagePath.remove(".thumbnail");

                    if (!QFileInfo(originalImagePath).exists()) {
                        // Skip images that have a thumbnail only but no corresponding original image
                        continue;
                    }

                    QPixmap pix(path);

                    if (!pix.isNull()) {
                        QGraphicsPixmapItem *pixmapItem =
                                new UBThumbnailPixmap(pix); //deleted while replaced in the scene or by the scene destruction

                        items.append(pixmapItem);
                        itemsPath.append(QUrl::fromLocalFile(originalImagePath));
                        mPixmapItemToFilepath.insert(pixmapItem, originalImagePath);
                    }
                }
            }
        }
    }

    mLibraryUI->imageThumbnailView->setGraphicsItems(items, itemsPath);

    QApplication::restoreOverrideCursor();
}

void UBLibraryController::createNewFolder() {
    QTreeWidget* currentTreeWidget = 0;
    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image:
        currentTreeWidget = mLibraryUI->imageLibraryTreeWidget;
        break;
    case TabIndex::Interactive:
        currentTreeWidget = mLibraryUI->interactiveLibraryTreeWidget;
        break;
    }

    if (currentTreeWidget != 0) {
        QList<QTreeWidgetItem *> tw = currentTreeWidget->selectedItems();

        if (tw.size() > 0) {
            UBLibraryFolderItem* fi = dynamic_cast<UBLibraryFolderItem*> (tw.at(0));

            if (fi) {
                QString newFolderPath = fi->dir().path() + "/" + tr("New Folder");

                int count = 1;

                if (QDir(newFolderPath).exists()) {
                    while (QDir(newFolderPath + QString(" %1").arg(count)).exists()) {
                        count++;
                    }

                    newFolderPath = newFolderPath + QString(" %1").arg(count);
                }

                UBLibraryFolderItem* newFi = new UBLibraryFolderItem(QDir(
                        newFolderPath), QString(), fi, true, QStringList());

                currentTreeWidget->expandItem(fi);
                currentTreeWidget->setCurrentItem(newFi);
            }
        }
    }
}

QStringList UBLibraryController::addItemsToCurrentLibrary(
        const QDir& pSelectedFolder, const QStringList& pExtensions) {
    QString extensions;

    foreach(QString ext, pExtensions)
    {
        extensions += " *.";
        extensions += ext;
    }

    QString defaultPath =
            UBSettings::settings()->lastImportToLibraryPath->get().toString();
    QStringList filenames = QFileDialog::getOpenFileNames(mParentWidget, tr(
            "Add Item to Library"), defaultPath, tr("All Supported (%1)").arg(
                    extensions));

    QStringList importedFiles;
    if (filenames.size() == 0)
        return filenames;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    UBApplication::processEvents();

    int i = 1;
    int count = filenames.count();

    foreach(const QString filename, filenames)
    {
        QFileInfo source(filename);

        // we update last import location. We do it only once at the first imported file
        if (i == 1) {
            UBSettings::settings()->lastImportToLibraryPath->set(QVariant(
                    source.absolutePath()));
        }

        if (source.isDir()) {
            QString target = pSelectedFolder.path() + "/"
                             + QFileInfo(source).fileName();
            UBFileSystemUtils::copyDir(source.absoluteFilePath(), target);
            importedFiles << target;
        } else if (source.isFile() && (filename.toLower().endsWith(".wgt")
            || filename.toLower().endsWith(".wdgt")
            || filename.toLower().endsWith(".pwgt"))) {
            QString baseFileName = source.fileName();

            QString filePath = pSelectedFolder.path() + "/" + baseFileName;
            QString nextItemPath = UBFileSystemUtils::nextAvailableFileName(
                    filePath);

            QFile file(filename);

            UBFileSystemUtils::expandZipToDir(file, nextItemPath);
            importedFiles << nextItemPath;
        } else if (source.isFile() && filename.toLower().endsWith(".pdf")) {
            XPDFRenderer pdfRenderer(filename);

            if (pdfRenderer.isValid()) {
                QString title = pdfRenderer.title();

                if (title.length() == 0)
                    title = QFileInfo(source).completeBaseName();

                for (int i = 0; i < pdfRenderer.pageCount(); i++) {
                    UBApplication::showMessage(tr(
                            "Copying page %1 of file %2 to library %3") .arg(i
                                                                             + 1) .arg(title) .arg(pSelectedFolder.dirName()));

                    QString filePath = pSelectedFolder.path() + "/" + title
                                       + QString(" %1 %2.png").arg(tr("Page")).arg(i + 1);
                    QSize size = pdfRenderer.pageSizeF(i + 1).toSize();
                    QImage image(size, QImage::Format_RGB32);
                    QPainter p(&image);

                    p.setRenderHint(QPainter::SmoothPixmapTransform);
                    p.setRenderHint(QPainter::Antialiasing);

                    pdfRenderer.render(&p, i + 1, QRectF(0, 0, size.width(),
                                                         size.height()));
                    p.end();

                    image.save(filePath);

                    importedFiles << filePath;
                }
            } else {
                qWarning() << "Failed to load file " << filename << "as PDF";
            }
        } else {
            QString target = pSelectedFolder.path() + "/"
                             + QFileInfo(source).fileName();
            QFile sourceFile(source.absoluteFilePath());
            UBApplication::showMessage(
                    tr("Copying file %1 to library %2 (%3/%4)").arg(QFileInfo(
                            source).fileName()).arg(pSelectedFolder.dirName()).arg(
                                    i).arg(count));
            sourceFile.copy(target);
            importedFiles << target;
        }

        i++;
    }

    UBApplication::showMessage(tr("Finished copying %1 files ...").arg(
            filenames.size()));

    QApplication::restoreOverrideCursor();
    return importedFiles;
}

void UBLibraryController::addInteractivesToLibrary() {
    QList<QTreeWidgetItem *> tw =
            mLibraryUI->interactiveLibraryTreeWidget->selectedItems();

    if (tw.size() > 0) {
        UBLibraryFolderItem* fi = dynamic_cast<UBLibraryFolderItem*> (tw.at(0));

        if (fi) {
            addItemsToCurrentLibrary(fi->dir(),
                                     UBSettings::settings()->widgetFileExtensions);

            refreshInteractiveThumbnailsView(); // TODO UB 4.x optimisation : could do better than refreshing the entire directory
        }
    }
}

void UBLibraryController::addImagesToLibrary() {
    QList<QTreeWidgetItem*> tw =
            mLibraryUI->imageLibraryTreeWidget->selectedItems();

    if (tw.size() > 0) {
        UBLibraryFolderItem* fi = dynamic_cast<UBLibraryFolderItem*> (tw.at(0));

        if (fi) {
            QStringList extensions = UBSettings::imageFileExtensions;
            extensions << "pdf";

            QStringList filenames = addItemsToCurrentLibrary(fi->dir(), extensions);

            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            UBApplication::processEvents();

            quint32 i = 1, count = filenames.count();
            foreach(const QString filename, filenames)
            {
                // Create thumbnail
                // For large images, what actually takes time is the creation of the thumbnail, the copy is fast
                QPixmap pix(filename);
                UBApplication::showMessage(
                        tr("Creating thumbnail %1 (%2/%3)").arg(filename).arg(i).arg(
                                count));
                QString thumbnailPath = UBFileSystemUtils::thumbnailPath(
                        filename);
                pix.scaledToWidth(qMin(UBSettings::maxThumbnailWidth,
                                       pix.width()), Qt::SmoothTransformation).save(
                                               thumbnailPath);
                UBPlatformUtils::hideFile(thumbnailPath);
                i++;
            }

            refreshImageThumbnailsView(); // TODO UB 4.x optimisation : could do better than refreshing the entire directory
            QApplication::restoreOverrideCursor();
        }
    }
}

void UBLibraryController::removeDir() {
    QTreeWidget* currentTreeWidget = 0;
    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image:
        currentTreeWidget = mLibraryUI->imageLibraryTreeWidget;
        break;
    case TabIndex::Interactive:
        currentTreeWidget = mLibraryUI->interactiveLibraryTreeWidget;
        break;
    }

    if (currentTreeWidget != 0) {
        QList<QTreeWidgetItem *> tw = currentTreeWidget->selectedItems();

        if (tw.size() > 0) {
            UBLibraryFolderItem* fi =
                    dynamic_cast<UBLibraryFolderItem*> (tw.last());

            if (fi && fi->canWrite()) {
                if (QMessageBox::question(
                        0,
                        tr("Remove Folder"),
                        tr(
                                "Are you sure you want to remove the folder %1 from the library?").arg(
                                        fi->dir().dirName()), QMessageBox::Yes
                        | QMessageBox::No) == QMessageBox::Yes) {
                    qDebug() << "deleting dir" << fi->dir().path();

                    UBFileSystemUtils::deleteDir(fi->dir().path());
                    fi->parent()->removeChild(fi);
                }
            }
        }
    }
}

void UBLibraryController::remove() {
    UBThumbnailWidget* currentThumbnailView = 0;
    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image:
        currentThumbnailView = mLibraryUI->imageThumbnailView;
        break;
    case TabIndex::Interactive:
        currentThumbnailView = mLibraryUI->interactiveThumbnailView;
        break;
    }

    if (currentThumbnailView) {
        if (currentThumbnailView->selectedItems().size() > 0) {
            removeItemsFromLibrary(currentThumbnailView);
        } else {
            removeDir();
        }
    }
}

void UBLibraryController::removeItemsFromLibrary(
        UBThumbnailWidget* pThumbnailView) {
    QList<QGraphicsItem*> selectedItems = pThumbnailView->selectedItems();

    if (selectedItems.size() > 0) {
        if (QMessageBox::question(0, tr("Remove Item"), tr(
                "Are you sure you want to remove the item from the library?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            QGraphicsPixmapItem* pixmapItem =
                    dynamic_cast<QGraphicsPixmapItem*> (selectedItems.last());

            if (pixmapItem && mPixmapItemToFilepath.contains(pixmapItem)) {
                QString path = mPixmapItemToFilepath.take(pixmapItem);
                QFile file(path);
                QFile thumbnailFile(UBFileSystemUtils::thumbnailPath(path));

                file.remove();
                thumbnailFile.remove();

                refreshImageThumbnailsView();
            } else if (pixmapItem
                       && mInteractiveItemToFilepath.contains(pixmapItem)) {
                QString path = mInteractiveItemToFilepath.take(pixmapItem);

                //is it a tool ?
                if (path.startsWith("uniboardTool://")) {
                    QStringList
                            uris =
                            UBSettings::settings()->favoritesNativeToolUris->get().toStringList();
                    if (uris.contains(path)) {
                        uris.removeAll(path);
                        UBSettings::settings()->favoritesNativeToolUris->set(uris);
                    }
                } else {
                    qDebug() << "file to delete " << path;
                    QFileInfo fileInfo(path);
                    if (fileInfo.isDir()) {
                        UBFileSystemUtils::deleteDir(fileInfo.absoluteFilePath());
                    } else {
                        QFile(fileInfo.absoluteFilePath()).remove();
                    }
                }
                refreshInteractiveThumbnailsView();
            } else {
                QGraphicsSvgItem* svgItem =
                        dynamic_cast<QGraphicsSvgItem*> (selectedItems.last());

                if (svgItem && mSvgImageItemToFilepath.contains(svgItem)) {
                    QString path = mSvgImageItemToFilepath.take(svgItem);
                    QFile file(path);

                    file.remove();

                    refreshImageThumbnailsView();
                }
            }
        }
    }
}

void UBLibraryController::refreshInteractiveThumbnailsView()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> items;
    QList<QUrl> itemsPath;
    QStringList labels;

    QList<QTreeWidgetItem *> tw =
            mLibraryUI->interactiveLibraryTreeWidget->selectedItems();

    if (tw.size() > 0) {
        UBLibraryFolderItem* fi = dynamic_cast<UBLibraryFolderItem*> (tw.last());

        if (fi) {
            QStringList toolUris;

            if (fi == mInteractiveUniboardTi) {
                toolUris = UBToolsManager::manager()->allToolIDs();
            }

            foreach(QString toolUri, toolUris)
            {
                UBToolsManager::UBToolDescriptor tool =
                        UBToolsManager::manager()->toolByID(toolUri);

                QGraphicsPixmapItem *pixmapItem = new UBThumbnailPixmap(
                        tool.icon);

                if (!pixmapItem->pixmap().isNull()) {
                    QString id = tool.id;

                    mInteractiveItemToFilepath.insert(pixmapItem, id);
                    items.append(pixmapItem);
                    itemsPath.append(QUrl(id));
                    labels.append(tool.label + " " + tool.version);
                }
            }

            QStringList widgetPaths =
                    UBPersistenceManager::persistenceManager()->allWidgets(
                            fi->dir());

            for (int i = 0; i < widgetPaths.size(); i++) {
                QString path = widgetPaths.at(i);

                QString iconFilepath = UBAbstractWidget::iconFilePath(
                        QUrl::fromLocalFile(path));

                QGraphicsPixmapItem *pixmapItem = new UBThumbnailPixmap(QPixmap(
                        iconFilepath));

                mInteractiveItemToFilepath.insert(pixmapItem, path);

                QString name = UBAbstractWidget::widgetName(QUrl::fromLocalFile(
                        path));

                items.append(pixmapItem);
                itemsPath.append(QUrl::fromLocalFile(path));
                labels.append(name);
            }
        }
    }
    mLibraryUI->interactiveThumbnailView->setGraphicsItems(items, itemsPath, labels);

    QApplication::restoreOverrideCursor();
}

void UBLibraryController::showLibraryDialog(bool show)
{
    if (show && !mLibraryWindow) {
        Qt::WindowFlags flag = Qt::Dialog | Qt::WindowMaximizeButtonHint
                               | Qt::WindowCloseButtonHint;
        flag &= ~Qt::WindowContextHelpButtonHint;
        flag &= ~Qt::WindowMinimizeButtonHint;

        mLibraryWindow = new QDialog(mParentWidget, flag);

        mLibraryUI = new Ui::library();
        mLibraryUI->setupUi(mLibraryWindow);
        mLibraryUI->libraryTab->setCurrentIndex(TabIndex::Gip);

        // image tab
        QString ubPictures =
                UBSettings::settings()->uniboardImageLibraryDirectory();
        QString userPictures = UBSettings::settings()->uniboardUserImageDirectory();

        UBLibraryFolderItem *ubLibrary = new UBLibraryFolderItem(QDir(ubPictures),
                                                                 tr("Sankore 3.1"), 0, false, QStringList());
        UBLibraryFolderItem *userLibrary = new UBLibraryFolderItem(QDir(
                userPictures), tr("My Images"), 0, true, QStringList());

        mLibraryUI->imageLibraryTreeWidget->addTopLevelItem(userLibrary);
        mLibraryUI->imageLibraryTreeWidget->addTopLevelItem(ubLibrary);

        QStringList imageMimeTypes;
        imageMimeTypes << "image";
        mLibraryUI->imageLibraryTreeWidget->setMimeTypes(imageMimeTypes);
        mLibraryUI->imageLibraryTreeWidget->setFileExtensions(
                UBSettings::imageFileExtensions);

        ubLibrary->setIcon(0, QIcon(":/images/myContent.png"));
        ubLibrary->setExpanded(true);

        userLibrary->setIcon(0, QIcon(":/images/myFolder.png"));
        userLibrary->setExpanded(true);

        if (userLibrary->childCount() > 0) {
            foreach(QTreeWidgetItem* ti, mLibraryUI->imageLibraryTreeWidget->selectedItems())
            {
                ti->setSelected(false);
            }

            userLibrary->child(0)->setSelected(true);

            selectionChanged();
        } else {
            ubLibrary->setSelected(true);
        }

        mLibraryUI->imageLibraryTreeWidget->setRootIsDecorated(false);
        mLibraryUI->imageLibrariesStack->setCurrentWidget(
                mLibraryUI->imagePageLocal);

        mImageWebView = new UBLibraryWebView(mLibraryUI->imagePageOnline);
        mLibraryUI->imagePageOnline->layout()->addWidget(mImageWebView);

        // Interactive tab
        QString ubInteractive =
                UBSettings::settings()->uniboardInteractiveLibraryDirectory();
        QString userInteractive =
                UBSettings::settings()->uniboardInteractiveUserDirectory();
        QString ubInteractiveFavorites =
                UBSettings::settings()->uniboardInteractiveFavoritesDirectory();

        mInteractiveUniboardTi = new UBLibraryFolderItem(QDir(ubInteractive), tr(
                "Sankore 3.1"), 0, false,
                                                         UBSettings::interactiveContentFileExtensions);

        UBLibraryFolderItem *userInteractiveLibrary = new UBLibraryFolderItem(QDir(
                userInteractive), tr("My Applications"), 0, true,
                                                                              UBSettings::interactiveContentFileExtensions);

        mLibraryUI->interactiveLibraryTreeWidget->addTopLevelItem(
                userInteractiveLibrary);
        mLibraryUI->interactiveLibraryTreeWidget->addTopLevelItem(
                mInteractiveUniboardTi);

        QStringList icMimeTypes;
        icMimeTypes << "application/widget";
        icMimeTypes << "application/x-shockwave-flash";
        mLibraryUI->interactiveLibraryTreeWidget->setMimeTypes(icMimeTypes);

        mLibraryUI->interactiveLibraryTreeWidget->setFileExtensions(
                UBSettings::interactiveContentFileExtensions);

        QStringList icProtocoles;
        icProtocoles << "uniboardTool://";
        mLibraryUI->interactiveLibraryTreeWidget->setProtocoles(icProtocoles);

        mInteractiveUniboardTi->setIcon(0, QIcon(":/images/myContent.png"));
        mInteractiveUniboardTi->setExpanded(true);

        userInteractiveLibrary->setIcon(0, QIcon(":/images/myFolder.png"));
        userInteractiveLibrary->setExpanded(true);

        mInteractiveUniboardTi->setSelected(true);

        mLibraryUI->interactiveLibraryTreeWidget->setRootIsDecorated(false);
        mLibraryUI->interactiveLibrariesStack->setCurrentWidget(
                mLibraryUI->interactivePageLocal);

        mInteractiveWebView = new UBLibraryWebView(
                mLibraryUI->interactivePageOnline);
        mLibraryUI->interactivePageOnline->layout()->addWidget(mInteractiveWebView);

        // video tab

        QString userVideos = UBSettings::settings()->uniboardUserVideoDirectory();

        UBLibraryFolderItem *userVideoLibrary = new UBLibraryFolderItem(QDir(
                userVideos), tr("My Movies"), 0, false, QStringList());

        mLibraryUI->videoLibraryTreeWidget->addTopLevelItem(userVideoLibrary);
        userVideoLibrary->setIcon(0, QIcon(":/images/myFolder.png"));
        userVideoLibrary->setExpanded(true);
        userVideoLibrary->setSelected(true);

        mLibraryUI->videoLibraryTreeWidget->setRootIsDecorated(false);

        QStringList videoMimeTypes;
        videoMimeTypes << "video";
        mLibraryUI->videoLibraryTreeWidget->setMimeTypes(videoMimeTypes);

        mLibraryUI->videoLibrariesStack->setCurrentWidget(
                mLibraryUI->videoPageLocal);

        mVideoWebView = new UBLibraryWebView(mLibraryUI->videoPageOnline);
        mLibraryUI->videoPageOnline->layout()->addWidget(mVideoWebView);

        mLibraryUI->imageThumbnailView->setThumbnailWidth(
                UBSettings::settings()->imageThumbnailWidth->get().toInt());
        mLibraryUI->gipThumbnailView->setThumbnailWidth(UBSettings::defaultGipWidth);
        mLibraryUI->soundThumbnailView->setThumbnailWidth(
                UBSettings::defaultSoundWidth);
        mLibraryUI->shapeThumbnailView->setThumbnailWidth(
                UBSettings::settings()->shapeThumbnailWidth->get().toInt());
        mLibraryUI->interactiveThumbnailView->setThumbnailWidth(
                UBSettings::defaultWidgetIconWidth);
        mLibraryUI->videoThumbnailView->setThumbnailWidth(
                UBSettings::settings()->videoThumbnailWidth->get().toInt());

        connect(mLibraryUI->imageLibraryTreeWidget, SIGNAL(itemSelectionChanged()),
                this, SLOT(itemSelectionChanged()));

        connect(mLibraryUI->imageLibraryTreeWidget,
                SIGNAL(itemChanged ( QTreeWidgetItem * , int )), this,
                SLOT(itemChanged ( QTreeWidgetItem * , int )));

        connect(mLibraryUI->imageLibraryTreeWidget, SIGNAL(contentChanged()), this,
                SLOT(refreshImageThumbnailsView()));

        connect(mLibraryUI->interactiveLibraryTreeWidget,
                SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));

        connect(mLibraryUI->interactiveLibraryTreeWidget,
                SIGNAL(itemChanged ( QTreeWidgetItem * , int )), this,
                SLOT(itemChanged ( QTreeWidgetItem * , int )));

        connect(mLibraryUI->interactiveLibraryTreeWidget, SIGNAL(contentChanged()),
                this, SLOT(refreshInteractiveThumbnailsView()));

        connect(mLibraryUI->videoLibraryTreeWidget, SIGNAL(itemSelectionChanged()),
                this, SLOT(itemSelectionChanged()));

        connect(mLibraryUI->videoLibraryTreeWidget, SIGNAL(contentChanged()), this,
                SLOT(refreshVideoThumbnailsView()));

        connect(mLibraryUI->newInteractiveFolderButton, SIGNAL(clicked()), this,
                SLOT(createNewFolder()));
        connect(mLibraryUI->newFolderButton, SIGNAL(clicked()), this,
                SLOT(createNewFolder()));
        connect(mLibraryUI->addImageToLibraryButton, SIGNAL(clicked()), this,
                SLOT(addImagesToLibrary()));
        connect(mLibraryUI->addInteractiveToLibraryButton, SIGNAL(clicked()), this,
                SLOT(addInteractivesToLibrary()));
        connect(mLibraryUI->addToDocumentButton, SIGNAL(clicked()), this,
                SLOT(addObjectFromFilesystemToPage()));
        connect(mLibraryUI->removeButton, SIGNAL(clicked()), this, SLOT(remove()));

        connect(mLibraryUI->addToPageButton, SIGNAL(clicked()), this,
                SLOT(addToPage()));
        connect(mLibraryUI->setAsBackgroundButton, SIGNAL(clicked()), this,
                SLOT(setAsBackground()));
        connect(mLibraryUI->removeBackgroundButton, SIGNAL(clicked()), this,
                SLOT(removeBackground()));

        connect(mLibraryUI->zoomSlider, SIGNAL(valueChanged(int)), this,
                SLOT(zoomSliderValueChanged(int)));

        connect(mLibraryUI->imageThumbnailView, SIGNAL(resized()), this,
                SLOT(thumbnailViewResized()));
        connect(mLibraryUI->shapeThumbnailView, SIGNAL(resized()), this,
                SLOT(thumbnailViewResized()));
        connect(mLibraryUI->interactiveThumbnailView, SIGNAL(resized()), this,
                SLOT(thumbnailViewResized()));
        connect(mLibraryUI->videoThumbnailView, SIGNAL(resized()), this,
                SLOT(thumbnailViewResized()));
        connect(mLibraryUI->gipThumbnailView, SIGNAL(resized()), this,
                SLOT(thumbnailViewResized()));
        connect(mLibraryUI->soundThumbnailView, SIGNAL(resized()), this,
                SLOT(thumbnailViewResized()));

        connect(mLibraryUI->imageThumbnailView, SIGNAL(selectionChanged()), this,
                SLOT(selectionChanged()));
        connect(mLibraryUI->shapeThumbnailView, SIGNAL(selectionChanged()), this,
                SLOT(selectionChanged()));
        connect(mLibraryUI->interactiveThumbnailView, SIGNAL(selectionChanged()),
                this, SLOT(selectionChanged()));
        connect(mLibraryUI->videoThumbnailView, SIGNAL(selectionChanged()), this,
                SLOT(selectionChanged()));
        connect(mLibraryUI->gipThumbnailView, SIGNAL(selectionChanged()), this,
                SLOT(selectionChanged()));
        connect(mLibraryUI->soundThumbnailView, SIGNAL(selectionChanged()), this,
                SLOT(selectionChanged()));

        mLibraryUI->imageLibraryTreeWidget->setIndentation(30);
        mLibraryUI->interactiveLibraryTreeWidget->setIndentation(30);
        mLibraryUI->videoLibraryTreeWidget->setIndentation(30);

        mLibraryUI->imageThumbnailView->setBackgroundBrush(
                UBSettings::documentViewLightColor);
        mLibraryUI->shapeThumbnailView->setBackgroundBrush(
                UBSettings::documentViewLightColor);
        mLibraryUI->interactiveThumbnailView->setBackgroundBrush(
                UBSettings::documentViewLightColor);
        mLibraryUI->gipThumbnailView->setBackgroundBrush(
                UBSettings::documentViewLightColor);
        mLibraryUI->videoThumbnailView->setBackgroundBrush(
                UBSettings::documentViewLightColor);
        mLibraryUI->soundThumbnailView->setBackgroundBrush(
                UBSettings::documentViewLightColor);

        int viewWidth = mParentWidget->width() / 2;
        int viewHeight = mParentWidget->height() * 2.0 / 3.0;

        mLibraryWindow->setGeometry((mParentWidget->width() - viewWidth) / 2,
                                    (mParentWidget->height() - viewHeight) / 2, viewWidth, viewHeight);

        connect(mLibraryUI->libraryTab, SIGNAL(currentChanged(int)), this,
                SLOT(tabChanged(int)));

        mLibraryUI->libraryTab->setCurrentIndex(TabIndex::Gip);
        tabChanged(TabIndex::Gip);

        connect(mLibraryWindow, SIGNAL(finished(int)), this,
                SIGNAL(dialogClosed(int)));

        loadLibraries();
    }

    if (mLibraryWindow) {
        mLibraryWindow->setVisible(show);
        if (show) {
            mLibraryWindow->activateWindow();

            if (mNeedRefreshOnNextDisplay) {
                refreshImageThumbnailsView();
                mNeedRefreshOnNextDisplay = false;
            }
        }
    }

    selectionChanged();
}

void UBLibraryController::tabChanged(int value) {
    selectionChanged();

    switch (value) {
    case TabIndex::Image:
        refreshImageThumbnailsView();
        break;

    case TabIndex::Shape:
        refreshShapeThumbnailsView();
        break;

    case TabIndex::Interactive:
        refreshInteractiveThumbnailsView();
        break;

    case TabIndex::Video:
        refreshVideoThumbnailsView();
        break;

    case TabIndex::Sound:
        refreshSoundThumbnailsView();
        break;

    default:
        break;
    }
}

void UBLibraryController::selectionChanged() {
    bool hasSelectedItems = false;
    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image: {
            bool canWrite = false;
            bool isTopLevel = false;

            QList<QTreeWidgetItem *> tw =
                    mLibraryUI->imageLibraryTreeWidget->selectedItems();
            if (tw.size() > 0) {
                UBLibraryFolderItem* fi =
                        dynamic_cast<UBLibraryFolderItem*> (tw.last());
                if (fi) {
                    canWrite = fi->canWrite();
                    isTopLevel = !fi->parent();

                    mLibraryUI->imageLibrariesStack->setCurrentWidget(
                            mLibraryUI->imagePageLocal);
                } else {

                    UBOnlineLibraryItem* oi =
                            dynamic_cast<UBOnlineLibraryItem*> (tw.last());
                    if (oi) {

                        mLibraryUI->imageLibrariesStack->setCurrentWidget(
                                mLibraryUI->imagePageOnline);
                        mImageWebView->load(oi->url(), oi);
                    }
                }
            }

            mLibraryUI->newFolderButton->setEnabled(canWrite && isTopLevel);
            mLibraryUI->addImageToLibraryButton->setEnabled(canWrite && !isTopLevel);
            mLibraryUI->addToDocumentButton->setEnabled(true);

            mLibraryUI->removeButton->setEnabled(canWrite && !isTopLevel);

            hasSelectedItems = mLibraryUI->imageThumbnailView->selectedItems().size()
                               > 0;

            mLibraryUI->addToPageButton->setEnabled(hasSelectedItems);
            mLibraryUI->setAsBackgroundButton->setEnabled(hasSelectedItems);

            mLibraryUI->removeBackgroundButton->setEnabled(
                    activeScene()->hasBackground());

            mLibraryUI->zoomSlider->setValue(
                    mLibraryUI->imageThumbnailView->thumbnailWidth());
            mLibraryUI->zoomSlider->setVisible(true);
        }
        break;
    case TabIndex::Shape: {
            mLibraryUI->newFolderButton->setEnabled(false);
            mLibraryUI->addImageToLibraryButton->setEnabled(false);
            mLibraryUI->addToDocumentButton->setEnabled(false);
            mLibraryUI->removeButton->setEnabled(false);

            hasSelectedItems = mLibraryUI->shapeThumbnailView->selectedItems().size()
                               > 0;

            mLibraryUI->addToPageButton->setEnabled(hasSelectedItems);
            mLibraryUI->setAsBackgroundButton->setEnabled(hasSelectedItems);

            mLibraryUI->removeBackgroundButton->setEnabled(
                    activeScene()->hasBackground());

            mLibraryUI->zoomSlider->setValue(
                    mLibraryUI->shapeThumbnailView->thumbnailWidth());
            mLibraryUI->zoomSlider->setVisible(true);
        }
        break;
    case TabIndex::Interactive: {
            bool canWrite = false;
            bool isTopLevel = false;

            QList<QTreeWidgetItem *> tw =
                    mLibraryUI->interactiveLibraryTreeWidget->selectedItems();
            if (tw.size() > 0) {
                UBLibraryFolderItem* fi =
                        dynamic_cast<UBLibraryFolderItem*> (tw.last());
                if (fi) {
                    canWrite = fi->canWrite();
                    isTopLevel = !fi->parent();
                    mLibraryUI->interactiveLibrariesStack->setCurrentWidget(
                            mLibraryUI->interactivePageLocal);
                } else {
                    UBOnlineLibraryItem* oi =
                            dynamic_cast<UBOnlineLibraryItem*> (tw.last());

                    if (oi) {
                        mLibraryUI->interactiveLibrariesStack->setCurrentWidget(
                                mLibraryUI->interactivePageOnline);
                        mInteractiveWebView->load(oi->url(), oi);
                    }
                }
            }

            mLibraryUI->newInteractiveFolderButton->setEnabled(canWrite && isTopLevel);
            mLibraryUI->addInteractiveToLibraryButton->setEnabled(canWrite
                                                                  && !isTopLevel);
            mLibraryUI->addToDocumentButton->setEnabled(true);

            hasSelectedItems
                    = mLibraryUI->interactiveThumbnailView->selectedItems().size() > 0;

            mLibraryUI->removeButton->setEnabled((canWrite && !isTopLevel) || (canWrite
                                                                               && hasSelectedItems));

            mLibraryUI->addToPageButton->setEnabled(hasSelectedItems);
            mLibraryUI->setAsBackgroundButton->setEnabled(false);
            mLibraryUI->removeBackgroundButton->setEnabled(false);

            mLibraryUI->zoomSlider->setValue(
                    mLibraryUI->interactiveThumbnailView->thumbnailWidth());
            mLibraryUI->zoomSlider->setVisible(false);
        }
        break;
    case TabIndex::Video: {
            QList<QTreeWidgetItem *> tw =
                    mLibraryUI->videoLibraryTreeWidget->selectedItems();
            if (tw.size() > 0) {
                hasSelectedItems = true;
                UBOnlineLibraryItem* oi =
                        dynamic_cast<UBOnlineLibraryItem*> (tw.last());
                if (oi) {
                    mLibraryUI->videoLibrariesStack->setCurrentWidget(
                            mLibraryUI->videoPageOnline);
                    mVideoWebView->load(oi->url(), oi);
                } else {
                    mLibraryUI->videoLibrariesStack->setCurrentWidget(
                            mLibraryUI->videoPageLocal);
                }
            }

            mLibraryUI->newFolderButton->setEnabled(false);
            mLibraryUI->addImageToLibraryButton->setEnabled(false);
            mLibraryUI->addToDocumentButton->setEnabled(true);
            mLibraryUI->removeButton->setEnabled(false);
            mLibraryUI->addToPageButton->setEnabled(true);
            mLibraryUI->setAsBackgroundButton->setEnabled(false);
            mLibraryUI->removeBackgroundButton->setEnabled(false);

            mLibraryUI->zoomSlider->setValue(mLibraryUI->videoThumbnailView->thumbnailWidth());
            mLibraryUI->zoomSlider->setVisible(true);
        }
        break;
    case TabIndex::Gip: {
            hasSelectedItems = mLibraryUI->gipThumbnailView->selectedItems().size() > 0;

            mLibraryUI->newFolderButton->setEnabled(false);
            mLibraryUI->addImageToLibraryButton->setEnabled(false);
            mLibraryUI->addToDocumentButton->setEnabled(false);
            mLibraryUI->removeButton->setEnabled(false);
            mLibraryUI->addToPageButton->setEnabled(hasSelectedItems);
            mLibraryUI->setAsBackgroundButton->setEnabled(false);
            mLibraryUI->removeBackgroundButton->setEnabled(false);
            mLibraryUI->zoomSlider->setValue(mLibraryUI->gipThumbnailView->thumbnailWidth());
            mLibraryUI->zoomSlider->setVisible(true);
        }
        break;

    case TabIndex::Sound: {
            hasSelectedItems = mLibraryUI->soundThumbnailView->selectedItems().size()
                               > 0;

            mLibraryUI->newFolderButton->setEnabled(false);
            mLibraryUI->addImageToLibraryButton->setEnabled(false);
            mLibraryUI->addToDocumentButton->setEnabled(false);
            mLibraryUI->removeButton->setEnabled(false);
            mLibraryUI->addToPageButton->setEnabled(hasSelectedItems);
            mLibraryUI->setAsBackgroundButton->setEnabled(false);
            mLibraryUI->removeBackgroundButton->setEnabled(false);
            mLibraryUI->zoomSlider->setValue(mLibraryUI->soundThumbnailView->thumbnailWidth());
            mLibraryUI->zoomSlider->setVisible(true);
        }
    default:
        break;
    }
}

void UBLibraryController::zoomSliderValueChanged(int value) {
    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image:
        mLibraryUI->imageThumbnailView->setThumbnailWidth(value);
        UBSettings::settings()->imageThumbnailWidth->set(value);
        break;
    case TabIndex::Shape:
        mLibraryUI->shapeThumbnailView->setThumbnailWidth(value);
        UBSettings::settings()->shapeThumbnailWidth->set(value);
        break;
    case TabIndex::Interactive:
        mLibraryUI->interactiveThumbnailView->setThumbnailWidth(value);
        break;
    case TabIndex::Video:
        mLibraryUI->videoThumbnailView->setThumbnailWidth(value);
        UBSettings::settings()->videoThumbnailWidth->set(value);
        break;
    case TabIndex::Gip:
        mLibraryUI->gipThumbnailView->setThumbnailWidth(value);
        UBSettings::settings()->gipThumbnailWidth->set(value);
        break;
    case TabIndex::Sound:
        mLibraryUI->soundThumbnailView->setThumbnailWidth(value);
        UBSettings::settings()->soundThumbnailWidth->set(value);
        break;
    default:
        break;
    }
}

void UBLibraryController::addShape() {
    QPointF pos(0, 0);

    if (activeScene()) {
        QPointF topLeftCorner = mBoardController->controlGeometry().topLeft();
        pos = mBoardController->controlView()->mapToScene(topLeftCorner.toPoint());
    }

    int i = 0;

    foreach(QGraphicsItem *item, mLibraryUI->shapeThumbnailView->selectedItems())
    {
        ++i;

        QGraphicsSvgItem* svgItem = dynamic_cast<QGraphicsSvgItem*> (item);

        if (svgItem && mSvgItemToFilepath.contains(svgItem) && activeScene()) {
            QUrl url = QUrl::fromLocalFile(mSvgItemToFilepath.value(svgItem));
            QGraphicsItem* itemInScene = activeScene()->addSvg(url, QPointF(0,
                                                                            0));

            itemInScene->setPos(QPoint(pos.x() + 10 * i, pos.y() + 10 * i));
        }
    }
}

void UBLibraryController::setShapeAsBackground() {
    QList<QGraphicsItem*> selectedItems =
            mLibraryUI->shapeThumbnailView->selectedItems();

    if (selectedItems.size() > 0) {
        QGraphicsSvgItem* svgItem =
                dynamic_cast<QGraphicsSvgItem*> (selectedItems.last());

        if (svgItem && mSvgItemToFilepath.contains(svgItem) && activeScene()) {
            QString path = mSvgItemToFilepath.value(svgItem);
            UBGraphicsSvgItem* boardItem = activeScene()->addSvg(
                    QUrl::fromLocalFile(path), QPointF(0, 0));
            activeScene()->setAsBackgroundObject(boardItem, true);
        }
    }
}

void UBLibraryController::addToPage() {
    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image:
        addImage();
        break;
    case TabIndex::Shape:
        addShape();
        break;
    case TabIndex::Interactive:
        addInteractiveToCurrentPage();
        break;
    case TabIndex::Video:
        addVideo();
        break;
    case TabIndex::Sound:
        addAudio();
        break;
    default:
        break;
    }

    mLibraryWindow->done(0);

    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
}


void UBLibraryController::setAsBackground()
{
    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image:
        setImageAsBackground();
        break;
    case TabIndex::Shape:
        setShapeAsBackground();
        break;
    default:
        break;
    }

    selectionChanged();
}

QRectF UBLibraryController::visibleSceneRect() {
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

void UBLibraryController::addImagesToCurrentPage(const QList<QUrl>& images) {
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
            itemInScene = activeScene()->scaleToFitDocumentSize(itemInScene,
                                                                false, UBSettings::objectInControlViewMargin);

            itemInScene->setPos(QPoint(pos.x() + 50 * mLastItemOffsetIndex,
                                       pos.y() + 50 * mLastItemOffsetIndex));
        }
    }
}

void UBLibraryController::addVideosToCurrentPage(const QList<QUrl>& videos) {
    QPointF pos = visibleSceneRect().topLeft();

    foreach(const QUrl url, videos)
    {
        mLastItemOffsetIndex++;
        mLastItemOffsetIndex = qMin(mLastItemOffsetIndex, 5);

        UBGraphicsVideoItem* itemInScene =
                UBApplication::boardController->addVideo(url, false, pos);
        itemInScene->setPos(QPoint(pos.x() + 50 * mLastItemOffsetIndex, pos.y()
                                   + 50 * mLastItemOffsetIndex));
    }
}

void UBLibraryController::addAudiosToCurrentPage(const QList<QUrl>& sounds) {
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

void UBLibraryController::addInteractivesToCurrentPage(
        const QList<QUrl>& widgets) {
    foreach(const QUrl url, widgets)
    {
        mBoardController->downloadURL(url, QPointF(0, 0));
    }
}

void UBLibraryController::addImage() {
    QList<QUrl> images;

    foreach(QGraphicsItem *item, mLibraryUI->imageThumbnailView->selectedItems())
    {
        QGraphicsPixmapItem* pixmapItem =
                dynamic_cast<QGraphicsPixmapItem*> (item);

        if (pixmapItem && activeScene()) {
            images << QUrl::fromLocalFile(mPixmapItemToFilepath.value(
                    pixmapItem));
        } else {
            QGraphicsSvgItem* svgItem = dynamic_cast<QGraphicsSvgItem*> (item);

            images << QUrl::fromLocalFile(
                    mSvgImageItemToFilepath.value(svgItem));
        }
    }

    addImagesToCurrentPage(images);
}

void UBLibraryController::addVideo() {
    QList<QUrl> urls;

    foreach(QGraphicsItem *item, mLibraryUI->videoThumbnailView->selectedItems())
    {
        UBThumbnailVideo* videoItem = dynamic_cast<UBThumbnailVideo*> (item);

        if (videoItem && activeScene()) {
            urls << videoItem->path();
        }
    }

    addVideosToCurrentPage(urls);
}

void UBLibraryController::addAudio() {
    QList<QUrl> urls;

    foreach(QGraphicsItem *item, mLibraryUI->soundThumbnailView->selectedItems())
    {
        QGraphicsPixmapItem* pixmapItem =
                dynamic_cast<QGraphicsPixmapItem*> (item);

        if (pixmapItem && activeScene()) {
            urls << QUrl::fromLocalFile(mSoundItemToFilepath.value(pixmapItem));
        }
    }

    addAudiosToCurrentPage(urls);
}

void UBLibraryController::setImageAsBackground() {
    QList<QGraphicsItem*> selectedItems =
            mLibraryUI->imageThumbnailView->selectedItems();

    if (selectedItems.size() > 0) {
        QGraphicsPixmapItem* pixmapItem =
                dynamic_cast<QGraphicsPixmapItem*> (selectedItems.last());

        if (pixmapItem && activeScene()) {
            QPixmap pix = mPixmapItemToFilepath.value(pixmapItem);
            UBGraphicsPixmapItem* boardItem = activeScene()->addPixmap(pix,
                                                                       QPointF(0, 0));
            activeScene()->setAsBackgroundObject(boardItem, true);

            return;
        }

        QGraphicsSvgItem* svgItem =
                dynamic_cast<QGraphicsSvgItem*> (selectedItems.last());

        if (svgItem && activeScene() && mSvgImageItemToFilepath.contains(svgItem)) {
            QUrl url = QUrl::fromLocalFile(mSvgImageItemToFilepath.value(svgItem));
            QGraphicsSvgItem* boardItem = activeScene()->addSvg(url, QPointF(0, 0));
            activeScene()->setAsBackgroundObject(boardItem, true, true);
        }
    }
}




void UBLibraryController::closeWindow() {
    mLibraryWindow->hide();
}

void UBLibraryController::itemSelectionChanged() {
    selectionChanged();

    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image:
        refreshImageThumbnailsView();
        break;
    case TabIndex::Interactive:
        refreshInteractiveThumbnailsView();
        break;
    case TabIndex::Video:
        refreshVideoThumbnailsView();
        break;
    case TabIndex::Sound:
        refreshSoundThumbnailsView();
        break;
    default:
        break;
    }
}

void UBLibraryController::itemChanged(QTreeWidgetItem * item, int column) {
    UBLibraryFolderItem* fi = dynamic_cast<UBLibraryFolderItem*> (item);

    if (fi) {
        QString newName = item->text(column);

        QString oldPath = fi->dir().canonicalPath();

        int lastSlashIndex = oldPath.lastIndexOf("/");

        QString newPath = oldPath.left(lastSlashIndex + 1) + newName;

        qDebug() << "renaming" << oldPath << "to" << newPath;

        if (fi->dir().rename(oldPath, newPath)) {
            fi->setDir(QDir(newPath));
        } else {
            item->setText(column, fi->dir().dirName());
        }
    }
}

void UBLibraryController::thumbnailViewResized() {
    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image:
        mLibraryUI->zoomSlider->setMaximum(mLibraryUI->imageThumbnailView->width());
        break;
    case TabIndex::Shape:
        mLibraryUI->zoomSlider->setMaximum(mLibraryUI->shapeThumbnailView->width());
        break;
    case TabIndex::Interactive:
        mLibraryUI->zoomSlider->setMaximum(
                mLibraryUI->interactiveThumbnailView->width());
        break;
    case TabIndex::Video:
        mLibraryUI->zoomSlider->setMaximum(mLibraryUI->videoThumbnailView->width());
        break;
    case TabIndex::Gip:
        mLibraryUI->zoomSlider->setMaximum(mLibraryUI->gipThumbnailView->width());
        break;
    case TabIndex::Sound:
        mLibraryUI->zoomSlider->setMaximum(mLibraryUI->soundThumbnailView->width());
        break;
    default:
        break;
    }
}

UBLibraryFolderItem::UBLibraryFolderItem(const QDir& pDir, const QString& name,
                                         QTreeWidgetItem * parent, bool canWrite,
                                         const QStringList& pExtensionsToHide) :
QTreeWidgetItem(parent), mExtensionsToHide(pExtensionsToHide), mDir(pDir),
mCanWrite(canWrite) {
    if (name.length() == 0) {
        setText(0, UBLibraryController::trUtf8(pDir.dirName().toUtf8()));
    } else {
        setText(0, UBLibraryController::trUtf8(name.toUtf8()));
    }

    if (canWrite) {
        setFlags(flags() | Qt::ItemIsEditable);
    }


    if (!mDir.exists())
        mDir.mkpath(mDir.path());

    refreshSubDirs();
}

void UBLibraryFolderItem::refreshSubDirs() {
    foreach(QTreeWidgetItem *ti, takeChildren())
        delete ti;

    QStringList subDirs = mDir.entryList(QStringList(), QDir::Dirs
                                         | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name); // TODO UB 4.x support symlinks properly

    foreach(QString subDirName, subDirs)
    {
        QString ext = UBFileSystemUtils::extension(subDirName);

        if (!mExtensionsToHide.contains(ext)) {
            new UBLibraryFolderItem(QDir(mDir.path() + "/" + subDirName),
                                    UBLibraryController::trUtf8(subDirName.toUtf8()), this,
                                    mCanWrite, mExtensionsToHide);
        }
    }
}

void UBLibraryController::refreshVideoThumbnailsView() {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> items;
    QList<QUrl> itemsPath;
    QStringList labels;

    QList<QTreeWidgetItem *> tw =
            mLibraryUI->videoLibraryTreeWidget->selectedItems();

    if (tw.size() > 0) {
        UBLibraryFolderItem* fi = dynamic_cast<UBLibraryFolderItem*> (tw.last());

        if (fi) {
            QStringList
                    videoPaths = UBPersistenceManager::persistenceManager()->allVideos(fi->dir());

            for (int i = 0; i < videoPaths.size(); i++) {
                QString path = videoPaths.at(i);

                UBThumbnailVideo *pixmapItem = new UBThumbnailVideo(QUrl::fromLocalFile(path));

                items.append(pixmapItem);
                itemsPath.append(QUrl::fromLocalFile(path));

                QFileInfo file(path);

                labels.append(file.completeBaseName());
            }
        }
    }

    mLibraryUI->videoThumbnailView->setGraphicsItems(items, itemsPath, labels);

    QApplication::restoreOverrideCursor();
}

void UBLibraryController::refreshSoundThumbnailsView() {
    QList<QGraphicsItem*> soundItems;
    QList<QUrl> soundItemsPath;
    QStringList soundLabels;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QStringList soundWidgetPaths = UBPersistenceManager::persistenceManager()->allSounds();

    for (int i = 0; i < soundWidgetPaths.size(); i++) {
        QString path = soundWidgetPaths.at(i);

        QGraphicsPixmapItem *pixmapItem = new UBThumbnailPixmap(QPixmap(
                ":/images/soundIcon.svg"));

        QString name = path;

        mSoundItemToFilepath.insert(pixmapItem, path);
        soundItems.append(pixmapItem);
        soundItemsPath.append(QUrl::fromLocalFile(path));
        soundLabels.append(UBFileSystemUtils::lastPathComponent(name));
    }

    mLibraryUI->soundThumbnailView->setGraphicsItems(soundItems, soundItemsPath,
                                                     soundLabels);

    QApplication::restoreOverrideCursor();
}

void UBLibraryController::addObjectFromFilesystemToPage() {

    bool hasAddedObjects = false;

    switch (mLibraryUI->libraryTab->currentIndex()) {
    case TabIndex::Image: {
            QString extensions;

            foreach(QString ext, UBSettings::settings()->imageFileExtensions)
            {
                extensions += " *.";
                extensions += ext;
            }

            QString defaultPath =
                    UBSettings::settings()->lastPicturePath->get().toString();

            QStringList filenames = QFileDialog::getOpenFileNames(mParentWidget, tr(
                    "Add Images to Current Page"), defaultPath,
                                                                  tr("All Images (%1)").arg(extensions));

            if (filenames.size() > 0) {
                QFileInfo fi(filenames.at(0));
                UBSettings::settings()->lastPicturePath->set(QVariant(
                        fi.dir().absolutePath()));

                QList<QUrl> urls;
                foreach(const QString s, filenames)
                {
                    urls << QUrl::fromLocalFile(s);
                }

                addImagesToCurrentPage(urls);

                hasAddedObjects = true;
            }
            break;
        }

    case TabIndex::Interactive: {
            QString defaultWidgetPath =
                    UBSettings::settings()->lastWidgetPath->get().toString();

            QString extensions;

            foreach(QString ext, UBSettings::settings()->interactiveContentFileExtensions)
            {
                extensions += " *.";
                extensions += ext;
            }

            QStringList filenames = QFileDialog::getOpenFileNames(mParentWidget, tr(
                    "Add Applications to Current Page"), defaultWidgetPath, tr(
                            "All Applications (%1)").arg(extensions));

            if (filenames.size() > 0) {
                QFileInfo fi(filenames.at(0));
                UBSettings::settings()->lastWidgetPath->set(QVariant(
                        fi.dir().absolutePath()));

                QList<QUrl> urls;
                foreach(const QString s, filenames)
                {
                    urls << QUrl::fromLocalFile(s);
                }

                addInteractivesToCurrentPage(urls);

                hasAddedObjects = true;
            }

            break;
        }

    case TabIndex::Video: {
            QString defaultVideoPath =
                    UBSettings::settings()->lastVideoPath->get().toString();

            QStringList filenames = QFileDialog::getOpenFileNames(mParentWidget, tr(
                    "Add Movies to Current Page"), defaultVideoPath, tr(
                            "All Files (*.*)"));
            if (filenames.size() > 0) {
                QFileInfo fi(filenames.at(0));
                UBSettings::settings()->lastVideoPath->set(QVariant(
                        fi.dir().absolutePath()));

                QList<QUrl> urls;
                foreach(const QString s, filenames)
                {
                    urls << QUrl::fromLocalFile(s);
                }

                addVideosToCurrentPage(urls);

                hasAddedObjects = true;
            }

            break;
        }
    case TabIndex::Gip: {
            //TODO
            break;
        }
    default:
        break;

    }

    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);

    if (hasAddedObjects) {
        mLibraryWindow->done(0);
    }
}

void UBLibraryController::addInteractiveToCurrentPage() {
    QList<QUrl> widgets;

    foreach(QGraphicsItem *item, mLibraryUI->interactiveThumbnailView->selectedItems())
    {
        QGraphicsPixmapItem* iconItem =
                dynamic_cast<QGraphicsPixmapItem*> (item);

        if (iconItem && activeScene()) {
            QString url = mInteractiveItemToFilepath.value(iconItem);

            if (url.startsWith("uniboardTool")) {
                widgets << QUrl(url);
            } else {
                widgets << QUrl::fromLocalFile(url);
            }
        }
    }

    addInteractivesToCurrentPage(widgets);
}

void UBLibraryController::loadLibraries()
{
    if (mLibraryFileToDownload.size() > 0) {
        QString mainLibrary = mLibraryFileToDownload.takeFirst();

        UBNetworkAccessManager *nam =
                UBNetworkAccessManager::defaultAccessManager();

        UBServerXMLHttpRequest *librariesReq = new UBServerXMLHttpRequest(nam); // Deleted automatically after reply content is consumed
        connect(librariesReq, SIGNAL(finished(bool, const QByteArray&)), this,
                SLOT(getLibraryListResponse(bool, const QByteArray&)));

        librariesReq->get(QUrl(mainLibrary));
    }
}

void UBLibraryController::getLibraryListResponse(bool ok,
                                                 const QByteArray& replyContent) {
    if (!ok)
        return;

    QXmlStreamReader xml(replyContent);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == "library") {
                QStringRef libraryTitle = xml.attributes().value("title");
                QStringRef libraryHref = xml.attributes().value("href");
                QStringRef libraryType = xml.attributes().value("type");

                if (!libraryTitle.isNull() && !libraryTitle.isNull()
                    && !libraryType.isNull()) {
                    QString title = libraryTitle.toString();
                    QUrl href(libraryHref.toString());

                    UBOnlineLibraryItem *ubLibrary = new UBOnlineLibraryItem(href,
                                                                             title, 0);

                    if (libraryType.toString() == "image") {
                        if (!mImageOnlineTi) {
                            mImageOnlineTi = new QTreeWidgetItem();
                            mImageOnlineTi->setText(0, tr("Online"));
                            mImageOnlineTi->setIcon(0, QPixmap(
                                    ":/images/online.png"));

                            mLibraryUI->imageLibraryTreeWidget->addTopLevelItem(
                                    mImageOnlineTi);
                        }
                        mImageOnlineTi->addChild(ubLibrary);
                        mImageOnlineTi->setExpanded(true);
                    } else if (libraryType.toString() == "video") {
                        if (!mVideoOnlineTi) {
                            mVideoOnlineTi = new QTreeWidgetItem();
                            mVideoOnlineTi->setText(0, tr("Online"));
                            mVideoOnlineTi->setIcon(0, QPixmap(
                                    ":/images/online.png"));

                            mLibraryUI->videoLibraryTreeWidget->addTopLevelItem(
                                    mVideoOnlineTi);
                        }
                        mVideoOnlineTi->addChild(ubLibrary);
                        mVideoOnlineTi->setExpanded(true);
                    } else if (libraryType.toString() == "interactive") {
                        if (!mInteractiveOnlineTi) {
                            mInteractiveOnlineTi = new QTreeWidgetItem();
                            mInteractiveOnlineTi->setText(0, tr("Online"));
                            mInteractiveOnlineTi->setIcon(0, QPixmap(
                                    ":/images/online.png"));

                            mLibraryUI->interactiveLibraryTreeWidget->addTopLevelItem(
                                    mInteractiveOnlineTi);
                        }
                        mInteractiveOnlineTi->addChild(ubLibrary);
                        mInteractiveOnlineTi->setExpanded(true);

                    }
                }
            }
        }
    }

    loadLibraries();
}

UBLibraryWebView::UBLibraryWebView(QWidget * parent) :
        QWebView(parent), mCurrentLibraryItem(0) {
    setPage(new UBWebPage(this));

    QWebView::settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebView::page()->setNetworkAccessManager(
            UBNetworkAccessManager::defaultAccessManager());

    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this,
            SLOT(javaScriptWindowObjectCleared()));
    connect(this, SIGNAL(iconChanged()), this, SLOT(newIconAvailable()));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
}

void UBLibraryWebView::javaScriptWindowObjectCleared() {
    UBLibraryAPI *libraryAPI = new UBLibraryAPI(this);

    page()->mainFrame()->addToJavaScriptWindowObject("uniboard", libraryAPI);
}

void UBLibraryWebView::loadFinished(bool ok) {
    if (ok) {
        QString installDragHandler =
                "document.body.ondragstart = function(event) {";

        installDragHandler += "var p = event.target.parentNode;";
        installDragHandler += "if (p && p.href){";
        installDragHandler += "event.preventDefault();";
        installDragHandler += "uniboard.startDrag(p.href);";
        installDragHandler += "}";
        installDragHandler += "}";

        page()->mainFrame()->evaluateJavaScript(installDragHandler);
    }

    mCurrentLibraryItem->setIcon(0, QWebSettings::iconForUrl(url()));
}

void UBLibraryWebView::newIconAvailable() {
    if (mCurrentLibraryItem)
        mCurrentLibraryItem->setIcon(0, icon());

}

QWebView * UBLibraryWebView::createWindow(QWebPage::WebWindowType type) {
    if (type == QWebPage::WebBrowserWindow) {
        UBApplication::applicationController->showInternet();
        return UBApplication::webController->createNewTab();
    } else {
        return this;
    }
}

UBLibraryPreloader::UBLibraryPreloader(QObject* pParent) :
        QObject(pParent) {
    QTimer::singleShot(30000, this, SLOT(loadLibrary()));
}

void UBLibraryPreloader::loadLibrary() {
    QStringList libraries = UBLibraryController::onlineLibraries();

    if (libraries.length() > 0) {
        QString mainLibrary = libraries.at(0);

        UBNetworkAccessManager *nam =
                UBNetworkAccessManager::defaultAccessManager();

        UBServerXMLHttpRequest *librariesReq = new UBServerXMLHttpRequest(nam); // Deleted automatically after reply content is consumed
        connect(librariesReq, SIGNAL(finished(bool, const QByteArray&)), this,
                SLOT(getLibraryListResponse(bool, const QByteArray&)));

        librariesReq->get(QUrl(mainLibrary));
    }
}

void UBLibraryPreloader::getLibraryListResponse(bool ok,
                                                const QByteArray& replyContent) {
    if (!ok)
        return;

    QXmlStreamReader xml(replyContent);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == "library") {
                QStringRef libraryHref = xml.attributes().value("href");
                QStringRef libraryType = xml.attributes().value("type");

                if (!libraryType.isNull() && !libraryHref.isNull()) {
                    if (libraryType.toString() == "interactive") {
                        QUrl href(libraryHref.toString());

                        mWebView = new QWebView(UBApplication::mainWindow);
                        mWebView->setVisible(false);

                        connect(mWebView, SIGNAL(loadFinished(bool)), this,
                                SLOT(loadFinished(bool)));

                        mWebView->load(QUrl(libraryHref.toString()));

                        return;
                    }
                }
            }
        }
    }
}

void UBLibraryPreloader::loadFinished(bool ok) {
    Q_UNUSED(ok);

    mWebView->deleteLater();
    deleteLater();
}

void UBLibraryController::addNativeToolToFavorites(const QUrl& url) {
    QString id = url.toString();

    QStringList favoritesNativeTools =
            UBSettings::settings()->favoritesNativeToolUris->get().toStringList();

    if (!favoritesNativeTools.contains(id)) {
        favoritesNativeTools << id;
        UBSettings::settings()->favoritesNativeToolUris->set(favoritesNativeTools);
    }

    refreshInteractiveThumbnailsView();
}

QString UBLibraryController::favoritePath()
{
    return favoriteVirtualPath;
}



UBLibElement::UBLibElement() {
    mType = eUBLibElementType_Category;
    mName = QObject::tr("/Home", "Category list label on navigation tool bar");
    mbMoveable = false;
}


UBLibElement::UBLibElement(eUBLibElementType type, const QUrl &path, const QString &name)
{
    mType = type;
    mPath = path;
    mName = name;
    mInfo = "";
    mbMoveable = true;

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
    mpElem = pElem;
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

UBLibElement* UBLibElement::trashElement()
{
    static UBLibElement *trashElement;
    if (trashElement)
        return trashElement;

    trashElement = new UBLibElement(eUBLibElementType_Folder, UBSettings::trashLibraryPaletteDirPath(), QObject::tr("Trash", "Pictures category element"));
    QImage *categoryImage = new QImage(":images/libpalette/TrashCategory.svg");
    trashElement->setThumbnail(categoryImage);
    trashElement->setMoveable(false);

    return trashElement;
}

