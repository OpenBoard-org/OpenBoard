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




#include "UBWidgetUniboardAPI.h"

#include <QDomDocument>
#include <QtGui>

#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "document/UBDocumentProxy.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsWidgetItem.h"

#include "adaptors/UBThumbnailAdaptor.h"

#include "UBWidgetMessageAPI.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBDownloadManager.h"
#include "gui/UBBackgroundManager.h"

#include "core/memcheck.h"

//Known extentions for files, add if you know more supported
const QString audioExtentions = ".mp3.wma.ogg";
const QString videoExtentions = ".avi.flv";
const QString imageExtentions = ".png.jpg.tif.bmp.tga";
const QString htmlExtentions = ".htm.html.xhtml";

//Allways use aliases instead of const char* itself
const QString imageAlias    = "image";
const QString imageAliasCap = "Image";
const QString videoAlias    = "video";
const QString videoAliasCap = "Video";
const QString audioAlias    = "audio";
const QString audioAliasCap = "Audio";

//Xml tag names
const QString tMainSection = "mimedata";
const QString tType = "type";
const QString tPath = "path";
const QString tMessage = "message";
const QString tReady = "ready";

const QString tMimeText = "text/plain";


//Name of path inside widget to store objects
const QString objectsPath = "objects";

UBWidgetUniboardAPI::UBWidgetUniboardAPI(std::shared_ptr<UBGraphicsScene> pScene, UBGraphicsWidgetItem *widget)
    : QObject(pScene.get())
    , mScene(pScene)
    , mGraphicsWidget(widget)
    , mIsVisible(false)
    , mMessagesAPI(nullptr)
    , mDatastoreAPI(nullptr)
    , mProcessFileDrop(false)
 {
    UBGraphicsW3CWidgetItem* w3CGraphicsWidget = dynamic_cast<UBGraphicsW3CWidgetItem*>(widget);

    if (w3CGraphicsWidget)
    {
        mMessagesAPI = new UBWidgetMessageAPI(w3CGraphicsWidget);
        mDatastoreAPI = new UBDatastoreAPI(w3CGraphicsWidget);
    }

    connect(UBDownloadManager::downloadManager(), SIGNAL(downloadFinished(bool,sDownloadFileDesc,QByteArray)), this, SLOT(onDownloadFinished(bool,sDownloadFileDesc,QByteArray)));
}


UBWidgetUniboardAPI::~UBWidgetUniboardAPI()
{
    // NOOP
}

QObject* UBWidgetUniboardAPI::messages() const
{
    return mMessagesAPI;
}


QObject* UBWidgetUniboardAPI::datastore() const
{
    return mDatastoreAPI;
}


void UBWidgetUniboardAPI::setTool(const QString& toolString)
{
    if (UBApplication::boardController->activeScene() != mScene.lock())
        return;

    const QString lower = toolString.toLower();

    if (lower == "pen")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Pen);
    }
    else if (lower == "marker")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Marker);
    }
    else if (lower == "arrow")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    }
    else if (lower == "play")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Play);
    }
    else if (lower == "line")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Line);
    }
}


void UBWidgetUniboardAPI::setPenColor(const QString& penColor)
{
    if (UBApplication::boardController->activeScene() != mScene.lock())
        return;

    UBSettings* settings = UBSettings::settings();

    bool conversionState = false;

    int index = penColor.toInt(&conversionState) - 1;

    if (conversionState && index > 0 && index <= 4)
    {
        UBApplication::boardController->setPenColorOnDarkBackground(settings->penColors(true).at(index - 1));
        UBApplication::boardController->setPenColorOnLightBackground(settings->penColors(false).at(index - 1));
    }
    else
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QColor svgColor = QColor::fromString(penColor);
#else
        QColor svgColor;
        svgColor.setNamedColor(penColor);
#endif
        if (svgColor.isValid())
        {
            UBApplication::boardController->setPenColorOnDarkBackground(svgColor);
            UBApplication::boardController->setPenColorOnLightBackground(svgColor);
        }
    }
}


