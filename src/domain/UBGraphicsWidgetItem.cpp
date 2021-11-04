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




#include <QtNetwork>
#include <QtXml>
#include <QWebFrame>

#include "UBGraphicsWidgetItem.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsWidgetItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"

#include "api/UBWidgetUniboardAPI.h"
#include "api/UBW3CWidgetAPI.h"

 #include "board/UBBoardController.h"

#include "core/memcheck.h"
#include "core/UBApplicationController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "network/UBNetworkAccessManager.h"

#include "web/UBWebPage.h"
#include "web/UBWebKitUtils.h"
#include "web/UBWebController.h"

bool UBGraphicsWidgetItem::sInlineJavaScriptLoaded = false;
QStringList UBGraphicsWidgetItem::sInlineJavaScripts;

UBGraphicsWidgetItem::UBGraphicsWidgetItem(const QUrl &pWidgetUrl, QGraphicsItem *parent)
    : QGraphicsWebView(parent)
    , mInitialLoadDone(false)
    , mIsFreezable(true)
    , mIsResizable(false)
    , mLoadIsErronous(false)
    , mCanBeContent(0)
    , mCanBeTool(0)
    , mWidgetUrl(pWidgetUrl)
    , mIsFrozen(false)
    , mIsTakingSnapshot(false)
    , mShouldMoveWidget(false)
    , mUniboardAPI(0)
{
    setData(UBGraphicsItemData::ItemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly

    QGraphicsWebView::setPage(new UBWebPage(this));
    QGraphicsWebView::settings()->setAttribute(QWebSettings::JavaEnabled, true);
    QGraphicsWebView::settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QGraphicsWebView::settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
    QGraphicsWebView::settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    QGraphicsWebView::settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    QGraphicsWebView::settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    QGraphicsWebView::settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
    QGraphicsWebView::settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
#ifdef QT_DEBUG
    QGraphicsWebView::settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif

    page()->setNetworkAccessManager(UBNetworkAccessManager::defaultAccessManager());

    setAcceptDrops(true);
    setAutoFillBackground(false);

    QPalette pagePalette = page()->palette();
    pagePalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    pagePalette.setBrush(QPalette::Window, QBrush(Qt::transparent));
    page()->setPalette(pagePalette);

    QPalette viewPalette = palette();
    pagePalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    viewPalette.setBrush(QPalette::Window, QBrush(Qt::transparent));
    setPalette(viewPalette);

    setDelegate(new UBGraphicsWidgetItemDelegate(this));

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    QGraphicsWebView::setAcceptHoverEvents(true);
}


UBGraphicsWidgetItem::~UBGraphicsWidgetItem()
{
    /* NOOP */
}

void UBGraphicsWidgetItem::initialize()
{
    setMinimumSize(nominalSize());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); // Necessary to set if we want z value to be assigned correctly

    if (Delegate() && Delegate()->frame() && resizable())
        Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

    QPalette palette = page()->palette();
    palette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    page()->setPalette(palette);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(page()->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared,
            this, &UBGraphicsWidgetItem::javaScriptWindowObjectCleared);
    connect(page(), SIGNAL(geometryChangeRequested(const QRect&)), this, SLOT(geometryChangeRequested(const QRect&)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(mainFrameLoadFinished (bool)));
    connect(page()->mainFrame(), SIGNAL(initialLayoutCompleted()), this, SLOT(initialLayoutCompleted()));
    connect(page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(onLinkClicked(const QUrl&)));
}

void UBGraphicsWidgetItem::onLinkClicked(const QUrl& url)
{
    load(url);
}

void UBGraphicsWidgetItem::initialLayoutCompleted()
{
    mInitialLoadDone = true;
}

QUrl UBGraphicsWidgetItem::mainHtml()
{
    return mMainHtmlUrl;
}

void UBGraphicsWidgetItem::loadMainHtml()
{
    mInitialLoadDone = false;
    load(mMainHtmlUrl);
}

QUrl UBGraphicsWidgetItem::widgetUrl()
{
    return mWidgetUrl;
}

QString UBGraphicsWidgetItem::mainHtmlFileName()
{
    return mMainHtmlFileName;
}

bool UBGraphicsWidgetItem::canBeContent()
{
    // if we under MAC OS
    #if defined(Q_OS_MAC)
        return mCanBeContent & UBGraphicsWidgetItem::type_MAC;
    #endif

    // if we under UNIX OS
    #if defined(Q_OS_UNIX)
        return mCanBeContent & UBGraphicsWidgetItem::type_UNIX;
    #endif

    // if we under WINDOWS OS
    #if defined(Q_OS_WIN)
        return mCanBeContent & UBGraphicsWidgetItem::type_WIN;
    #endif
}

bool UBGraphicsWidgetItem::canBeTool()
{
    // if we under MAC OS
    #if defined(Q_OS_MAC)
        return mCanBeTool & UBGraphicsWidgetItem::type_MAC;
    #endif

        // if we under UNIX OS
    #if defined(Q_OS_UNIX)
        return mCanBeTool & UBGraphicsWidgetItem::type_UNIX;
    #endif

        // if we under WINDOWS OS
    #if defined(Q_OS_WIN)
        return mCanBeTool & UBGraphicsWidgetItem::type_WIN;
    #endif
}

QString UBGraphicsWidgetItem::preference(const QString& key) const
{
    return mPreferences.value(key);
}

void UBGraphicsWidgetItem::setPreference(const QString& key, QString value)
{
    if (key == "" || (mPreferences.contains(key) && mPreferences.value(key) == value))
        return;

    mPreferences.insert(key, value);
    if (scene())
        scene()->setModified(true);
}

QMap<QString, QString> UBGraphicsWidgetItem::preferences() const
{
    return mPreferences;
}


void UBGraphicsWidgetItem::removePreference(const QString& key)
{
    mPreferences.remove(key);
}


void UBGraphicsWidgetItem::removeAllPreferences()
{
    mPreferences.clear();
}

QString UBGraphicsWidgetItem::datastoreEntry(const QString& key) const
{
    if (mDatastore.contains(key))
        return mDatastore.value(key);
    else
        return QString();
}

void UBGraphicsWidgetItem::setDatastoreEntry(const QString& key, QString value)
{
    if (key == "" || (mDatastore.contains(key) && mDatastore.value(key) == value))
        return;

    mDatastore.insert(key, value);
    if (scene())
        scene()->setModified(true);
}

QMap<QString, QString> UBGraphicsWidgetItem::datastoreEntries() const
{
    return mDatastore;
}


void UBGraphicsWidgetItem::removeDatastoreEntry(const QString& key)
{
    mDatastore.remove(key);
}


void UBGraphicsWidgetItem::removeAllDatastoreEntries()
{
    mDatastore.clear();
}

void UBGraphicsWidgetItem::removeScript()
{
    if (page() && page()->mainFrame())
        page()->mainFrame()->evaluateJavaScript("if(widget && widget.onremove) { widget.onremove();}");
}

void UBGraphicsWidgetItem::processDropEvent(QGraphicsSceneDragDropEvent *event)
{
    mUniboardAPI->ProcessDropEvent(event);
}
bool UBGraphicsWidgetItem::isDropableData(const QMimeData *data) const
{
    return mUniboardAPI->isDropableData(data);
}

QUrl UBGraphicsWidgetItem::getOwnFolder() const
{
    return ownFolder;
}

void UBGraphicsWidgetItem::setOwnFolder(const QUrl &newFolder)
{
    ownFolder = newFolder;
}

void UBGraphicsWidgetItem::setSnapshotPath(const QUrl &newFilePath)
{
    SnapshotFile = newFilePath;
}

QUrl UBGraphicsWidgetItem::getSnapshotPath()
{
    return SnapshotFile;
}

void UBGraphicsWidgetItem::clearSource()
{
    UBFileSystemUtils::deleteDir(getOwnFolder().toLocalFile());
    UBFileSystemUtils::deleteFile(getSnapshotPath().toLocalFile());
}

void UBGraphicsWidgetItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

QSize UBGraphicsWidgetItem::nominalSize() const
{
    return mNominalSize;
}

bool UBGraphicsWidgetItem::hasLoadedSuccessfully() const
{
    return (mInitialLoadDone && !mLoadIsErronous);
}

bool UBGraphicsWidgetItem::freezable()
{
    return mIsFreezable;
}

bool UBGraphicsWidgetItem::resizable()
{
    return mIsResizable;
}

bool UBGraphicsWidgetItem::isFrozen()
{
    return mIsFrozen;
}

QPixmap UBGraphicsWidgetItem::snapshot()
{
    return mSnapshot;
}

QPixmap UBGraphicsWidgetItem::takeSnapshot()
{
    mIsTakingSnapshot = true;

    QPixmap pixmap(size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    QStyleOptionGraphicsItem options;
    paint(&painter, &options);

    mIsTakingSnapshot = false;

    mSnapshot = pixmap;

    return pixmap;
}

void UBGraphicsWidgetItem::setSnapshot(const QPixmap& pix)
{
    mSnapshot = pix;
}

UBGraphicsScene* UBGraphicsWidgetItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}

int UBGraphicsWidgetItem::widgetType(const QUrl& pUrl)
{
    QString mime = UBFileSystemUtils::mimeTypeFromFileName(pUrl.toString());

    if (mime == "application/vnd.apple-widget")
        return UBWidgetType::Apple;
    else if (mime == "application/widget")
        return UBWidgetType::W3C;
    else
        return UBWidgetType::Other;
}

QString UBGraphicsWidgetItem::widgetName(const QUrl& widgetPath)
{
    QString name;
    QString version;
    QFile w3CConfigFile(widgetPath.toLocalFile() + "/config.xml");
    QFile appleConfigFile(widgetPath.toLocalFile() + "/Info.plist");

    if (w3CConfigFile.exists() && w3CConfigFile.open(QFile::ReadOnly)) {
        QDomDocument doc;
        doc.setContent(w3CConfigFile.readAll());
        QDomElement root = doc.firstChildElement("widget");
        if (!root.isNull()) {
            QDomElement nameElement = root.firstChildElement("name");
            if (!nameElement.isNull())
                name = nameElement.text();
            version = root.attribute("version", "");
        }
        w3CConfigFile.close();
    }
    else if (appleConfigFile.exists() && appleConfigFile.open(QFile::ReadOnly)) {
        QDomDocument doc;
        doc.setContent(appleConfigFile.readAll());
        QDomElement root = doc.firstChildElement("plist");
        if (!root.isNull()) {
            QDomElement dictElement = root.firstChildElement("dict");
            if (!dictElement.isNull()) {
                QDomNodeList childNodes  = dictElement.childNodes();

                /* looking for something like
                 * ..
                 * <key>CFBundleDisplayName</key>
                 * <string>brain scans</string>
                 * ..
                 */

                for(int i = 0; i < childNodes.count() - 1; i++) {
                    if (childNodes.at(i).isElement()) {
                        QDomElement elKey = childNodes.at(i).toElement();
                        if (elKey.text() == "CFBundleDisplayName") {
                            if (childNodes.at(i + 1).isElement()) {
                               QDomElement elValue = childNodes.at(i + 1).toElement();
                               name = elValue.text();
                            }
                        }
                        else if (elKey.text() == "CFBundleShortVersionString") {
                            if (childNodes.at(i + 1).isElement()) {
                               QDomElement elValue = childNodes.at(i + 1).toElement();
                               version = elValue.text();
                            }
                        }
                    }
                }
            }
        }
        appleConfigFile.close();
    }
    QString result;

    if (name.length() > 0) {
        result = name;
        if (version.length() > 0) {
            result += " ";
            result += version;
        }
    }
    return result;
}

QString UBGraphicsWidgetItem::iconFilePath(const QUrl& pUrl)
{
    /* TODO UB 4.x read config.xml widget.icon param first */

    QStringList files;

    files << "icon.svg";  /* W3C widget default 1 */
    files << "icon.ico";  /* W3C widget default 2 */
    files << "icon.png";  /* W3C widget default 3 */
    files << "icon.gif";  /* W3C widget default 4 */
    files << "Icon.png";  /* Apple widget default */

    QString file = UBFileSystemUtils::getFirstExistingFileFromList(pUrl.toLocalFile(), files);
    /* default */
    if (file.length() == 0)
    {
        file = QString(":/images/defaultWidgetIcon.png");
    }
    return file;
}

void UBGraphicsWidgetItem::freeze()
{
    QPixmap pix = takeSnapshot();
    mIsFrozen = true;
    setSnapshot(pix);
}

void UBGraphicsWidgetItem::unFreeze()
{
    mIsFrozen = false;
}

bool UBGraphicsWidgetItem::event(QEvent *event)
{
    if (mShouldMoveWidget && event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(event);
        if (mouseMoveEvent->buttons() & Qt::LeftButton) {
            QPointF scenePos = mapToScene(mouseMoveEvent->pos());
            QPointF newPos = pos() + scenePos - mLastMousePos;
            setPos(newPos);
            mLastMousePos = scenePos;
            event->accept();
            return true;
        }
    }
    else if (event->type() == QEvent::ShortcutOverride)
        event->accept();

    return QGraphicsWebView::event(event);
}

void UBGraphicsWidgetItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    processDropEvent(event);
    QGraphicsWebView::dropEvent(event);
}

void UBGraphicsWidgetItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!Delegate()->mousePressEvent(event))
        setSelected(true); /* forcing selection */

    QGraphicsWebView::mousePressEvent(event);

    // did webkit consume the mouse press ?
    mShouldMoveWidget = !event->isAccepted() && (event->buttons() & Qt::LeftButton);

    mLastMousePos = mapToScene(event->pos());

    event->accept();
}

void UBGraphicsWidgetItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mShouldMoveWidget = false;

    Delegate()->mouseReleaseEvent(event);
    QGraphicsWebView::mouseReleaseEvent(event);
}

void UBGraphicsWidgetItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    sendJSEnterEvent();
    Delegate()->hoverEnterEvent(event);
}
void UBGraphicsWidgetItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    sendJSLeaveEvent();
    Delegate()->hoverLeaveEvent(event);
}

void UBGraphicsWidgetItem::sendJSEnterEvent()
{
    if (page() && page()->mainFrame())
        page()->mainFrame()->evaluateJavaScript("if(widget && widget.onenter) { widget.onenter();}");
}

void UBGraphicsWidgetItem::sendJSLeaveEvent()
{
    if (page() && page()->mainFrame())
        page()->mainFrame()->evaluateJavaScript("if(widget && widget.onleave) { widget.onleave();}");
}

