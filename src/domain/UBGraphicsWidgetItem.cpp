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




#include <QtNetwork>
#include <QtXml>
#include <QWebChannel>
#include <QWebEngineView>

#include "UBGraphicsWidgetItem.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsWidgetItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"
#include "UBWebEngineView.h"

#include "api/UBWidgetUniboardAPI.h"
#include "api/UBW3CWidgetAPI.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"

#include "core/memcheck.h"
#include "core/UBApplicationController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "web/UBWebController.h"
#include "web/simplebrowser/webpage.h"

bool UBGraphicsWidgetItem::sInlineJavaScriptLoaded = false;
QStringList UBGraphicsWidgetItem::sInlineJavaScripts;

#ifndef Q_OS_WIN
/*
 * workaround for a bug related to (at least) QTBUG-79216 - to be removed when bug is fixed
*/
const QMap<Qt::Key, QString> UBGraphicsWidgetItem::sDeadKeys
{
    {Qt::Key_Dead_Circumflex, "^"},   // ^ (Caret)
    {Qt::Key_Dead_Grave, "`"},        // ` (Backtick)
    {Qt::Key_Dead_Tilde, "~"},        // ~ (Tilde)
    {Qt::Key_Dead_Acute, "´"},        // ´ (Acute Accent)
    {Qt::Key_Dead_Diaeresis, "¨"}     // ¨ (Diaeresis)
};

const QMap<QString, QString> UBGraphicsWidgetItem::sAccentedCharacters
{
    // Dead Key: ^
    {"^a", u8"â"},
    {"^A", u8"Â"},
    {"^e", u8"ê"},
    {"^E", u8"Ê"},
    {"^i", u8"î"},
    {"^I", u8"Î"},
    {"^o", u8"ô"},
    {"^O", u8"Ô"},
    {"^u", u8"û"},
    {"^U", u8"Û"},

    // Dead Key: ´
    {"´a", u8"á"},
    {"´A", u8"Á"},
    {"´e", u8"é"},
    {"´E", u8"É"},
    {"´i", u8"í"},
    {"´I", u8"Í"},
    {"´o", u8"ó"},
    {"´O", u8"Ó"},
    {"´u", u8"ú"},
    {"´U", u8"Ú"},

    // Dead Key: `
    {"`a", u8"à"},
    {"`A", u8"À"},
    {"`e", u8"è"},
    {"`E", u8"È"},
    {"`i", u8"ì"},
    {"`I", u8"Ì"},
    {"`o", u8"ò"},
    {"`O", u8"Ò"},
    {"`u", u8"ù"},
    {"`U", u8"Ù"},

    // Dead Key: ~
    {"~n", u8"ñ"},
    {"~N", u8"Ñ"},

    // Dead Key: '
    {"'c", u8"ç"},
    {"'C", u8"Ç"},

    // Dead Key: ¨
    {"¨a", u8"ä"},
    {"¨A", u8"Ä"},
    {"¨e", u8"ë"},
    {"¨E", u8"Ë"},
    {"¨i", u8"ï"},
    {"¨I", u8"Ï"},
    {"¨o", u8"ö"},
    {"¨O", u8"Ö"},
    {"¨u", u8"ü"},
    {"¨U", u8"Ü"},
    {"¨y", u8"ÿ"},
    {"¨Y", u8"Ÿ"}
};
#endif

