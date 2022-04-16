/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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



#include "UBPersistenceManager.h"
#include "gui/UBMainWindow.h"

#include <QtXml>
#include <QVariant>
#include <QDomDocument>
#include <QXmlStreamWriter>
#include <QModelIndex>

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBForeignObjectsHandler.h"

#include "document/UBDocumentProxy.h"

#include "adaptors/UBExportPDF.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsSvgItem.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "document/UBDocumentController.h"

#include "core/memcheck.h"

const QString UBPersistenceManager::imageDirectory = "images"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::objectDirectory = "objects"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::widgetDirectory = "widgets"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::videoDirectory = "videos"; // added to UBPersistenceManager::mAllDirectories
const QString UBPersistenceManager::audioDirectory = "audios"; // added to
const QString UBPersistenceManager::fileDirectory = "files"; // Issue 1683 (Evolution) - AOU - 20131206

const QString UBPersistenceManager::myDocumentsName = "MyDocuments";
const QString UBPersistenceManager::modelsName = "Models";
const QString UBPersistenceManager::untitledDocumentsName = "UntitledDocuments";
const QString UBPersistenceManager::fFolders = "folders.xml";
const QString UBPersistenceManager::tFolder = "folder";
const QString UBPersistenceManager::aName = "name";

UBPersistenceManager * UBPersistenceManager::sSingleton = 0;

UBPersistenceManager::UBPersistenceManager(QObject *pParent)
    : QObject(pParent)
    , mHasPurgedDocuments(false)
{

    xmlFolderStructureFilename = "model";

    mDocumentSubDirectories << imageDirectory;
    mDocumentSubDirectories << objectDirectory;
    mDocumentSubDirectories << widgetDirectory;
    mDocumentSubDirectories << videoDirectory;
    mDocumentSubDirectories << audioDirectory;
    mDocumentSubDirectories << fileDirectory; // Issue 1683 (Evolution) - AOU - 20131206

    mDocumentRepositoryPath = UBSettings::userDocumentDirectory();
    mFoldersXmlStorageName =  mDocumentRepositoryPath + "/" + fFolders;

    mDocumentTreeStructureModel = new UBDocumentTreeModel(this);
    createDocumentProxiesStructure();


    emit proxyListChanged();
}

UBPersistenceManager* UBPersistenceManager::persistenceManager()
{
    if (!sSingleton)
    {
        sSingleton = new UBPersistenceManager(UBApplication::staticMemoryCleaner);
    }

    return sSingleton;
}

void UBPersistenceManager::destroy()
{
    if (sSingleton)
        delete sSingleton;
    sSingleton = NULL;
}

UBPersistenceManager::~UBPersistenceManager()
{
}

void UBPersistenceManager::createDocumentProxiesStructure(bool interactive)
{
    mDocumentRepositoryPath = UBSettings::userDocumentDirectory();

    QDir rootDir(mDocumentRepositoryPath);
    rootDir.mkpath(rootDir.path());

    QFileInfoList contentList = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);
    createDocumentProxiesStructure(contentList, interactive);

    if (QFileInfo(mFoldersXmlStorageName).exists()) {
        QDomDocument xmlDom;
        QFile inFile(mFoldersXmlStorageName);
        if (inFile.open(QIODevice::ReadOnly)) {
            QString domString(inFile.readAll());

            int errorLine = 0; int errorColumn = 0;
            QString errorStr;

            if (xmlDom.setContent(domString, &errorStr, &errorLine, &errorColumn)) {
                loadFolderTreeFromXml("", xmlDom.firstChildElement());
            } else {
                qDebug() << "Error reading content of " << mFoldersXmlStorageName << endl
                         << "Error:" << inFile.errorString()
                         << "Line:" << errorLine
                         << "Column:" << errorColumn;
            }
            inFile.close();
        } else {
            qDebug() << "Error reading" << mFoldersXmlStorageName << endl
                     << "Error:" << inFile.errorString();
        }
    }
}

void UBPersistenceManager::createDocumentProxiesStructure(const QFileInfoList &contentInfo, bool interactive)
{
    foreach(QFileInfo path, contentInfo)
    {
        QString fullPath = path.absoluteFilePath();

        QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(fullPath);

        QString docGroupName = metadatas.value(UBSettings::documentGroupName, QString()).toString();
        QString docName = metadatas.value(UBSettings::documentName, QString()).toString();

        if (docName.isEmpty()) {
            qDebug() << "Group name and document name are empty in UBPersistenceManager::createDocumentProxiesStructure()";
            continue;
        }

        QModelIndex parentIndex = mDocumentTreeStructureModel->goTo(docGroupName);
        if (!parentIndex.isValid()) {
            return;
        }

        UBDocumentProxy* docProxy = new UBDocumentProxy(fullPath, metadatas); // managed in UBDocumentTreeNode
        foreach(QString key, metadatas.keys()) {
            docProxy->setMetaData(key, metadatas.value(key));
        }

        if (metadatas.contains(UBSettings::documentPageCount))
        {
            int pageCount = metadatas.value(UBSettings::documentPageCount).toInt();
            if (pageCount == 0)
                pageCount = sceneCount(docProxy);

            docProxy->setPageCount(pageCount);
        }
        else
        {
            int pageCount = sceneCount(docProxy);
            docProxy->setPageCount(pageCount);
        }

        if (!interactive)
            mDocumentTreeStructureModel->addDocument(docProxy, parentIndex);
        else
            processInteractiveReplacementDialog(docProxy);
    }
}