void UBGraphicsWidgetItem::injectInlineJavaScript()
{
    if (!sInlineJavaScriptLoaded) {
        sInlineJavaScripts = UBApplication::applicationController->widgetInlineJavaScripts();
        sInlineJavaScriptLoaded = true;
    }

    foreach(QString script, sInlineJavaScripts)
        page()->mainFrame()->evaluateJavaScript(script);
}

void UBGraphicsWidgetItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{


    QGraphicsWebView::paint(painter, option, widget);


    if (!mInitialLoadDone) {
        QString message;

        message = tr("Loading ...");

        painter->setFont(QFont("Arial", 12));

        QFontMetrics fm = painter->fontMetrics();
        QRect txtBoundingRect = fm.boundingRect(message);

        txtBoundingRect.moveCenter(rect().center().toPoint());
        txtBoundingRect.adjust(-10, -5, 10, 5);

        painter->setPen(Qt::NoPen);
        painter->setBrush(UBSettings::paletteColor);
        painter->drawRoundedRect(txtBoundingRect, 3, 3);

        painter->setPen(Qt::white);
        painter->drawText(rect(), Qt::AlignCenter, message);
    }

    Delegate()->postpaint(painter, option, widget);
}

void UBGraphicsWidgetItem::geometryChangeRequested(const QRect& geom)
{
    resize(geom.width(), geom.height());
}