UBGraphicsWidgetItem::UBGraphicsWidgetItem(const QUrl &pWidgetUrl, QGraphicsItem *parent)
    : QGraphicsProxyWidget(parent)
    , mInitialLoadDone(false)
    , mIsFreezable(true)
    , mIsResizable(false)
    , mLoadIsErronous(false)
    , mCanBeContent(0)
    , mCanBeTool(0)
    , mWidgetUrl(pWidgetUrl)
    , mIsFrozen(false)
    , mIsWebActive(true)
    , mShouldMoveWidget(false)
    , mUniboardAPI(nullptr)
{
    mWebEngineView = new UBWebEngineView();
    setWidget(mWebEngineView);

    setData(UBGraphicsItemData::ItemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly

    // create the page using a profile
    QWebEngineProfile* profile = UBApplication::webController->webProfile();
    mWebEngineView->setPage(new WebPage(profile, mWebEngineView));

    // see https://stackoverflow.com/questions/31928444/qt-qwebenginepagesetwebchannel-transport-object
    mWebChannel = new QWebChannel(this);
    mWebEngineView->page()->setWebChannel(mWebChannel);

    // NOTE to enable fullscreen, we would have to move the page to a fullscreen view.
    // webEngineView->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    setAcceptDrops(true);
    setAutoFillBackground(false);

    mWebEngineView->setAttribute(Qt::WA_TranslucentBackground);
    mWebEngineView->page()->setBackgroundColor(QColor(Qt::transparent));

    setDelegate(new UBGraphicsWidgetItemDelegate(this));

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);

    // workaround for QTBUG-108284 - to be removed when bug is fixed
    QWindow* window = mWebEngineView->windowHandle();

    if (window)
    {
        window->installEventFilter(this);
    }
}

UBGraphicsWidgetItem::~UBGraphicsWidgetItem()
{
    // get ownership back and delete widget
    setWidget(nullptr);
    delete mWebEngineView;
}

void UBGraphicsWidgetItem::initialize()
{
    setMinimumSize(nominalSize());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); // Necessary to set if we want z value to be assigned correctly

    if (Delegate() && Delegate()->frame() && resizable())
        Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

    // inject the QWebChannel interface and initialization script
    // see https://doc.qt.io/qt-5.12/qtwebengine-overview.html#script-injection to do that with WebEngine
    // https://doc.qt.io/qt-5.12/qwebengineprofile.html#scripts
    UBWebController::injectScripts(mWebEngineView);

    connect(mWebEngineView->page(), SIGNAL(geometryChangeRequested(QRect)), this, SLOT(geometryChangeRequested(QRect)));
    connect(mWebEngineView, SIGNAL(loadFinished(bool)), this, SLOT(mainFrameLoadFinished(bool)));
}

QUrl UBGraphicsWidgetItem::mainHtml() const
{
    return mMainHtmlUrl;
}

void UBGraphicsWidgetItem::loadMainHtml()
{
    qDebug() << "load main HTML";
    mInitialLoadDone = false;
    mWebEngineView->load(mMainHtmlUrl);
}

void UBGraphicsWidgetItem::load(QUrl url)
{
    mWebEngineView->load(url);
}

QUrl UBGraphicsWidgetItem::widgetUrl() const
{
    return mWidgetUrl;
}

QString UBGraphicsWidgetItem::mainHtmlFileName() const
{
    return mMainHtmlFileName;
}

bool UBGraphicsWidgetItem::canBeContent() const
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

bool UBGraphicsWidgetItem::canBeTool() const
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

