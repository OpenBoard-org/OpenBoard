/*
 * Copyright (C) 2015-2025 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBDesktopPortal.h"

#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusVariant>
#include <QGuiApplication>
#include <QClipboard>
#include <QTimer>
#include <QMetaType>
#include <QDebug>

#include "board/UBBoardView.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "desktop/UBDesktopAnnotationController.h"
#include "frameworks/UBPlatformUtils.h"


enum : uint { MONITOR = 1, WINDOW = 2, VIRTUAL = 4 } SourceType;
enum : uint { HIDDEN = 1, EMBEDDED = 2, METADATA = 4 } CursorMode;
enum : uint { TRANSIENT = 0, APPLICATION = 1, PERSISTENT = 2 } PersistMode;

Q_DECLARE_METATYPE(UBDesktopPortal::Stream)
Q_DECLARE_METATYPE(UBDesktopPortal::Streams)

QPixmap UBDesktopPortal::loadScreenshotFromUri(const QUrl& uri) const
{
    if (uri.isEmpty())
    {
        return {};
    }

    QFile file(uri.toLocalFile());

    if (!file.exists())
    {
        qDebug() << "Screenshot image file does not exist" << uri;
        return {};
    }

    QPixmap pixmap{file.fileName()};
    file.remove();
    qDebug() << "DesktopPortal: loaded screenshot file" << uri;
    return pixmap;
}

QPixmap UBDesktopPortal::readClipboardScreenshot() const
{
    auto clipboard = QGuiApplication::clipboard();

    if (!clipboard)
    {
        qDebug() << "DesktopPortal: no clipboard available";
        return {};
    }

    QImage cbImage = clipboard->image();

    if (!cbImage.isNull())
    {
        QPixmap fromImage = QPixmap::fromImage(cbImage);
        qDebug() << "DesktopPortal: clipboard image found" << "size" << fromImage.size();
        return fromImage;
    }

    QPixmap cbPixmap = clipboard->pixmap();

    if (!cbPixmap.isNull())
    {
        qDebug() << "DesktopPortal: clipboard pixmap found" << "size" << cbPixmap.size();
        return cbPixmap;
    }

    qDebug() << "DesktopPortal: clipboard empty";
    return {};
}


QDBusArgument &operator << (QDBusArgument &arg, const UBDesktopPortal::Stream &stream)
{
    arg.beginStructure();
    arg << stream.node_id;

    arg.beginMap(qMetaTypeId<QString>(), qMetaTypeId<QDBusVariant>());
    for (auto it = stream.map.cbegin(); it != stream.map.cend(); ++it)
    {
        arg.beginMapEntry();
        arg << it.key() << QDBusVariant(it.value());
        arg.endMapEntry();
    }
    arg.endMap();

    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >> (const QDBusArgument &arg, UBDesktopPortal::Stream &stream)
{
    arg.beginStructure();
    arg >> stream.node_id;

    arg.beginMap();

    while (!arg.atEnd())
    {
        QString key;
        QDBusVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        arg.endMapEntry();
        stream.map.insert(key, value.variant());
    }

    arg.endMap();
    arg.endStructure();

    return arg;
}

QDBusArgument &operator << (QDBusArgument &arg, const UBDesktopPortal::Streams &streams)
{
    arg.beginArray(qMetaTypeId<UBDesktopPortal::Stream>());
    for (const auto &stream : streams)
    {
        arg << stream;
    }
    arg.endArray();
    return arg;
}


const QDBusArgument &operator >> (const QDBusArgument &arg, UBDesktopPortal::Streams &streams)
{
    streams.clear();
    arg.beginArray();
    while (!arg.atEnd())
    {
        UBDesktopPortal::Stream stream;
        arg >> stream;
        streams.append(stream);
    }
    arg.endArray();
    return arg;
}

UBDesktopPortal::UBDesktopPortal(QObject* parent)
    : QObject{parent}
{
    qDBusRegisterMetaType<Stream>();
    qDBusRegisterMetaType<Streams>();
}

UBDesktopPortal::~UBDesktopPortal()
{
    if (mScreencastPortal)
    {
        delete mScreencastPortal;
    }
}

void UBDesktopPortal::grabScreen(QScreen* screen, const QRect& rect)
{
    mScreenRect = screen->geometry();
    mInteractiveScreenshot = false;

    if (!rect.isNull())
    {
        mScreenRect = rect.translated(mScreenRect.topLeft());
    }

    qDebug() << "DesktopPortal: grabScreen"
             << "screen" << (screen ? screen->name() : QStringLiteral("<null>"))
             << "geom" << mScreenRect
             << "rect" << rect;

    QDBusInterface screenshotPortal("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Screenshot");

    if (screenshotPortal.isValid())
    {
        QMap<QString, QVariant> options;
        options["handle_token"] = createRequestToken();
        options["interactive"] = true; // required by some portals (e.g. GNOME)
        mInteractiveScreenshot = true;
        QDBusReply<QDBusObjectPath> reply = screenshotPortal.call("Screenshot", "", options);
        QDBusObjectPath objectPath = reply.value();
        QString path = objectPath.path();
        qDebug() << "DesktopPortal: Screenshot reply valid" << reply.isValid() << "path" << path << "options" << options;

        if (!reply.isValid())
        {
            qWarning() << "DesktopPortal: Screenshot call failed" << reply.error();
            emit screenGrabbed(QPixmap{});
            return;
        }

        const bool connected = QDBusConnection::sessionBus().connect(
                            "",
                            path,
                            "org.freedesktop.portal.Request",
                            "Response",
                            "ua{sv}",
                            this,
                            SLOT(handleScreenshotResponse(uint,QMap<QString,QVariant>)));

        if (!connected)
        {
            qWarning() << "DesktopPortal: failed to connect to screenshot response signal";
        }
    }
    else
    {
        qDebug() << "No valid screenshot portal";
        emit screenGrabbed(QPixmap{});
    }
}

void UBDesktopPortal::startScreenCast(bool withCursor)
{
    mWithCursor = withCursor;

    QDBusInterface* portal = screencastPortal();

    if (!portal)
    {
        return;
    }

    // Create ScreenCast session
    QString requestToken = createRequestToken();
    QMap<QString, QVariant> options;
    options["session_handle_token"] = createSessionToken();
    options["handle_token"] = requestToken;

    QDBusConnection::sessionBus().connect("", mRequestPath + requestToken, "org.freedesktop.portal.Request", "Response", "ua{sv}", this,
                                          SLOT(handleCreateSessionResponse(uint,QMap<QString,QVariant>)));

    const QDBusReply<QDBusObjectPath> reply = portal->call("CreateSession", options);

    if (!reply.isValid())
    {
        qWarning() << "Couldn't get reply to ScreenCast/CreateSession";
        qWarning() << "Error: " << reply.error().message();
        emit screenCastAborted();
        return;
    }
}

void UBDesktopPortal::stopScreenCast()
{
    if (mSession.isEmpty())
    {
        return;
    }

    QDBusInterface portal("org.freedesktop.portal.Desktop", mSession, "org.freedesktop.portal.Session");

    if (portal.isValid())
    {
        const QDBusReply<void> reply = portal.call("Close");

        if (!reply.isValid())
        {
            qWarning() << "Couldn't get reply to ScreenCast/Close";
            qWarning() << "Error: " << reply.error().message();
        }
    }

    mSession.clear();
}

void UBDesktopPortal::handleScreenshotResponse(uint code, const QMap<QString, QVariant>& results)
{
    qDebug() << "DesktopPortal: handleScreenshotResponse code" << code << "results" << results;

    QPixmap pixmap;
    auto finalizeScreenshot = [this](const QPixmap& pixmap){
        if (pixmap.isNull())
        {
            qDebug() << "DesktopPortal: screenshot pixmap still null, aborting";
            emit screenGrabbed(QPixmap{});
            return;
        }

        QPixmap screenshot;

        if (mInteractiveScreenshot)
        {
            // portal already applied the selection
            screenshot = pixmap;
        }
        else
        {
            QRect targetRect = mScreenRect;
            targetRect.moveTo(0, 0); // portal images are local to the grab
            targetRect &= pixmap.rect();
            screenshot = pixmap.copy(targetRect);
            qDebug() << "DesktopPortal: cropping screenshot" << "targetRect" << targetRect;
        }

        qDebug() << "DesktopPortal: emitting screenshot"
                 << "interactive" << mInteractiveScreenshot
                 << "source size" << pixmap.size()
                 << "final size" << screenshot.size();
        emit screenGrabbed(screenshot);
    };

    const QUrl uri(results.value("uri").toUrl());
    qDebug() << "DesktopPortal: screenshot URI" << uri;

    pixmap = loadScreenshotFromUri(uri);

    // GNOME may only place the shot on the clipboard when capturing a full screen
    if (pixmap.isNull())
    {
        pixmap = readClipboardScreenshot();
    }

    if (pixmap.isNull())
    {
        // Clipboard might not be populated yet on some portals (e.g. GNOME fullscreen); retry shortly.
        QTimer::singleShot(500, this, [this, finalizeScreenshot](){
            QPixmap delayed = readClipboardScreenshot();
            finalizeScreenshot(delayed);
        });
        return;
    }

    finalizeScreenshot(pixmap);
}

void UBDesktopPortal::handleCreateSessionResponse(uint response, const QVariantMap& results)
{
    if (response != 0)
    {
        qWarning() << "Failed to create session: " << response << results;
        mSession.clear();
        emit screenCastAborted();
        return;
    }

    mSession = results.value("session_handle").toString();

    QDBusInterface* portal = screencastPortal();

    if (!portal)
    {
        return;
    }

    // Select sources
    QString requestToken = createRequestToken();
    QMap<QString, QVariant> options;
    options["multiple"] = false;
    options["types"] = MONITOR;
    options["cursor_mode"] = mWithCursor ? EMBEDDED : HIDDEN;
    options["handle_token"] = requestToken;

    if (mSupportsPersistentScreencast)
    {
        options["persist_mode"] = PERSISTENT;   // restore token valid across application restart

        auto restoreToken = UBSettings::settings()->value("App/ScreenCastRestoreToken");

        if (restoreToken.isValid())
        {
            options["restore_token"] = restoreToken;
        }
    }

    // connect before call
    QDBusConnection::sessionBus().connect("", mRequestPath + requestToken, "org.freedesktop.portal.Request", "Response", "ua{sv}", this,
                                          SLOT(handleSelectSourcesResponse(uint,QMap<QString,QVariant>)));

    const QDBusReply<QDBusObjectPath> reply = portal->call("SelectSources", QDBusObjectPath(mSession), options);

    if (!reply.isValid())
    {
        qWarning() << "Couldn't get reply to ScreenCast/SelectSources";
        qWarning() << "Error: " << reply.error().message();
        emit screenCastAborted();
        return;
    }
}

void UBDesktopPortal::handleSelectSourcesResponse(uint response, const QVariantMap& results)
{
    Q_UNUSED(results);

    if (response != 0)
    {
        qWarning() << "Failed to select sources: " << response;
        mSession.clear();
        emit screenCastAborted();
        return;
    }

    QDBusInterface* portal = screencastPortal();

    if (!portal)
    {
        return;
    }

    // Start ScreenCast
    QString requestToken = createRequestToken();
    QMap<QString, QVariant> options;
    options["handle_token"] = requestToken;

    QDBusConnection::sessionBus().connect("", mRequestPath + requestToken, "org.freedesktop.portal.Request", "Response", "ua{sv}", this,
                                          SLOT(handleStartResponse(uint,QMap<QString,QVariant>)));

    const QDBusReply<QDBusObjectPath> reply = portal->call("Start", QDBusObjectPath(mSession), "", options);

    if (!reply.isValid())
    {
        qWarning() << "Couldn't get reply to ScreenCast/Start";
        qWarning() << "Error: " << reply.error().message();
        emit screenCastAborted();
        return;
    }

    // Hide annotation drawing view in desktop mode so that portal dialog is topmost
    showGlassPane(false);
}

void UBDesktopPortal::handleStartResponse(uint response, const QVariantMap& results)
{
    // Show annotation drawing view in desktop mode after portal dialog was closed
    showGlassPane(true);

    if (response != 0)
    {
        // The system Desktop dialog was canceled
        qDebug() << "Failed to start or cancel dialog: " << response;
        mSession.clear();
        emit screenCastAborted();
        return;
    }

    // save restore token
    const QVariant restoreTokenVariant = results.value("restore_token");
    QString restoreToken;

    if (restoreTokenVariant.canConvert<QDBusVariant>())
    {
        restoreToken = restoreTokenVariant.value<QDBusVariant>().variant().toString();
    }
    else
    {
        restoreToken = restoreTokenVariant.toString();
    }

    UBSettings::settings()->setValue("App/ScreenCastRestoreToken", restoreToken);

    // invalidate token when screen configuration changes
    static bool connected{false};

    if (!connected)
    {
        connected = true;
        // don't use the four argument version of connect as this will be disconnected
        // when the context object is destroyed
        connect(UBSettings::settings()->appScreenList, &UBSetting::changed, [](){
            UBSettings::settings()->setValue("App/ScreenCastRestoreToken", {});
        });
    }

    const Streams streams = qdbus_cast<Streams>(results.value("streams"));
    const Stream stream = streams.last();

    QDBusInterface* portal = screencastPortal();

    if (!portal)
    {
        return;
    }

    // Open PipeWire Remote
    QMap<QString, QVariant> options;
    const QDBusReply<QDBusUnixFileDescriptor> reply = portal->call("OpenPipeWireRemote", QDBusObjectPath(mSession), options);

    if (!reply.isValid())
    {
        qWarning() << "Couldn't get reply to ScreenCast/OpenPipeWireRemote";
        qWarning() << "Error: " << reply.error().message();
        emit screenCastAborted();
        return;
    }

    const int fd = reply.value().fileDescriptor();
    emit streamStarted(fd, stream.node_id);
}

QDBusInterface* UBDesktopPortal::screencastPortal()
{
    if (!mScreencastPortal)
    {
        mScreencastPortal = new QDBusInterface("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
                                               "org.freedesktop.portal.ScreenCast");
        mScreencastPortal->setParent(this);

        QString baseService = QDBusConnection::sessionBus().baseService();
        baseService.remove(0, 1);
        baseService.replace('.', '_');
        mRequestPath = "/org/freedesktop/portal/desktop/request/" + baseService + "/";

        const QVariant version = mScreencastPortal->property("version");
        mScreencastPortalVersion = version.isValid() ? version.toUInt() : 0;
        mSupportsPersistentScreencast = mScreencastPortalVersion >= 4;

        qDebug() << "request path" << mRequestPath << "portal version" << mScreencastPortalVersion
                 << "persistent support" << mSupportsPersistentScreencast;
    }

    if (mScreencastPortal->isValid())
    {
        return mScreencastPortal;
    }

    emit screenCastAborted();
    return nullptr;
}

QString UBDesktopPortal::createSessionToken() const
{
    static int sessionTokenCounter = 0;

    sessionTokenCounter += 1;
    return QString("obsess%1").arg(sessionTokenCounter);
}

QString UBDesktopPortal::createRequestToken() const
{
    static int requestTokenCounter = 0;

    requestTokenCounter += 1;
    return QString("obreq%1").arg(requestTokenCounter);
}

void UBDesktopPortal::showGlassPane(bool show) const
{
    if (UBApplication::applicationController->isShowingDesktop())
    {
        UBApplication::applicationController->uninotesController()->drawingView()->setVisible(show);

        if (show)
        {
            UBPlatformUtils::keepOnTop();
        }
    }
}