void UBGraphicsWidgetItem::javaScriptWindowObjectCleared()
{
    injectInlineJavaScript();

    if(!mUniboardAPI)
        mUniboardAPI = new UBWidgetUniboardAPI(scene(), this);

    page()->mainFrame()->addToJavaScriptWindowObject("sankore", mUniboardAPI);

}

void UBGraphicsWidgetItem::mainFrameLoadFinished (bool ok)
{
    mLoadIsErronous = !ok;
    update(boundingRect());

    if (mInitialLoadDone && scene() && scene()->renderingContext() == UBGraphicsScene::Screen)
        takeSnapshot();
}

void UBGraphicsWidgetItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (Delegate()->wheelEvent(event))
    {
        QGraphicsWebView::wheelEvent(event);
        event->accept();
    }
}

QVariant UBGraphicsWidgetItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == QGraphicsItem::ItemSelectedHasChanged) &&  scene()) {
        if (isSelected())
            scene()->setActiveWindow(this);
        else
            if(scene()->activeWindow() == this)
                scene()->setActiveWindow(0);
    }

    QVariant newValue = Delegate()->itemChange(change, value);
    return QGraphicsWebView::itemChange(change, newValue);
}

void UBGraphicsWidgetItem::resize(qreal w, qreal h)
{
    UBGraphicsWidgetItem::resize(QSizeF(w, h));
}


void UBGraphicsWidgetItem::resize(const QSizeF & pSize)
{
    if (pSize != size()) {
        QGraphicsWebView::setMaximumSize(pSize.width(), pSize.height());
        QGraphicsWebView::resize(pSize.width(), pSize.height());
        if (Delegate())
            Delegate()->positionHandles();
        if (scene())
            scene()->setModified(true);
    }
}

QSizeF UBGraphicsWidgetItem::size() const
{
    return QGraphicsWebView::size();
}