QDialog::DialogCode UBPersistenceManager::processInteractiveReplacementDialog(UBDocumentProxy *pProxy)
{
    //TODO claudio remove this hack necessary on double click on ubz file
    Qt::CursorShape saveShape;
    if(UBApplication::overrideCursor()){
        saveShape = UBApplication::overrideCursor()->shape();
        UBApplication::overrideCursor()->setShape(Qt::ArrowCursor);
    }
    else
        saveShape = Qt::ArrowCursor;

    QDialog::DialogCode result = QDialog::Rejected;

    if (UBApplication::documentController
            && UBApplication::documentController->mainWidget()) {
        QString docGroupName = pProxy->metaData(UBSettings::documentGroupName).toString();
        QModelIndex parentIndex = mDocumentTreeStructureModel->goTo(docGroupName);
        if (!parentIndex.isValid()) {
            UBApplication::overrideCursor()->setShape(saveShape);
            return QDialog::Rejected;
        }

        QStringList docList = mDocumentTreeStructureModel->nodeNameList(parentIndex, true);
        QString docName = pProxy->metaData(UBSettings::documentName).toString();

        if (docList.contains(docName)) {
            UBDocumentReplaceDialog *replaceDialog = new UBDocumentReplaceDialog(docName
                                                                                 , docList
                                                                                 , /*UBApplication::documentController->mainWidget()*/0
                                                                                 , Qt::Widget);
            if (replaceDialog->exec() == QDialog::Accepted)
            {
                result = QDialog::Accepted;
                QString resultName = replaceDialog->lineEditText();
                int i = docList.indexOf(resultName);
                if (i != -1) { //replace
                    QModelIndex replaceIndex = mDocumentTreeStructureModel->index(i, 0, parentIndex);
                    UBDocumentProxy *replaceProxy = mDocumentTreeStructureModel->proxyData(replaceIndex);

                    if (mDocumentTreeStructureModel->currentIndex() == replaceIndex)
                    {
                        UBApplication::documentController->selectDocument(pProxy, true, true);
                    }

                    if (replaceProxy) {
                        deleteDocument(replaceProxy);
                    }
                    if (replaceIndex.isValid()) {
                        mDocumentTreeStructureModel->removeRow(i, parentIndex);
                    }
                }
                pProxy->setMetaData(UBSettings::documentName, resultName);
                mDocumentTreeStructureModel->addDocument(pProxy, parentIndex);
            }
            replaceDialog->setParent(0);
            delete replaceDialog;
        } else {
            mDocumentTreeStructureModel->addDocument(pProxy, parentIndex);
            result = QDialog::Accepted;
        }

    }
    //TODO claudio the if is an hack
    if(UBApplication::overrideCursor())
        UBApplication::overrideCursor()->setShape(saveShape);

    return result;
}

QString UBPersistenceManager::adjustDocumentVirtualPath(const QString &str)
{
    QStringList pathList = str.split("/", QString::SkipEmptyParts);

    if (pathList.isEmpty()) {
        pathList.append(myDocumentsName);
        pathList.append(untitledDocumentsName);
    }

    if (pathList.first() != myDocumentsName
            && pathList.first() != UBSettings::trashedDocumentGroupNamePrefix
            && pathList.first() != modelsName) {
        pathList.prepend(myDocumentsName);
    }

    return pathList.join("/");
}

void UBPersistenceManager::closing()
{
    QDir rootDir(mDocumentRepositoryPath);
    rootDir.mkpath(rootDir.path());

    QFile outFile(mFoldersXmlStorageName);
    if (outFile.open(QIODevice::WriteOnly)) {
        QXmlStreamWriter writer(&outFile);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("content");
        saveFoldersTreeToXml(writer, QModelIndex());
        writer.writeEndElement();
        writer.writeEndDocument();

        outFile.close();
    } else {
        qDebug() << "failed to open document" <<  mFoldersXmlStorageName << "for writing" << endl
                 << "Error string:" << outFile.errorString();
    }
}

bool UBPersistenceManager::isSceneInCached(UBDocumentProxy *proxy, int index) const
{
    return mSceneCache.contains(proxy, index);
}

QStringList UBPersistenceManager::allShapes()
{
    QString shapeLibraryPath = UBSettings::settings()->applicationShapeLibraryDirectory();

    QDir dir(shapeLibraryPath);

    if (!dir.exists())
        dir.mkpath(shapeLibraryPath);

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(shapeLibraryPath + QString("/") + file);
    }

    return paths;
}