void UBWidgetUniboardAPI::setMarkerColor(const QString& penColor)
{
    if (UBApplication::boardController->activeScene() != mScene.lock())
        return;

    UBSettings* settings = UBSettings::settings();

    bool conversionState = false;

    int index = penColor.toInt(&conversionState);

    if (conversionState && index > 0 && index <= 4)
    {
        UBApplication::boardController->setMarkerColorOnDarkBackground(settings->markerColors(true).at(index - 1));
        UBApplication::boardController->setMarkerColorOnLightBackground(settings->markerColors(false).at(index - 1));
    }
    else
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
        QColor svgColor = QColor::fromString(penColor);
#else
        QColor svgColor;
        svgColor.setNamedColor(penColor);
#endif
        if (svgColor.isValid())
        {
            UBApplication::boardController->setMarkerColorOnDarkBackground(svgColor);
            UBApplication::boardController->setMarkerColorOnLightBackground(svgColor);
        }
    }
}


void UBWidgetUniboardAPI::addObject(QString pUrl, int width, int height, int x, int y, bool background)
{
    // not great, but make it easily scriptable --
    //
    // download url should be moved to the scene from the controller
    //

    if (UBApplication::boardController->activeScene() != mScene.lock())
        return;

    UBApplication::boardController->downloadURL(QUrl(pUrl), QString(), QPointF(x, y), QSize(width, height), background);

}


void UBWidgetUniboardAPI::setBackground(bool pIsDark, bool pIsCrossed)
{
    auto scene = mScene.lock();

    if (scene) {
        const auto backgroundManager = UBApplication::boardController->backgroundManager();
        const auto background = backgroundManager->guessBackground(pIsCrossed, false, false);
        scene->setSceneBackground(pIsDark, background);
    }
}


void UBWidgetUniboardAPI::moveTo(const qreal x, const qreal y)
{
    if (qIsNaN(x) || qIsNaN(y)
        || qIsInf(x) || qIsInf(y))
        return;

    auto scene = mScene.lock();

    if (scene)
    scene->moveTo(QPointF(x, y));
}


void UBWidgetUniboardAPI::drawLineTo(const qreal x, const qreal y, const qreal pWidth)
{
    if (qIsNaN(x) || qIsNaN(y) || qIsNaN(pWidth)
        || qIsInf(x) || qIsInf(y) || qIsInf(pWidth))
        return;

    auto scene = mScene.lock();

    if (scene)
    scene->drawLineTo(QPointF(x, y), pWidth,
        UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Line);
}


void UBWidgetUniboardAPI::eraseLineTo(const qreal x, const qreal y, const qreal pWidth)
{
    if (qIsNaN(x) || qIsNaN(y) || qIsNaN(pWidth)
       || qIsInf(x) || qIsInf(y) || qIsInf(pWidth))
       return;

    auto scene = mScene.lock();

    if (scene)
    scene->eraseLineTo(QPointF(x, y), pWidth);
}


void UBWidgetUniboardAPI::clear()
{
    auto scene = mScene.lock();

    if (scene)
        scene->clearContent(UBGraphicsScene::clearItemsAndAnnotations);
}


void UBWidgetUniboardAPI::zoom(const qreal factor, const qreal x, const qreal y)
{
   if (qIsNaN(factor) || qIsNaN(x) || qIsNaN(y)
       || qIsInf(factor) || qIsInf(x) || qIsInf(y))
       return;


    if (UBApplication::boardController->activeScene() != mScene.lock())
        return;

    UBApplication::boardController->zoom(factor, QPointF(x, y));
}


void UBWidgetUniboardAPI::centerOn(const qreal x, const qreal y)
{
   if (qIsNaN(x) || qIsNaN(y)
       || qIsInf(x) || qIsInf(y))
       return;

    if (UBApplication::boardController->activeScene() != mScene.lock())
        return;

    UBApplication::boardController->centerOn(QPointF(x, y));
}


void UBWidgetUniboardAPI::move(const qreal x, const qreal y)
{
    if (qIsNaN(x) || qIsNaN(y)
        || qIsInf(x) || qIsInf(y))
        return;

    if (UBApplication::boardController->activeScene() != mScene.lock())
        return;

    UBApplication::boardController->handScroll(x, y);
}


void UBWidgetUniboardAPI::addText(const QString& text, const qreal x, const qreal y, const int size, const QString& font
        , bool bold, bool italic)
{
    if (qIsNaN(x) || qIsNaN(y)
        || qIsInf(x) || qIsInf(y))
        return;

    if (UBApplication::boardController->activeScene() != mScene.lock())
        return;

    auto scene = mScene.lock();

    if (scene)
        scene->addTextWithFont(text, QPointF(x, y), size, font, bold, italic);

}