UBGraphicsAppleWidgetItem::UBGraphicsAppleWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent)
    : UBGraphicsWidgetItem(pWidgetUrl, parent)
{
    QString path = pWidgetUrl.toLocalFile();

    if (!path.endsWith(".wdgt") && !path.endsWith(".wdgt/")) {
        int lastSlashIndex = path.lastIndexOf("/");
        if (lastSlashIndex > 0)
            path = path.mid(0, lastSlashIndex + 1);
    }

    QFile plistFile(path + "/Info.plist");
    plistFile.open(QFile::ReadOnly);

    QByteArray plistBin = plistFile.readAll();
    QString plist = QString::fromUtf8(plistBin);

    int mainHtmlIndex = plist.indexOf("MainHTML");
    int mainHtmlIndexStart = plist.indexOf("<string>", mainHtmlIndex);
    int mainHtmlIndexEnd = plist.indexOf("</string>", mainHtmlIndexStart);

    if (mainHtmlIndex > -1 && mainHtmlIndexStart > -1 && mainHtmlIndexEnd > -1)
        mMainHtmlFileName = plist.mid(mainHtmlIndexStart + 8, mainHtmlIndexEnd - mainHtmlIndexStart - 8);

    mMainHtmlUrl = pWidgetUrl;
    mMainHtmlUrl.setPath(pWidgetUrl.path() + "/" + mMainHtmlFileName);

    load(mMainHtmlUrl);

    QPixmap defaultPixmap(pWidgetUrl.toLocalFile() + "/Default.png");

    setMaximumSize(defaultPixmap.size());

    mNominalSize = defaultPixmap.size();

    initialize();
}


UBGraphicsAppleWidgetItem::~UBGraphicsAppleWidgetItem()
{
    /* NOOP */
}

void UBGraphicsAppleWidgetItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

UBItem* UBGraphicsAppleWidgetItem::deepCopy() const
{
    UBGraphicsAppleWidgetItem *appleWidget = new UBGraphicsAppleWidgetItem(QGraphicsWebView::url(), parentItem());

    copyItemParameters(appleWidget);

    return appleWidget;

}

void UBGraphicsAppleWidgetItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsAppleWidgetItem *cp = dynamic_cast<UBGraphicsAppleWidgetItem*>(copy);
    if (cp)
    {
        foreach(QString key, mPreferences.keys())
        {
            cp->setPreference(key, mPreferences.value(key));
        }

        foreach(QString key, mDatastore.keys())
        {
            cp->setDatastoreEntry(key, mDatastore.value(key));
        }

        cp->setSourceUrl(this->sourceUrl());
        cp->setZValue(this->zValue());
    }

}




bool UBGraphicsW3CWidgetItem::sTemplateLoaded = false;
QString UBGraphicsW3CWidgetItem::sNPAPIWrappperConfigTemplate;
QMap<QString, QString> UBGraphicsW3CWidgetItem::sNPAPIWrapperTemplates;