QStringList UBPersistenceManager::allGips()
{
    QString gipLibraryPath = UBSettings::settings()->userGipLibraryDirectory();

    QDir dir(gipLibraryPath);

    QStringList files = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        QFileInfo fi(file);

        if (UBSettings::settings()->widgetFileExtensions.contains(fi.suffix()))
            paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}

QStringList UBPersistenceManager::allImages(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


QStringList UBPersistenceManager::allVideos(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


QStringList UBPersistenceManager::allWidgets(const QDir& dir)
{
    if (!dir.exists())
        dir.mkpath(dir.path());

    QStringList files = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QStringList paths;

    foreach(QString file, files)
    {
        QFileInfo fi(file);

        if (UBSettings::settings()->widgetFileExtensions.contains(fi.suffix()))
            paths.append(dir.path() + QString("/") + file);
    }

    return paths;
}


UBDocumentProxy* UBPersistenceManager::createDocument(const QString& pGroupName
                                                      , const QString& pName
                                                      , bool withEmptyPage
                                                      , QString directory
                                                      , int pageCount
                                                      , bool promptDialogIfExists)
{
    UBDocumentProxy *doc;
    if(directory.length() != 0 ){
        doc = new UBDocumentProxy(directory); // deleted in UBPersistenceManager::destructor
        doc->setPageCount(pageCount);
    }
    else{
        checkIfDocumentRepositoryExists();
        doc = new UBDocumentProxy();
    }

    if (pGroupName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroupName);
    }

    if (pName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentName, pName);
    }

    doc->setMetaData(UBSettings::documentVersion, UBSettings::currentFileVersion);
    QString currentDate =  UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime());
    doc->setMetaData(UBSettings::documentUpdatedAt,currentDate);
    doc->setMetaData(UBSettings::documentDate,currentDate);

    if (withEmptyPage) {
        createDocumentSceneAt(doc, 0);
    }
    else{
        this->generatePathIfNeeded(doc);
        QDir dir(doc->persistencePath());
        if (!dir.mkpath(doc->persistencePath()))
        {
            return 0; // if we can't create the path, abort function.
        }
    }

    bool addDoc = false;
    if (!promptDialogIfExists) {
        addDoc = true;
        mDocumentTreeStructureModel->addDocument(doc);
    } else if (processInteractiveReplacementDialog(doc) == QDialog::Accepted) {
        addDoc = true;
    }
    if (addDoc) {
        emit proxyListChanged();
    } else {
        deleteDocument(doc);
        doc = 0;
    }

    return doc;
}

UBDocumentProxy* UBPersistenceManager::createNewDocument(const QString& pGroupName
                                                      , const QString& pName
                                                      , bool withEmptyPage
                                                      , QString directory
                                                      , int pageCount
                                                      , bool promptDialogIfExists)
{
    UBDocumentProxy *resultDoc = createDocument(pGroupName, pName, withEmptyPage, directory, pageCount, promptDialogIfExists);
    if (resultDoc) {
        mDocumentTreeStructureModel->markDocumentAsNew(resultDoc);
    }

    return resultDoc;
}

UBDocumentProxy* UBPersistenceManager::createDocumentFromDir(const QString& pDocumentDirectory
                                                             , const QString& pGroupName
                                                             , const QString& pName
                                                             , bool withEmptyPage
                                                             , bool addTitlePage
                                                             , bool promptDialogIfExists)
{
    checkIfDocumentRepositoryExists();

    UBDocumentProxy* doc = new UBDocumentProxy(pDocumentDirectory); // deleted in UBPersistenceManager::destructor

    if (pGroupName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentGroupName, pGroupName);
    }

    if (pName.length() > 0)
    {
        doc->setMetaData(UBSettings::documentName, pName);
    }

    QMap<QString, QVariant> metadatas = UBMetadataDcSubsetAdaptor::load(pDocumentDirectory);

    if(withEmptyPage) createDocumentSceneAt(doc, 0);
    if(addTitlePage) persistDocumentScene(doc, mSceneCache.createScene(doc, 0, false), 0);

    foreach(QString key, metadatas.keys())
    {
        doc->setMetaData(key, metadatas.value(key));
    }

    doc->setUuid(QUuid::createUuid());
    doc->setPageCount(sceneCount(doc));

    for(int i = 0; i < doc->pageCount(); i++)
    {
        UBSvgSubsetAdaptor::setSceneUuid(doc, i, QUuid::createUuid());
    }

    //work around the
    bool addDoc = false;
    if (!promptDialogIfExists) {
        addDoc = true;
        mDocumentTreeStructureModel->addDocument(doc);
    } else if (processInteractiveReplacementDialog(doc) == QDialog::Accepted) {
        addDoc = true;
    }
    if (addDoc) {
        UBMetadataDcSubsetAdaptor::persist(doc);
        emit proxyListChanged();
        emit documentCreated(doc);
    } else {
        deleteDocument(doc);
        doc = 0;
    }

    return doc;
}