void UBGraphicsWidgetItem::setCanBeTool(bool tool)
{
    mCanBeTool = tool;
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

void UBGraphicsWidgetItem::runScript(const QString &script)
{
    if (mWebEngineView->page())
        mWebEngineView->page()->runJavaScript(script);
}

void UBGraphicsWidgetItem::removeScript()
{
}

bool UBGraphicsWidgetItem::processDropEvent(QGraphicsSceneDragDropEvent *event)
{
    return mUniboardAPI->ProcessDropEvent(event);
}

bool UBGraphicsWidgetItem::isDropableData(const QMimeData *data) const
{
    return mUniboardAPI->isDropableData(data);
}

QUrl UBGraphicsWidgetItem::getOwnFolder() const
{
    return mOwnFolder;
}

void UBGraphicsWidgetItem::setOwnFolder(const QUrl &newFolder)
{
    mOwnFolder = newFolder;
}

void UBGraphicsWidgetItem::setSnapshotPath(const QUrl &newFilePath)
{
    mSnapshotFile = newFilePath;
}

QUrl UBGraphicsWidgetItem::getSnapshotPath() const
{
    return mSnapshotFile;
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

bool UBGraphicsWidgetItem::freezable() const
{
    return mIsFreezable;
}

bool UBGraphicsWidgetItem::resizable() const
{
    return mIsResizable;
}

bool UBGraphicsWidgetItem::isFrozen() const
{
    return mIsFrozen;
}

void UBGraphicsWidgetItem::setFreezable(bool freezable)
{
    mIsFreezable = freezable;
}

bool UBGraphicsWidgetItem::isWebActive() const
{
    return mIsWebActive;
}

const QPixmap &UBGraphicsWidgetItem::snapshot() const
{
    return mSnapshot;
}

const QPixmap &UBGraphicsWidgetItem::takeSnapshot()
{
    QPixmap pixmap(size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    mWebEngineView->render(&painter);

    mSnapshot = pixmap;

    return mSnapshot;
}

void UBGraphicsWidgetItem::saveSnapshot() const
{
    if (mSnapshotFile.isLocalFile() && !mSnapshot.isNull())
    {
        mSnapshot.save(mSnapshotFile.toLocalFile());
    }
}

void UBGraphicsWidgetItem::updatePosition()
{
    // partial workaround for QTBUG-109068 to forward the position of the item
    // on the scene to the QWebEngineView
    QSize actualSize = size().toSize();
    mWebEngineView->resize(actualSize - QSize(1,1));
    mWebEngineView->resize(actualSize);
}

void UBGraphicsWidgetItem::setSnapshot(const QPixmap& pix, bool frozen)
{
    mSnapshot = pix;
    mIsFrozen = frozen;
}

std::shared_ptr<UBGraphicsScene> UBGraphicsWidgetItem::scene()
{
    auto scenePtr = dynamic_cast<UBGraphicsScene*>(QGraphicsItem::scene());
    return scenePtr ? scenePtr->shared_from_this() : nullptr;
}

int UBGraphicsWidgetItem::widgetType(const QUrl& pUrl)
{
    QString mime = UBFileSystemUtils::mimeTypeFromFileName(pUrl.toString());

    if (mime == "application/vnd.apple-widget") // NOTE @letsfindaway obsolete
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

void UBGraphicsWidgetItem::initAPI()
{
    registerAPI();
}

void UBGraphicsWidgetItem::freeze()
{
    takeSnapshot();
    mIsFrozen = true;
}

void UBGraphicsWidgetItem::unFreeze()
{
    mIsFrozen = false;
}

void UBGraphicsWidgetItem::setWebActive(bool active)
{
    if (active != mIsWebActive)
    {
        if (active)
        {
            // activate the web engine view
            setWidget(mWebEngineView);
            setVisible(true);
        }
        else
        {
            // deactivate the web engine view
            setWidget(nullptr);
            mWebEngineView->setVisible(false);
        }

        mIsWebActive = active;
    }
}

void UBGraphicsWidgetItem::inspectPage()
{
    mWebEngineView->inspectPage();
}

void UBGraphicsWidgetItem::closeInspector()
{
    mWebEngineView->closeInspector();
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

    return QGraphicsProxyWidget::event(event);
}

void UBGraphicsWidgetItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (processDropEvent(event))
    {
        /*
         * NOTE: With QWebEngine, the drop event arriving at the widget is always
         * the original event, not the one modified by processDropEvent. The
         * dropData is therefore forwarded to the widget using a property of the
         * widget API.
         * Anyway we need to send the event to the QGraphicsProxyWidget, else it
         * would not arrive at the widget at all.
         * In order to be sure that the property change has arrived at the widget,
         * we must delay sending the drop event, else it could arrive at the widget
         * before the dropData Property is set.
         * And to send the delayed event, we have to copy it, as the original event
         * will be deleted immediately after returning from this method.
         */

        // delay delivery of dropEvent to allow property to propagate to widget
        // create a copy as the original event will be deleted
        QGraphicsSceneDragDropEvent* copy = new QGraphicsSceneDragDropEvent();
        copy->setPos(event->pos());
        copy->setSource(event->source());
        copy->setButtons(event->buttons());
        copy->setMimeData(event->mimeData());
        copy->setScenePos(event->scenePos());
        copy->setModifiers(event->modifiers());
        copy->setScreenPos(event->screenPos());
        copy->setDropAction(event->dropAction());
        copy->setProposedAction(event->proposedAction());
        copy->setPossibleActions(event->possibleActions());
        copy->setWidget(event->widget());

        // deliver the event after 100ms
        QTimer::singleShot(100, this, [this,copy](){
            QGraphicsProxyWidget::dropEvent(copy);
            delete copy;
        });
    }
    else
    {
        QGraphicsProxyWidget::dropEvent(event);
    }
}

#ifndef Q_OS_WIN
/*
 * workaround for a bug related to (at least) QTBUG-79216 - to be removed when bug is fixed
*/
QString UBGraphicsWidgetItem::getAccentedLetter(Qt::Key deadKey, const QString &letter) const
{
    // suppress blank after dead key
    const QString combined = sDeadKeys.value(deadKey, "") + (letter == " " ? "" : letter);

    return sAccentedCharacters.value(combined, combined);
}
#endif

void UBGraphicsWidgetItem::keyPressEvent(QKeyEvent *event)
{
#ifndef Q_OS_WIN
    const bool isDeadKey = event->key() >= Qt::Key_Dead_Grave && event->key() <= Qt::Key_Dead_Longsolidusoverlay;

    if (isDeadKey)
    {
        if (mLastDeadKey != Qt::Key_unknown)
        {
            //two dead keys in a row
            // - same dead keys: write accent
            // - different dead keys: drop both
            if (mLastDeadKey == event->key())
            {
                QKeyEvent currentDeadKeyEvent(
                    QEvent::KeyPress,
                    event->key(),
                    event->modifiers(),
                    sDeadKeys.value((Qt::Key)event->key(), "")
                );

                QGraphicsProxyWidget::keyPressEvent(&currentDeadKeyEvent);
            }

            mLastDeadKey = Qt::Key_unknown;
        }
        else
        {
            mLastDeadKey = (Qt::Key)event->key();
        }
    }
    else if (mLastDeadKey != Qt::Key_unknown && !event->text().isEmpty())
    {
        const QString accentedText = getAccentedLetter(mLastDeadKey, event->text());

        if (!accentedText.isEmpty())
        {
            QKeyEvent accentedLetterEvent(
                QEvent::KeyPress,
                event->key(),
                event->modifiers(),
                accentedText
            );

            QGraphicsProxyWidget::keyPressEvent(&accentedLetterEvent);
        }

        mLastDeadKey = Qt::Key_unknown;
    }
    else
    {
        QGraphicsProxyWidget::keyPressEvent(event);
    }
#else
    QGraphicsProxyWidget::keyPressEvent(event);
#endif
}

void UBGraphicsWidgetItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!Delegate()->mousePressEvent(event))
        setSelected(true); /* forcing selection */

    QGraphicsProxyWidget::mousePressEvent(event);

    // did webengine consume the mouse press ?
    mShouldMoveWidget = !event->isAccepted() && (event->buttons() & Qt::LeftButton);

    mLastMousePos = mapToScene(event->pos());

    event->accept();
}

void UBGraphicsWidgetItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mShouldMoveWidget = false;

    Delegate()->mouseReleaseEvent(event);
    QGraphicsProxyWidget::mouseReleaseEvent(event);
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
}