UBGraphicsW3CWidgetItem::UBGraphicsW3CWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent)
    : UBGraphicsWidgetItem(pWidgetUrl, parent)
    , mW3CWidgetAPI(0)
{
    QString path = pWidgetUrl.toLocalFile();
    QDir potentialDir(path);

    if (!path.endsWith(".wgt") && !path.endsWith(".wgt/") && !potentialDir.exists()) {
        int lastSlashIndex = path.lastIndexOf("/");
        if (lastSlashIndex > 0)
            path = path.mid(0, lastSlashIndex + 1);
    }

    if (!path.endsWith("/"))
        path += "/";

    int width = 300;
    int height = 150;

    QFile configFile(path + "config.xml");
    configFile.open(QFile::ReadOnly);

    QDomDocument doc;
    doc.setContent(configFile.readAll());
    QDomNodeList widgetDomList = doc.elementsByTagName("widget");

    if (widgetDomList.count() > 0) {
        QDomElement widgetElement = widgetDomList.item(0).toElement();

        width = widgetElement.attribute("width", "300").toInt();
        height = widgetElement.attribute("height", "150").toInt();

        mMetadatas.id = widgetElement.attribute("id", "");

        /* some early widget (<= 4.3.4) where using identifier instead of id */
        if (mMetadatas.id.length() == 0)
             mMetadatas.id = widgetElement.attribute("identifier", "");

        mMetadatas.version = widgetElement.attribute("version", "");

        /* TODO UB 4.x map properly ub namespace */
        mIsResizable = widgetElement.attribute("ub:resizable", "false") == "true";
        mIsFreezable = widgetElement.attribute("ub:freezable", "true") == "true";

        QString roles = widgetElement.attribute("ub:roles", "content tool").trimmed().toLower();

        /* ------------------------------ */

        if (roles == "" || roles.contains("tool"))
            mCanBeTool = UBGraphicsWidgetItem::type_ALL;

        if (roles.contains("twin"))
            mCanBeTool |= UBGraphicsWidgetItem::type_WIN;

        if (roles.contains("tmac"))
            mCanBeTool |= UBGraphicsWidgetItem::type_MAC;

        if (roles.contains("tunix"))
            mCanBeTool |= UBGraphicsWidgetItem::type_UNIX;

        /* --------- */

        if (roles == "" || roles.contains("content"))
            mCanBeContent = UBGraphicsWidgetItem::type_ALL;

        if (roles.contains("cwin"))
            mCanBeContent |= UBGraphicsWidgetItem::type_WIN;

        if (roles.contains("cmac"))
            mCanBeContent |= UBGraphicsWidgetItem::type_MAC;

        if (roles.contains("cunix"))
            mCanBeContent |= UBGraphicsWidgetItem::type_UNIX;

        //------------------------------//

        QDomNodeList contentDomList = widgetElement.elementsByTagName("content");

        if (contentDomList.count() > 0) {
            QDomElement contentElement = contentDomList.item(0).toElement();
            mMainHtmlFileName = contentElement.attribute("src", "");
        }

        mMetadatas.name = textForSubElementByLocale(widgetElement, "name", QLocale::system());
        mMetadatas.description = textForSubElementByLocale(widgetElement, "description ", QLocale::system());

        QDomNodeList authorDomList = widgetElement.elementsByTagName("author");

        if (authorDomList.count() > 0) {
            QDomElement authorElement = authorDomList.item(0).toElement();

            mMetadatas.author = authorElement.text();
            mMetadatas.authorHref = authorElement.attribute("href", "");
            mMetadatas.authorEmail = authorElement.attribute("email ", "");
        }

        QDomNodeList propertiesDomList = widgetElement.elementsByTagName("preference");

        for (int i = 0; i < propertiesDomList.length(); i++) {
            QDomElement preferenceElement = propertiesDomList.at(i).toElement();
            QString prefName = preferenceElement.attribute("name", "");

            if (prefName.length() > 0) {
                QString prefValue = preferenceElement.attribute("value", "");
                bool readOnly = (preferenceElement.attribute("readonly", "false") == "true");

                mPreferences.insert(prefName, PreferenceValue(prefValue, readOnly));
            }
        }
    }

    if (mMainHtmlFileName.length() == 0) {
        QFile defaultStartFile(path + "index.htm");

        if (defaultStartFile.exists())
            mMainHtmlFileName = "index.htm";
        else {
            QFile secondDefaultStartFile(path + "index.html");

            if (secondDefaultStartFile.exists())
                mMainHtmlFileName = "index.html";
        }
    }

    mMainHtmlUrl = pWidgetUrl;
    mMainHtmlUrl.setPath(pWidgetUrl.path() + "/" + mMainHtmlFileName);
    /* is it a valid local file ? */
    QFile f(mMainHtmlUrl.toLocalFile());

    if(!f.exists())
        mMainHtmlUrl = QUrl(mMainHtmlFileName);

    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(javaScriptWindowObjectCleared()));

    load(mMainHtmlUrl);

    setMaximumSize(QSize(width, height));

    mNominalSize = QSize(width, height);

    initialize();
    setOwnFolder(pWidgetUrl);
}

UBGraphicsW3CWidgetItem::~UBGraphicsW3CWidgetItem()
{
    /* NOOP */
}

void UBGraphicsW3CWidgetItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

UBItem* UBGraphicsW3CWidgetItem::deepCopy() const
{
    UBGraphicsW3CWidgetItem *copy = new UBGraphicsW3CWidgetItem(mWidgetUrl, parentItem());
    copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable
    copyItemParameters(copy);

    return copy;
}

QMap<QString, UBGraphicsW3CWidgetItem::PreferenceValue> UBGraphicsW3CWidgetItem::preferences()
{
    return mPreferences;
}

UBGraphicsW3CWidgetItem::Metadata UBGraphicsW3CWidgetItem::metadatas() const
{
    return mMetadatas;
}

QString UBGraphicsW3CWidgetItem::createNPAPIWrapper(const QString& url, const QString& pMimeType, const QSize& sizeHint, const QString& pName)
{
    const QString userWidgetPath = UBSettings::settings()->userInteractiveDirectory() + "/" + tr("Web");
    QDir userWidgetDir(userWidgetPath);

    return createNPAPIWrapperInDir(url, userWidgetDir, pMimeType, sizeHint, pName);
}