int UBWidgetUniboardAPI::pageCount() const
{
    auto scene = mScene.lock();

    if (scene && scene->document())
        return scene->document()->pageCount();
    else
        return -1;
}


int UBWidgetUniboardAPI::currentPageNumber() const
{
    // TODO UB 4.x widget find a better way to get the current page number

    if (UBApplication::boardController->activeScene() != mScene.lock())
        return -1;

    return UBApplication::boardController->activeSceneIndex() + 1;
}

void UBWidgetUniboardAPI::setDropData(const QString &data)
{
    if (data != mDropData) {
        mDropData = data;
        emit dropDataChanged(mDropData);
    }
}

QString UBWidgetUniboardAPI::getObjDir()
{
    return mGraphicsWidget->getOwnFolder().toLocalFile() + "/" + objectsPath + "/";
}

void UBWidgetUniboardAPI::showMessage(const QString& message)
{
    UBApplication::boardController->showMessage(message, false);
}


QString UBWidgetUniboardAPI::pageThumbnail(const int pageNumber)
{
    if (UBApplication::boardController->activeScene() != mScene.lock())
        return "";

    std::shared_ptr<UBDocumentProxy> doc = UBApplication::boardController->selectedDocument();

    if (!doc)
        return "";

    if (pageNumber > doc->pageCount())
        return "";

    QUrl url = UBThumbnailAdaptor::thumbnailUrl(doc, pageNumber - 1);

    return url.toString();

}


void UBWidgetUniboardAPI::resize(qreal width, qreal height)
{
    if (qIsNaN(width) || qIsNaN(height)
        || qIsInf(width) || qIsInf(height))
        return;

    if (mGraphicsWidget)
    {
        mGraphicsWidget->resize(width, height);
    }
}


void UBWidgetUniboardAPI::setPreference(const QString& key, QString value)
{
    if (mGraphicsWidget)
    {
            mGraphicsWidget->setPreference(key, value);
    }
}


QString UBWidgetUniboardAPI::preference(const QString& key , const QString& pDefault)
{
    if (mGraphicsWidget && mGraphicsWidget->preferences().contains(key))
    {
        return mGraphicsWidget->preference(key);
    }
    else
    {
        return pDefault;
    }
}


QStringList UBWidgetUniboardAPI::preferenceKeys()
{
    QStringList keys;

    if (mGraphicsWidget)
        keys = mGraphicsWidget->preferences().keys();

    return keys;
}


QString UBWidgetUniboardAPI::uuid() const
{
    if (mGraphicsWidget)
        return UBStringUtils::toCanonicalUuid(mGraphicsWidget->uuid());
    else
        return "";
}


QString UBWidgetUniboardAPI::locale()
{
    return QLocale().name();
}

QString UBWidgetUniboardAPI::lang() const
{
    QString lang = QLocale().name();

    if (lang.length() > 2)
        lang[2] = QLatin1Char('-');

    return lang;
}

void UBWidgetUniboardAPI::sendFileMetadata(QString metaData)
{
    //  Build the QMap of metadata and send it to application
    QMap<QString, QString> qmMetaDatas;
    QDomDocument domDoc;
    domDoc.setContent(metaData);
    QDomElement rootElem = domDoc.documentElement();
    QDomNodeList children = rootElem.childNodes();
    for(int i=0; i<children.size(); i++){
        QDomNode dataNode = children.at(i);
        QDomElement keyElem = dataNode.firstChildElement("key");
        QDomElement valueElem = dataNode.firstChildElement("value");
        qmMetaDatas[keyElem.text()] = valueElem.text();
    }
    UBApplication::boardController->displayMetaData(qmMetaDatas);
}

void UBWidgetUniboardAPI::enableDropOnWidget(bool enable, bool processFileDrop)
{
    if (mGraphicsWidget)
    {
        mGraphicsWidget->setAcceptDrops(enable);
    }

    mProcessFileDrop = processFileDrop;
}