void UBPersistenceManager::deleteDocument(UBDocumentProxy* pDocumentProxy)
{
    checkIfDocumentRepositoryExists();

    emit documentWillBeDeleted(pDocumentProxy);

    if (QFileInfo(pDocumentProxy->persistencePath()).exists())
        UBFileSystemUtils::deleteDir(pDocumentProxy->persistencePath());

    mSceneCache.removeAllScenes(pDocumentProxy);

    pDocumentProxy->deleteLater();
}

UBDocumentProxy* UBPersistenceManager::duplicateDocument(UBDocumentProxy* pDocumentProxy)
{
    checkIfDocumentRepositoryExists();

    UBDocumentProxy *copy = new UBDocumentProxy(); // deleted in UBPersistenceManager::destructor

    generatePathIfNeeded(copy);

    UBFileSystemUtils::copyDir(pDocumentProxy->persistencePath(), copy->persistencePath());

    // regenerate scenes UUIDs
    for(int i = 0; i < pDocumentProxy->pageCount(); i++)
    {
        UBSvgSubsetAdaptor::setSceneUuid(pDocumentProxy, i, QUuid::createUuid());
    }

    foreach(QString key, pDocumentProxy->metaDatas().keys())
    {
        copy->setMetaData(key, pDocumentProxy->metaDatas().value(key));
    }    

    copy->setMetaData(UBSettings::documentName,
            pDocumentProxy->metaData(UBSettings::documentName).toString() + " " + tr("(copy)"));

    copy->setUuid(QUuid::createUuid());

    persistDocumentMetadata(copy);

    copy->setPageCount(sceneCount(copy));

    emit proxyListChanged();

    emit documentCreated(copy);

    return copy;

}


void UBPersistenceManager::deleteDocumentScenes(UBDocumentProxy* proxy, const QList<int>& indexes)
{
    checkIfDocumentRepositoryExists();

    int pageCount = UBPersistenceManager::persistenceManager()->sceneCount(proxy);

    QList<int> compactedIndexes;

    foreach(int index, indexes)
    {
        if (!compactedIndexes.contains(index))
            compactedIndexes.append(index);
    }

    if (compactedIndexes.size() == pageCount)
    {
        deleteDocument(proxy);
        return;
    }

    if (compactedIndexes.size() == 0)
        return;

    foreach(int index, compactedIndexes)
    {
        emit documentSceneWillBeDeleted(proxy, index);
    }

    QString sourceName = proxy->metaData(UBSettings::documentName).toString();
    UBDocumentProxy *trashDocProxy = createDocument(UBSettings::trashedDocumentGroupNamePrefix/* + sourceGroupName*/, sourceName, false);

    foreach(int index, compactedIndexes)
    {
        UBGraphicsScene *scene = loadDocumentScene(proxy, index);
        if (scene)
        {
            //scene is about to move into new document
            foreach (QUrl relativeFile, scene->relativeDependencies())
            {
                QString source = scene->document()->persistencePath() + "/" + relativeFile.toString();
                QString target = trashDocProxy->persistencePath() + "/" + relativeFile.toString();

                QFileInfo fi(target);
                QDir d = fi.dir();

                d.mkpath(d.absolutePath());
                QFile::copy(source, target);
            }

            insertDocumentSceneAt(trashDocProxy, scene, trashDocProxy->pageCount());
        }
    }

    for (int i = 1; i < pageCount; i++)
    {
        renamePage(trashDocProxy, i , i - 1);
    }

    foreach(int index, compactedIndexes)
    {
        QString svgFileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", index);

        QFile::remove(svgFileName);

        QString thumbFileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", index);

        QFile::remove(thumbFileName);

        mSceneCache.removeScene(proxy, index);

        proxy->decPageCount();

    }

    qSort(compactedIndexes);

    int offset = 1;

    for (int i = compactedIndexes.at(0) + 1; i < pageCount; i++)
    {
        if(compactedIndexes.contains(i))
        {
            offset++;
        }
        else
        {
            renamePage(proxy, i , i - offset);

            mSceneCache.moveScene(proxy, i, i - offset);

        }
    }
}