QString UBGraphicsW3CWidgetItem::createNPAPIWrapperInDir(const QString& pUrl, const QDir& pDir, const QString& pMimeType, const QSize& sizeHint, const QString& pName)
{
    QString url = pUrl;
    url = UBFileSystemUtils::removeLocalFilePrefix(url);
    QString name = pName;

    QFileInfo fi(url);

    if (name.length() == 0)
        name = fi.baseName();

    if (fi.exists())
        url = fi.fileName();

    loadNPAPIWrappersTemplates();

    QString htmlTemplate;

    if (pMimeType.length() > 0 && sNPAPIWrapperTemplates.contains(pMimeType))
        htmlTemplate = sNPAPIWrapperTemplates.value(pMimeType);
    else {
        QString extension = UBFileSystemUtils::extension(url);
        if (sNPAPIWrapperTemplates.contains(extension))
            htmlTemplate = sNPAPIWrapperTemplates.value(extension);
    }

    if (htmlTemplate.length() > 0) {
        htmlTemplate = htmlTemplate.replace(QString("{in.url}"), url)
            .replace(QString("{in.width}"), QString("%1").arg(sizeHint.width()))
            .replace(QString("{in.height}"), QString("%1").arg(sizeHint.height()));

        QString configTemplate = sNPAPIWrappperConfigTemplate
            .replace(QString("{in.id}"), url)
            .replace(QString("{in.width}"), QString("%1").arg(sizeHint.width()))
            .replace(QString("{in.height}"), QString("%1").arg(sizeHint.height()))
            .replace(QString("{in.name}"), name)
            .replace(QString("{in.startFile}"), QString("index.htm"));

        QString dirPath = pDir.path();
        if (!pDir.exists())
            pDir.mkpath(dirPath);

        QString widgetLibraryPath = dirPath + "/" + name + ".wgt";
        QDir widgetLibraryDir(widgetLibraryPath);

        if (widgetLibraryDir.exists())
            if (!UBFileSystemUtils::deleteDir(widgetLibraryDir.path()))
                qWarning() << "Cannot delete old widget " << widgetLibraryDir.path();

        widgetLibraryDir.mkpath(widgetLibraryPath);
        if (fi.exists()) {
            QString target = widgetLibraryPath + "/" + fi.fileName();
            QString source = pUrl;
            source = UBFileSystemUtils::removeLocalFilePrefix(source);
            QFile::copy(source, target);
        }

        QFile configFile(widgetLibraryPath + "/config.xml");

        if (!configFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open file " << configFile.fileName();
            return QString();
        }

        QTextStream outConfig(&configFile);
        outConfig.setCodec("UTF-8");

        outConfig << configTemplate;
        configFile.close();

        QFile indexFile(widgetLibraryPath + "/index.htm");

        if (!indexFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open file " << indexFile.fileName();
            return QString();
        }

        QTextStream outIndex(&indexFile);
        outIndex.setCodec("UTF-8");

        outIndex << htmlTemplate;
        indexFile.close();

        return widgetLibraryPath;
    }
    else
        return QString();
}

QString UBGraphicsW3CWidgetItem::createHtmlWrapperInDir(const QString& html, const QDir& pDir, const QSize& sizeHint, const QString& pName)
{
    QString widgetPath = pDir.path() + "/" + pName + ".wgt";
    widgetPath = UBFileSystemUtils::nextAvailableFileName(widgetPath);
    QDir widgetDir(widgetPath);

    if (!widgetDir.exists())
        widgetDir.mkpath(widgetDir.path());

    QFile configFile(widgetPath + "/" + "config.xml");

    if (configFile.exists())
        configFile.remove(configFile.fileName());

    if (!configFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file " << configFile.fileName();
        return "";
    }

    QTextStream outConfig(&configFile);
    outConfig.setCodec("UTF-8");
    outConfig << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    outConfig << "<widget xmlns=\"http://www.w3.org/ns/widgets\"" << endl;
    outConfig << "    xmlns:ub=\"http://uniboard.mnemis.com/widgets\"" << endl;
    outConfig << "    id=\"http://uniboard.mnemis.com/" << pName << "\"" <<endl;

    outConfig << "    version=\"1.0\"" << endl;
    outConfig << "    width=\"" << sizeHint.width() << "\"" << endl;
    outConfig << "    height=\"" << sizeHint.height() << "\"" << endl;
    outConfig << "    ub:resizable=\"true\">" << endl;

    outConfig << "  <name>" << pName << "</name>" << endl;
    outConfig << "  <content src=\"" << pName << ".html\"/>" << endl;

    outConfig << "</widget>" << endl;

    configFile.close();

    const QString fullHtmlFileName = widgetPath + "/" + pName + ".html";

    QFile widgetHtmlFile(fullHtmlFileName);
    if (widgetHtmlFile.exists())
        widgetHtmlFile.remove(widgetHtmlFile.fileName());
    if (!widgetHtmlFile.open(QIODevice::WriteOnly)) {
        qWarning() << "cannot open file " << widgetHtmlFile.fileName();
        return QString();
    }

    QTextStream outStartFile(&widgetHtmlFile);
    outStartFile.setCodec("UTF-8");

    outStartFile << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">" << endl;
    outStartFile << "<html>" << endl;
    outStartFile << "<head>" << endl;
    outStartFile << "    <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">" << endl;
    outStartFile << "</head>" << endl;
    outStartFile << "  <body>" << endl;
    outStartFile << html << endl;
    outStartFile << "  </body>" << endl;
    outStartFile << "</html>" << endl;

    widgetHtmlFile.close();

    return widgetPath;
}