bool UBWidgetUniboardAPI::ProcessDropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!mProcessFileDrop) {
        return false;
    }

    const QMimeData *pMimeData = event->mimeData();

    QString destFileName;
    QString contentType;
    bool downloaded = false;

    Qt::DropActions dropActions = event->possibleActions();
    Qt::MouseButtons dropMouseButtons = event->buttons();
    Qt::KeyboardModifiers dropModifiers = event->modifiers();
    QMimeData *dropMimeData = new QMimeData;
    qDebug() << event->possibleActions();


    if (pMimeData->hasHtml()) { //Dropping element from web browser
        QString qsHtml = pMimeData->html();
        QString url = UBApplication::urlFromHtml(qsHtml);

        if(!url.isEmpty()) {
            QString str = "test string";

            QMimeData mimeData;
            mimeData.setData(tMimeText, str.toLatin1());

            sDownloadFileDesc desc;
            desc.dest = sDownloadFileDesc::graphicsWidget;
            desc.modal = true;
            desc.srcUrl = url;
            desc.currentSize = 0;
            desc.name = QFileInfo(url).fileName();
            desc.totalSize = 0; // The total size will be retrieved during the download

            desc.dropPoint = event->pos().toPoint(); //Passing pure event point. No modifications
            desc.dropActions = dropActions;
            desc.dropMouseButtons = dropMouseButtons;
            desc.dropModifiers = dropModifiers;

            registerIDWidget(UBDownloadManager::downloadManager()->addFileToDownload(desc));

        }

    } else  if (pMimeData->hasUrls()) { //Local file processing
        QUrl curUrl = pMimeData->urls().constFirst();
        QString sUrl = curUrl.toString();

        if (sUrl.startsWith("file://") || sUrl.startsWith("/")) {
            QString fileName = curUrl.toLocalFile();
            QString extention = UBFileSystemUtils::extension(fileName);
            contentType = UBFileSystemUtils::mimeTypeFromFileName(fileName);

            if (supportedTypeHeader(contentType)) {
                destFileName = getObjDir() + QUuid::createUuid().toString() + "." + extention;

                if (!UBFileSystemUtils::copyFile(fileName, destFileName)) {
                    qDebug() << "can't copy from" << fileName << "to" << destFileName;
                    delete dropMimeData;
                    return false;
                }
                downloaded = true;

            }
        }
    }
    qDebug() << destFileName;
    QString mimeText = createMimeText(downloaded, contentType, destFileName);
    dropMimeData->setData(tMimeText, mimeText.toLatin1());

    event->setMimeData(dropMimeData);
    setDropData(mimeText);
    return true;
}

void UBWidgetUniboardAPI::onDownloadFinished(bool pSuccess, sDownloadFileDesc desc, QByteArray pData)
{
    //if widget recieves is waiting for this id then process
    if (!takeIDWidget(desc.id))
        return;

    if (!pSuccess) {
        qDebug() << "can't download the whole data. An error occured";
        return;
    }

    QString contentType = desc.contentTypeHeader;
    QString extention = UBFileSystemUtils::fileExtensionFromMimeType(contentType);

    if (!supportedTypeHeader(contentType)) {
        qDebug() << "actions for mime type" << contentType << "are not supported";
        return;
    }

    QString objDir = getObjDir();
    if (!QDir().exists(objDir)) {
        if (!QDir().mkpath(objDir)) {
            qDebug() << "can't create objects directory path. Check the permissions";
            return;
        }
    }

    QString destFileName = objDir + QUuid::createUuid().toString() + "." + extention;
    QFile destFile(destFileName);

    if (!destFile.open(QIODevice::WriteOnly)) {
        qDebug() << "can't open" << destFileName << "for wrighting";
        return;
    }

    if (destFile.write(pData) == -1) {
        qDebug() << "can't implement data writing";
        return;
    }

    QGraphicsView *tmpView = mGraphicsWidget->scene()->views().at(0);
    QPoint dropPoint(mGraphicsWidget->mapFromScene(tmpView->mapToScene(desc.dropPoint)).toPoint());

    QMimeData dropMimeData;
    QString mimeText = createMimeText(true, contentType, destFileName);
    dropMimeData.setData(tMimeText, mimeText.toLatin1());

    destFile.close();

    //To make js interpreter accept drop event we need to generate move event first.
    QDragMoveEvent pseudoMove(dropPoint, desc.dropActions, &dropMimeData, desc.dropMouseButtons, desc.dropModifiers);
    QApplication::sendEvent(mGraphicsWidget,&pseudoMove);

    QDropEvent readyEvent(dropPoint, desc.dropActions, &dropMimeData, desc.dropMouseButtons, desc.dropModifiers);
    //sending event to destination either it had been downloaded or not
    QApplication::sendEvent(mGraphicsWidget,&readyEvent);
    readyEvent.acceptProposedAction();
}