void UBGraphicsWidgetItem::sendJSLeaveEvent()
{
}

void UBGraphicsWidgetItem::injectInlineJavaScript()
{
    if (!sInlineJavaScriptLoaded) {
        sInlineJavaScripts = UBApplication::applicationController->widgetInlineJavaScripts();
        sInlineJavaScriptLoaded = true;
    }

    foreach(QString script, sInlineJavaScripts)
        mWebEngineView->page()->runJavaScript(script);
}

void UBGraphicsWidgetItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (isFrozen() || !isWebActive())
    {
        painter->drawPixmap(0, 0, snapshot());
    }
    else if (mInitialLoadDone)
    {
        QGraphicsProxyWidget::paint(painter, option, widget);
    }
    else
    {
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

bool UBGraphicsWidgetItem::eventFilter(QObject *obj, QEvent *ev)
{
    // workaround for QTBUG-108284 - to be removed when bug is fixed
    // forward CursorChange events to QGraphicsProxyWidget
    if (ev->type() == QEvent::CursorChange)
    {
        QWindow* window = dynamic_cast<QWindow*>(obj);

        if (window)
        {
            setCursor(window->cursor());
        }
    }

    return QGraphicsProxyWidget::eventFilter(obj, ev);
}

void UBGraphicsWidgetItem::geometryChangeRequested(const QRect& geom)
{
    resize(geom.width(), geom.height());
}

void UBGraphicsWidgetItem::registerAPI()
{
    injectInlineJavaScript();

    if (!mUniboardAPI)
    {
        mUniboardAPI = new UBWidgetUniboardAPI(scene(), this);
        mWebEngineView->page()->webChannel()->registerObject("sankore", mUniboardAPI);
    }
    else
    {
        mUniboardAPI->setScene(scene());
    }
}

void UBGraphicsWidgetItem::mainFrameLoadFinished (bool ok)
{
    mInitialLoadDone = true;
    mLoadIsErronous = !ok;

#ifdef Q_OS_MACOS
    //partial workaround for https://bugreports.qt.io/browse/QTBUG-118136 on MacOS
    runScript("el_qtbug_118136 = document.querySelector('input[type=text]'); if (el_qtbug_118136) el_qtbug_118136.focus()");
#endif

    // repaint when initial rendering is done
    update();
    updatePosition();
}

void UBGraphicsWidgetItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (Delegate()->wheelEvent(event))
    {
        QGraphicsProxyWidget::wheelEvent(event);
        event->accept();
    }
}