QString UBGraphicsW3CWidgetItem::freezedWidgetPage()
{
    static QString defaultcontent;

    if (defaultcontent.isNull()) {
        QString freezedWidgetDefaultContentFilePath = freezedWidgetFilePath();
        QFile wrapperFile(freezedWidgetDefaultContentFilePath);
        if (!wrapperFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "can't open wrapper file " + freezedWidgetDefaultContentFilePath;
            defaultcontent = "";
        }
        else {
            QByteArray arr = wrapperFile.readAll();
            if (!arr.isEmpty())
                defaultcontent = QString(arr);
            else {
                qDebug() << "content of " + freezedWidgetDefaultContentFilePath + "is empty";
                defaultcontent = QString();
            }
        }
    }

    return defaultcontent;
}

QString UBGraphicsW3CWidgetItem::freezedWidgetFilePath()
{
    return UBPlatformUtils::applicationResourcesDirectory() + "/etc/" + "freezedWidgetWrapper.html";
}

bool UBGraphicsW3CWidgetItem::hasNPAPIWrapper(const QString& pMimeType)
{
    loadNPAPIWrappersTemplates();

    return sNPAPIWrapperTemplates.contains(pMimeType);
}

void UBGraphicsW3CWidgetItem::javaScriptWindowObjectCleared()
{
    UBGraphicsWidgetItem::javaScriptWindowObjectCleared();

    if(!mW3CWidgetAPI)
        mW3CWidgetAPI = new UBW3CWidgetAPI(this);

    page()->mainFrame()->addToJavaScriptWindowObject("widget", mW3CWidgetAPI);

}

void UBGraphicsW3CWidgetItem::loadNPAPIWrappersTemplates()
{
    if (!sTemplateLoaded) {
        sNPAPIWrapperTemplates.clear();

        QString etcPath = UBPlatformUtils::applicationResourcesDirectory() + "/etc/";

        QDir etcDir(etcPath);

        foreach(QString fileName, etcDir.entryList()) {
            if (fileName.startsWith("npapi-wrapper") && (fileName.endsWith(".htm") || fileName.endsWith(".html"))) {

                QString htmlContent = UBFileSystemUtils::readTextFile(etcPath + fileName);

                if (htmlContent.length() > 0) {
                    QStringList tokens = fileName.split(".");

                    if (tokens.length() >= 4) {
                        QString mime = tokens.at(tokens.length() - 4 );
                        mime += "/" + tokens.at(tokens.length() - 3);

                        QString fileExtension = tokens.at(tokens.length() - 2);

                        sNPAPIWrapperTemplates.insert(mime, htmlContent);
                        sNPAPIWrapperTemplates.insert(fileExtension, htmlContent);
                    }
                }
            }
        }
        sNPAPIWrappperConfigTemplate = UBFileSystemUtils::readTextFile(etcPath + "npapi-wrapper.config.xml");
        sTemplateLoaded = true;
    }
}

QString UBGraphicsW3CWidgetItem::textForSubElementByLocale(QDomElement rootElement, QString subTagName, QLocale locale)
{
    QDomNodeList subList = rootElement.elementsByTagName(subTagName);

    QString lang = locale.name();

    if (lang.length() > 2)
        lang[2] = QLatin1Char('-');

    if (subList.count() > 1) {
        for(int i = 0; i < subList.count(); i++) {
            QDomNode node = subList.at(i);
            QDomElement element = node.toElement();

            QString configLang = element.attribute("xml:lang", "");

            if(lang == configLang || (configLang.length() == 2 && configLang == lang.left(2)))
                 return element.text();
        }
    }

    if (subList.count() >= 1) {
        QDomElement element = subList.item(0).toElement();
        return element.text();
    }

    return QString();
}

void UBGraphicsW3CWidgetItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsW3CWidgetItem *cp = dynamic_cast<UBGraphicsW3CWidgetItem*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setSourceUrl(this->sourceUrl());

        cp->resize(this->size());

        foreach(QString key, this->UBGraphicsWidgetItem::preferences().keys())
        {
            cp->setPreference(key, UBGraphicsWidgetItem::preferences().value(key));
        }

        foreach(QString key, mDatastore.keys())
        {
            cp->setDatastoreEntry(key, mDatastore.value(key));
        }

        cp->setZValue(this->zValue());
    }
}