QString UBWidgetUniboardAPI::createMimeText(bool downloaded, const QString &mimeType, const QString &fileName)
{
    QString mimeXml;
    QXmlStreamWriter writer(&mimeXml);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(tMainSection);

    writer.writeTextElement(tReady, boolToStr(downloaded));

    if (downloaded) {
        if (!mimeType.isEmpty()) {
            writer.writeTextElement(tType, mimeType);  //writing type of element
        }
        if (!QFile::exists(fileName)) {
            qDebug() << "file" << fileName << "doesn't exist";
            return QString();
        }

        QString relatedFileName = fileName;
        relatedFileName = relatedFileName.remove(mGraphicsWidget->getOwnFolder().toLocalFile() + "/");
        writer.writeTextElement(tPath, relatedFileName);   //writing path to created object
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    return mimeXml;
}

bool UBWidgetUniboardAPI::supportedTypeHeader(const QString &typeHeader) const
{
    return     typeHeader.startsWith(imageAlias) || typeHeader.startsWith(imageAliasCap)
            || typeHeader.startsWith(audioAlias) || typeHeader.startsWith(audioAliasCap)
            || typeHeader.startsWith(videoAlias) || typeHeader.startsWith(videoAliasCap);
}

bool UBWidgetUniboardAPI::takeIDWidget(int id)
{
    if (webDownloadIds.contains(id)) {
        webDownloadIds.removeAll(id);
        return true;
    }
    return false;
}

bool UBWidgetUniboardAPI::isDropableData(const QMimeData *pMimeData) const
{
    QString fileName = QString();
    if (pMimeData->hasHtml()) {
        fileName = UBApplication::urlFromHtml(pMimeData->html());
        if (fileName.isEmpty())
            return false;
    } else if (pMimeData->hasUrls()) {
        fileName = pMimeData->urls().at(0).toLocalFile();
        if (fileName.isEmpty())
            return false;
    }

    if (supportedTypeHeader(UBFileSystemUtils::mimeTypeFromFileName(fileName)))
        return true;

    return false;
}


UBDocumentDatastoreAPI::UBDocumentDatastoreAPI(UBGraphicsW3CWidgetItem *graphicsWidget)
    : UBW3CWebStorage(graphicsWidget)
    , mGraphicsW3CWidget(graphicsWidget)
{
    // NOOP
}



UBDocumentDatastoreAPI::~UBDocumentDatastoreAPI()
{
    // NOOP
}


QString UBDocumentDatastoreAPI::key(int index)
{
   QMap<QString, QString> entries = mGraphicsW3CWidget->datastoreEntries();

   if (index < entries.size())
       return entries.keys().at(index);
   else
       return "";

}


QString UBDocumentDatastoreAPI::getItem(const QString& key)
{
    QMap<QString, QString> entries = mGraphicsW3CWidget->datastoreEntries();
    if (entries.contains(key))
    {
        return entries.value(key);
    }
    else
    {
        return "";
    }
}


int UBDocumentDatastoreAPI::length() const
{
   return mGraphicsW3CWidget->datastoreEntries().size();
}


void UBDocumentDatastoreAPI::setItem(const QString& key, const QString& value)
{
    if (mGraphicsW3CWidget)
    {
        mGraphicsW3CWidget->setDatastoreEntry(key, value);
    }
}


void UBDocumentDatastoreAPI::removeItem(const QString& key)
{
    mGraphicsW3CWidget->removeDatastoreEntry(key);
}
void

 UBDocumentDatastoreAPI::clear()
{
    mGraphicsW3CWidget->removeAllDatastoreEntries();
}


UBDatastoreAPI::UBDatastoreAPI(UBGraphicsW3CWidgetItem *widget)
    : QObject(widget)
{
    mDocumentDatastore = new UBDocumentDatastoreAPI(widget);
}


QObject* UBDatastoreAPI::document() const
{
    return mDocumentDatastore;
}