QVariant UBGraphicsWidgetItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == QGraphicsItem::ItemSelectedHasChanged) &&  scene()) {
        if (isSelected())
            scene()->setActiveWindow(this);
        else if (scene()->activeWindow() == this)
            scene()->setActiveWindow(nullptr);
    } else if (change == QGraphicsItem::ItemTransformHasChanged) {
        updatePosition();
    }

    QVariant newValue = Delegate()->itemChange(change, value);
    return QGraphicsProxyWidget::itemChange(change, newValue);
}

void UBGraphicsWidgetItem::resize(qreal w, qreal h)
{
    UBGraphicsWidgetItem::resize(QSizeF(w, h));
}


void UBGraphicsWidgetItem::resize(const QSizeF & pSize)
{
    if (pSize != size()) {
        mWebEngineView->setMaximumSize(pSize.width(), pSize.height());
        mWebEngineView->resize(pSize.width(), pSize.height());
        if (Delegate())
            Delegate()->positionHandles();
        if (scene())
            scene()->setModified(true);
    }
}

QSizeF UBGraphicsWidgetItem::size() const
{
    return mWebEngineView->size();
}


// NOTE @letsfindaway obsolete
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

    mWebEngineView->load(mMainHtmlUrl);

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
    UBGraphicsAppleWidgetItem *appleWidget = new UBGraphicsAppleWidgetItem(mWebEngineView->url(), parentItem());

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

    if (!f.exists())
        mMainHtmlUrl = QUrl(mMainHtmlFileName);

    mWebEngineView->load(mMainHtmlUrl);

    mNominalSize = QSize(width, height);
    setMaximumSize(mNominalSize);


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

QMap<QString, UBGraphicsW3CWidgetItem::PreferenceValue> UBGraphicsW3CWidgetItem::preferences() const
{
    return mPreferences;
}

UBGraphicsW3CWidgetItem::Metadata UBGraphicsW3CWidgetItem::metadatas() const
{
    return mMetadatas;
}

void UBGraphicsW3CWidgetItem::removeScript()
{
    if (mW3CWidgetAPI)
    {
        emit mW3CWidgetAPI->onremove();
    }
}