void UBPersistenceManager::duplicateDocumentScene(UBDocumentProxy* proxy, int index)
{
    checkIfDocumentRepositoryExists();

    int pageCount = UBPersistenceManager::persistenceManager()->sceneCount(proxy);

    for (int i = pageCount; i > index + 1; i--)
    {
        renamePage(proxy, i - 1 , i);

        mSceneCache.moveScene(proxy, i - 1, i);

    }

    copyPage(proxy, index , index + 1);

    //TODO: write a proper way to handle object on disk
    UBGraphicsScene *scene = loadDocumentScene(proxy, index + 1);

    foreach(QGraphicsItem* item, scene->items())
    {
        UBGraphicsMediaItem *mediaItem = qgraphicsitem_cast<UBGraphicsMediaItem*> (item);

        if (mediaItem){
            QString source = mediaItem->mediaFileUrl().toLocalFile();
            QString destination = source;
            QUuid newUuid = QUuid::createUuid();
            QString fileName = QFileInfo(source).completeBaseName();
            destination = destination.replace(fileName,newUuid.toString());
            QFile::copy(source,destination);
            mediaItem->setMediaFileUrl(QUrl::fromLocalFile(destination));
            continue;
        }

        UBGraphicsWidgetItem* widget = qgraphicsitem_cast<UBGraphicsWidgetItem*>(item);
        if(widget){
            QUuid newUUid = QUuid::createUuid();
            QString newUUidString = newUUid.toString().remove("{").remove("}");
            QString actualUuidString = widget->uuid().toString().remove("{").remove("}");

            QString widgetSourcePath = proxy->persistencePath() + "/" + UBPersistenceManager::widgetDirectory + "/{" + actualUuidString + "}.wgt";
            QString screenshotSourcePath = proxy->persistencePath() + "/" +  UBPersistenceManager::widgetDirectory + "/" + actualUuidString + ".png";

            QString widgetDestinationPath = widgetSourcePath;
            widgetDestinationPath = widgetDestinationPath.replace(actualUuidString,newUUidString);
            QString screenshotDestinationPath = screenshotSourcePath;
            screenshotDestinationPath = screenshotDestinationPath.replace(actualUuidString,newUUidString);

            UBFileSystemUtils::copyDir(widgetSourcePath,widgetDestinationPath);
            QFile::copy(screenshotSourcePath,screenshotDestinationPath);

            widget->setUuid(newUUid);

            widget->widgetUrl(QUrl::fromLocalFile(widgetDestinationPath));

            continue;
        }

        UBGraphicsPixmapItem* pixmapItem = qgraphicsitem_cast<UBGraphicsPixmapItem*>(item);
        if(pixmapItem){
            QString source = proxy->persistencePath() + "/" +  UBPersistenceManager::imageDirectory + "/" + pixmapItem->uuid().toString() + ".png";
            QString destination = source;
            QUuid newUuid = QUuid::createUuid();
            QString fileName = QFileInfo(source).completeBaseName();
            destination = destination.replace(fileName,newUuid.toString());
            QFile::copy(source,destination);
            pixmapItem->setUuid(newUuid);
            continue;
        }

        UBGraphicsSvgItem* svgItem = qgraphicsitem_cast<UBGraphicsSvgItem*>(item);
        if(svgItem){
            QString source = proxy->persistencePath() + "/" +  UBPersistenceManager::imageDirectory + "/" + svgItem->uuid().toString() + ".svg";
            QString destination = source;
            QUuid newUuid = QUuid::createUuid();
            QString fileName = QFileInfo(source).completeBaseName();
            destination = destination.replace(fileName,newUuid.toString());
            QFile::copy(source,destination);
            svgItem->setUuid(newUuid);
            continue;
        }

    }
    scene->setModified(true);

    proxy->incPageCount();

    persistDocumentScene(proxy,scene, index + 1);

    emit documentSceneCreated(proxy, index + 1);
}