void UBGraphicsW3CWidgetItem::sendJSEnterEvent()
{
    const int tool = UBDrawingController::drawingController()->stylusTool();
    const bool isPointing = tool == UBStylusTool::Selector || tool == UBStylusTool::Play;

    if (mW3CWidgetAPI && isPointing)
    {
        emit mW3CWidgetAPI->onenter();
    }
}

void UBGraphicsW3CWidgetItem::sendJSLeaveEvent()
{
    if (mW3CWidgetAPI)
    {
        emit mW3CWidgetAPI->onleave();
    }
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        outConfig.setCodec("UTF-8");
#endif

        outConfig << configTemplate;
        configFile.close();

        QFile indexFile(widgetLibraryPath + "/index.htm");

        if (!indexFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open file " << indexFile.fileName();
            return QString();
        }

        QTextStream outIndex(&indexFile);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        outIndex.setCodec("UTF-8");
#endif

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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    outConfig.setCodec("UTF-8");
#endif
    outConfig << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << '\n';
    outConfig << "<widget xmlns=\"http://www.w3.org/ns/widgets\"" << '\n';
    outConfig << "    xmlns:ub=\"http://uniboard.mnemis.com/widgets\"" << '\n';
    outConfig << "    id=\"http://uniboard.mnemis.com/" << pName << "\"" <<'\n';

    outConfig << "    version=\"2.0\"" << '\n';
    outConfig << "    width=\"" << sizeHint.width() << "\"" << '\n';
    outConfig << "    height=\"" << sizeHint.height() << "\"" << '\n';
    outConfig << "    ub:resizable=\"true\">" << '\n';

    outConfig << "  <name>" << pName << "</name>" << '\n';
    outConfig << "  <content src=\"" << pName << ".html\"/>" << '\n';

    outConfig << "</widget>" << '\n';

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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    outStartFile.setCodec("UTF-8");
#endif

    outStartFile << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">" << '\n';
    outStartFile << "<html>" << '\n';
    outStartFile << "<head>" << '\n';
    outStartFile << "    <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">" << '\n';
    outStartFile << "</head>" << '\n';
    outStartFile << "  <body>" << '\n';
    outStartFile << html << '\n';
    outStartFile << "  </body>" << '\n';
    outStartFile << "</html>" << '\n';

    widgetHtmlFile.close();

    return widgetPath;
}

bool UBGraphicsW3CWidgetItem::hasNPAPIWrapper(const QString& pMimeType)
{
    loadNPAPIWrappersTemplates();

    return sNPAPIWrapperTemplates.contains(pMimeType);
}

void UBGraphicsW3CWidgetItem::registerAPI()
{
    UBGraphicsWidgetItem::registerAPI();

    if (!mW3CWidgetAPI)
    {
        mW3CWidgetAPI = new UBW3CWidgetAPI(this);
        mWebEngineView->page()->webChannel()->registerObject("widget", mW3CWidgetAPI);
    }
}

void UBGraphicsW3CWidgetItem::loadNPAPIWrappersTemplates()
{
    if (!sTemplateLoaded) {
        sNPAPIWrapperTemplates.clear();

        QString templatePath = UBPlatformUtils::applicationTemplateDirectory();

        QDir templateDir(templatePath);

        foreach(QString fileName, templateDir.entryList()) {
            if (fileName.startsWith("npapi-wrapper") && (fileName.endsWith(".htm") || fileName.endsWith(".html"))) {

                QString htmlContent = UBFileSystemUtils::readTextFile(templatePath + fileName);

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
        sNPAPIWrappperConfigTemplate = UBFileSystemUtils::readTextFile(templatePath + "npapi-wrapper.config.xml");
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

            if (lang == configLang || (configLang.length() == 2 && configLang == lang.left(2)))
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
        cp->setData(UBGraphicsItemData::ItemIsHiddenOnDisplay, this->data(UBGraphicsItemData::ItemIsHiddenOnDisplay));
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
        cp->setSnapshot(this->snapshot(), this->isFrozen());
    }
}