void UBPersistenceManager::copyDocumentScene(UBDocumentProxy *from, int fromIndex, UBDocumentProxy *to, int toIndex)
{
    if (from == to && toIndex <= fromIndex) {
        qDebug() << "operation is not supported" << Q_FUNC_INFO;
        return;
    }

    checkIfDocumentRepositoryExists();

    for (int i = to->pageCount(); i > toIndex; i--) {
        renamePage(to, i - 1, i);
        mSceneCache.moveScene(to, i - 1, i);
    }

    UBForeighnObjectsHandler hl;
    hl.copyPage(QUrl::fromLocalFile(from->persistencePath()), fromIndex,
                QUrl::fromLocalFile(to->persistencePath()), toIndex);

    to->incPageCount();

    QString thumbTmp(from->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", fromIndex));
    QString thumbTo(to->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", toIndex));

    QFile::remove(thumbTo);
    QFile::copy(thumbTmp, thumbTo);

    Q_ASSERT(QFileInfo(thumbTmp).exists());
    Q_ASSERT(QFileInfo(thumbTo).exists());
    const QPixmap *pix = new QPixmap(thumbTmp);
    UBDocumentController *ctrl = UBApplication::documentController;
    ctrl->addPixmapAt(pix, toIndex);
    ctrl->TreeViewSelectionChanged(ctrl->firstSelectedTreeIndex(), QModelIndex());

//    emit documentSceneCreated(to, toIndex + 1);
}


UBGraphicsScene* UBPersistenceManager::createDocumentSceneAt(UBDocumentProxy* proxy, int index, bool useUndoRedoStack)
{
    int count = sceneCount(proxy);

    for(int i = count - 1; i >= index; i--)
        renamePage(proxy, i , i + 1);

    mSceneCache.shiftUpScenes(proxy, index, count -1);

    UBGraphicsScene *newScene = mSceneCache.createScene(proxy, index, useUndoRedoStack);

    newScene->setBackground(UBSettings::settings()->isDarkBackground(),
            UBSettings::settings()->UBSettings::pageBackground());

    newScene->setBackgroundGridSize(UBSettings::settings()->crossSize);

    proxy->incPageCount();

    persistDocumentScene(proxy, newScene, index);

    emit documentSceneCreated(proxy, index);

    return newScene;
}


void UBPersistenceManager::insertDocumentSceneAt(UBDocumentProxy* proxy, UBGraphicsScene* scene, int index, bool persist)
{
    scene->setDocument(proxy);

    int count = sceneCount(proxy);

    for(int i = count - 1; i >= index; i--)
    {
        renamePage(proxy, i , i + 1);
    }

    mSceneCache.shiftUpScenes(proxy, index, count -1);

    mSceneCache.insert(proxy, index, scene);

    proxy->incPageCount();

    if (persist) {
        persistDocumentScene(proxy, scene, index);
    }

    emit documentSceneCreated(proxy, index);

}


void UBPersistenceManager::moveSceneToIndex(UBDocumentProxy* proxy, int source, int target)
{
    checkIfDocumentRepositoryExists();

    if (source == target)
        return;

    QFile svgTmp(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", source));
    svgTmp.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.tmp", target));

    QFile thumbTmp(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", source));
    thumbTmp.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.tmp", target));

    if (source < target)
    {
        for (int i = source + 1; i <= target; i++)
        {
            renamePage(proxy, i , i - 1);
        }
    }
    else
    {
        for (int i = source - 1; i >= target; i--)
        {
            renamePage(proxy, i , i + 1);
        }
    }

    QFile svg(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.tmp", target));
    svg.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", target));

    QFile thumb(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.tmp", target));
    thumb.rename(proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", target));

    mSceneCache.moveScene(proxy, source, target);
}


UBGraphicsScene* UBPersistenceManager::loadDocumentScene(UBDocumentProxy* proxy, int sceneIndex)
{
    if (mSceneCache.contains(proxy, sceneIndex))
        return mSceneCache.value(proxy, sceneIndex);
    else {
        UBGraphicsScene* scene = UBSvgSubsetAdaptor::loadScene(proxy, sceneIndex);
        if(!scene){
            createDocumentSceneAt(proxy,0);
            scene = UBSvgSubsetAdaptor::loadScene(proxy, 0);
        }

        if (scene)
            mSceneCache.insert(proxy, sceneIndex, scene);

        return scene;
    }
}

void UBPersistenceManager::reassignDocProxy(UBDocumentProxy *newDocument, UBDocumentProxy *oldDocument)
{
    return mSceneCache.reassignDocProxy(newDocument, oldDocument);
}

void UBPersistenceManager::persistDocumentScene(UBDocumentProxy* pDocumentProxy, UBGraphicsScene* pScene, const int pSceneIndex)
{
    checkIfDocumentRepositoryExists();

    pScene->deselectAllItems();

    generatePathIfNeeded(pDocumentProxy);

    QDir dir(pDocumentProxy->persistencePath());
    dir.mkpath(pDocumentProxy->persistencePath());

    if (pDocumentProxy->isModified())
        UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);

    if (pScene->isModified())
    {
        UBSvgSubsetAdaptor::persistScene(pDocumentProxy, pScene, pSceneIndex);

        UBThumbnailAdaptor::persistScene(pDocumentProxy, pScene, pSceneIndex);

        pScene->setModified(false);
    }

    mSceneCache.insert(pDocumentProxy, pSceneIndex, pScene);
}


UBDocumentProxy* UBPersistenceManager::persistDocumentMetadata(UBDocumentProxy* pDocumentProxy)
{
    UBMetadataDcSubsetAdaptor::persist(pDocumentProxy);

    emit documentMetadataChanged(pDocumentProxy);

    return pDocumentProxy;
}


void UBPersistenceManager::renamePage(UBDocumentProxy* pDocumentProxy, const int sourceIndex, const int targetIndex)
{
    QFile svg(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", sourceIndex));
    svg.rename(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg",  targetIndex));

    QFile thumb(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex));
    thumb.rename(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex));
}


void UBPersistenceManager::copyPage(UBDocumentProxy* pDocumentProxy, const int sourceIndex, const int targetIndex)
{
    QFile svg(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg",sourceIndex));
    svg.copy(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", targetIndex));

    UBSvgSubsetAdaptor::setSceneUuid(pDocumentProxy, targetIndex, QUuid::createUuid());

    QFile thumb(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex));
    thumb.copy(pDocumentProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex));
}


int UBPersistenceManager::sceneCount(const UBDocumentProxy* proxy)
{
    const QString pPath = proxy->persistencePath();

    int pageIndex = 0;
    bool moreToProcess = true;
    bool addedMissingZeroPage = false;

    while (moreToProcess)
    {
        QString fileName = pPath + UBFileSystemUtils::digitFileFormat("/page%1.svg", pageIndex);

        QFile file(fileName);

        if (file.exists())
        {
            pageIndex++;
        }
        else
        {
            moreToProcess = false;
        }
    }

    if(pageIndex == 1 && addedMissingZeroPage){
        // increment is done only to check if there are other pages than the missing zero page
        // This situation means -> no pages on the document
        return 0;
    }

    return pageIndex;
}

QStringList UBPersistenceManager::getSceneFileNames(const QString& folder)
{
    QDir dir(folder, "page???.svg", QDir::Name, QDir::Files);
    return dir.entryList();
}

QString UBPersistenceManager::generateUniqueDocumentPath(const QString& baseFolder)
{
    QDateTime now = QDateTime::currentDateTime();
    QString dirName = now.toString("yyyy-MM-dd hh-mm-ss.zzz");

    return baseFolder + QString("/OpenBoard Document %1").arg(dirName);
}

QString UBPersistenceManager::generateUniqueDocumentPath()
{
    return generateUniqueDocumentPath(UBSettings::userDocumentDirectory());
}


void UBPersistenceManager::generatePathIfNeeded(UBDocumentProxy* pDocumentProxy)
{
    if (pDocumentProxy->persistencePath().length() == 0)
    {
        pDocumentProxy->setPersistencePath(generateUniqueDocumentPath());
    }
}


bool UBPersistenceManager::addDirectoryContentToDocument(const QString& documentRootFolder, UBDocumentProxy* pDocument)
{
    QStringList sourceScenes = getSceneFileNames(documentRootFolder);
    if (sourceScenes.empty())
        return false;

    int targetPageCount = pDocument->pageCount();

    for(int sourceIndex = 0 ; sourceIndex < sourceScenes.size(); sourceIndex++)
    {
        int targetIndex = targetPageCount + sourceIndex;

        QFile svg(documentRootFolder + "/" + sourceScenes[sourceIndex]);
        if (!svg.copy(pDocument->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.svg", targetIndex)))
            return false;

        UBSvgSubsetAdaptor::setSceneUuid(pDocument, targetIndex, QUuid::createUuid());

        QFile thumb(documentRootFolder + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceIndex));
        // We can ignore error in this case, thumbnail will be genarated
        thumb.copy(pDocument->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetIndex));
    }

    foreach(QString dir, mDocumentSubDirectories)
    {
        qDebug() << "copying " << documentRootFolder << "/" << dir << " to " << pDocument->persistencePath() << "/" + dir;

        QDir srcDir(documentRootFolder + "/" + dir);
        if (srcDir.exists())
            if (!UBFileSystemUtils::copyDir(documentRootFolder + "/" + dir, pDocument->persistencePath() + "/" + dir))
                return false;
    }

    pDocument->setPageCount(sceneCount(pDocument));

    //issue NC - NNE - 20131213 : At this point, all is well done.
    return true;
}


bool UBPersistenceManager::isEmpty(UBDocumentProxy* pDocumentProxy)
{
    if(!pDocumentProxy)
        return true;

    if (pDocumentProxy->pageCount() > 1)
        return false;

    UBGraphicsScene *theSoleScene = UBSvgSubsetAdaptor::loadScene(pDocumentProxy, 0);

    bool empty = false;

    if (theSoleScene)
    {
        empty = theSoleScene->isEmpty();
        delete theSoleScene;
    }
    else
    {
        empty = true;
    }

    return empty;
}


void UBPersistenceManager::purgeEmptyDocuments()
{
    QList<UBDocumentProxy*> toBeDeleted;

    foreach(UBDocumentProxy* docProxy, mDocumentTreeStructureModel->newDocuments())
    {
        if (isEmpty(docProxy))
        {
            toBeDeleted << docProxy;
        }
    }

    foreach(UBDocumentProxy* docProxy, toBeDeleted)
    {
        deleteDocument(docProxy);
    }
}

bool UBPersistenceManager::addFileToDocument(UBDocumentProxy* pDocumentProxy,
                                                     QString path,
                                                     const QString& subdir,
                                                     QUuid objectUuid,
                                                     QString& destinationPath,
                                                     QByteArray* data)
{
    Q_ASSERT(path.length());
    QFileInfo fi(path);

    if (!pDocumentProxy || objectUuid.isNull())
        return false;
    if (data == NULL && !fi.exists())
        return false;

    qDebug() << fi.suffix();

    QString fileName = subdir + "/" + objectUuid.toString() + "." + fi.suffix();

    destinationPath = pDocumentProxy->persistencePath() + "/" + fileName;

    if (!QFile::exists(destinationPath))
    {
        QDir dir;
        dir.mkdir(pDocumentProxy->persistencePath() + "/" + subdir);
        if (!QFile::exists(pDocumentProxy->persistencePath() + "/" + subdir))
            return false;

        if (data == NULL)
        {
            QFile source(path);
            return source.copy(destinationPath);
        }
        else
        {
            QFile newFile(destinationPath);

            if (newFile.open(QIODevice::WriteOnly))
            {
                qint64 n = newFile.write(*data);
                newFile.flush();
                newFile.close();
                return n == data->size();
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
}

bool UBPersistenceManager::addGraphicsWidgetToDocument(UBDocumentProxy *pDocumentProxy,
                                                       QString path,
                                                       QUuid objectUuid,
                                                       QString& destinationPath)
{
    QFileInfo fi(path);

    if (!fi.exists() || !pDocumentProxy || objectUuid.isNull())
        return false;

    QString widgetRootDir = path;
    QString extension = QFileInfo(widgetRootDir).suffix();

    destinationPath = pDocumentProxy->persistencePath() + "/" + widgetDirectory +  "/" + objectUuid.toString() + "." + extension;

    if (!QFile::exists(destinationPath)) {
        QDir dir;
        if (!dir.mkpath(destinationPath))
            return false;
        return UBFileSystemUtils::copyDir(widgetRootDir, destinationPath);
    }
    else
        return false;
}


void UBPersistenceManager::documentRepositoryChanged(const QString& path)
{
    Q_UNUSED(path);
    checkIfDocumentRepositoryExists();
}


void UBPersistenceManager::checkIfDocumentRepositoryExists()
{
    QDir rp(mDocumentRepositoryPath);

    if (!rp.exists())
    {
        // we have lost the document repository ..

        QString humanPath = QDir::cleanPath(mDocumentRepositoryPath);
        humanPath = QDir::toNativeSeparators(humanPath);

        UBApplication::mainWindow->warning(tr("Document Repository Loss"),tr("OpenBoard has lost access to the document repository '%1'. Unfortunately the application must shut down to avoid data corruption. Latest changes may be lost as well.").arg(humanPath));

        UBApplication::quit();
    }
}

void UBPersistenceManager::saveFoldersTreeToXml(QXmlStreamWriter &writer, const QModelIndex &parentIndex)
{
    for (int i = 0; i < mDocumentTreeStructureModel->rowCount(parentIndex); i++)
    {
        QModelIndex currentIndex = mDocumentTreeStructureModel->index(i, 0, parentIndex);
        if (mDocumentTreeStructureModel->isCatalog(currentIndex))
        {
            writer.writeStartElement(tFolder);
            writer.writeAttribute(aName, mDocumentTreeStructureModel->nodeFromIndex(currentIndex)->nodeName());
            saveFoldersTreeToXml(writer, currentIndex);
            writer.writeEndElement();
        }
    }
}

void UBPersistenceManager::loadFolderTreeFromXml(const QString &path, const QDomElement &element)
{

    QDomElement iterElement = element.firstChildElement();
    while(!iterElement.isNull())
    {
        QString leafPath;
        if (tFolder == iterElement.tagName())
        {
            leafPath = iterElement.attribute(aName);

            if (!leafPath.isEmpty())
            {
                mDocumentTreeStructureModel->goTo(path + "/" + leafPath);
                if (!iterElement.firstChildElement().isNull())
                    loadFolderTreeFromXml(path + "/" +  leafPath, iterElement);
            }
        }
        iterElement = iterElement.nextSiblingElement();
    }
}

bool UBPersistenceManager::mayHaveVideo(UBDocumentProxy* pDocumentProxy)
{
    QDir videoDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::videoDirectory);

    return videoDir.exists() && videoDir.entryInfoList().length() > 0;
}

bool UBPersistenceManager::mayHaveAudio(UBDocumentProxy* pDocumentProxy)
{
    QDir audioDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::audioDirectory);

    return audioDir.exists() && audioDir.entryInfoList().length() > 0;
}

bool UBPersistenceManager::mayHavePDF(UBDocumentProxy* pDocumentProxy)
{
    QDir objectDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::objectDirectory);

    QStringList filters;
    filters << "*.pdf";

    return objectDir.exists() && objectDir.entryInfoList(filters).length() > 0;
}


bool UBPersistenceManager::mayHaveSVGImages(UBDocumentProxy* pDocumentProxy)
{
    QDir imageDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::imageDirectory);

    QStringList filters;
    filters << "*.svg";

    return imageDir.exists() && imageDir.entryInfoList(filters).length() > 0;
}


bool UBPersistenceManager::mayHaveWidget(UBDocumentProxy* pDocumentProxy)
{
    QDir widgetDir(pDocumentProxy->persistencePath() + "/" + UBPersistenceManager::widgetDirectory);

    return widgetDir.exists() && widgetDir.entryInfoList(QDir::Dirs).length() > 0;
}
