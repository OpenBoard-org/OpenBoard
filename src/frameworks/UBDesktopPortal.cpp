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

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
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

const QDBusArgument &operator >> (const QDBusArgument &arg, UBDesktopPortal::Stream &stream)
{
    arg.beginStructure();
    arg >> stream.node_id;

    arg.beginMap();

    while (!arg.atEnd())
    {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        arg.endMapEntry();
        stream.map.insert(key, value);
    }

    arg.endMap();
    arg.endStructure();

    return arg;
}

UBDesktopPortal::UBDesktopPortal(QObject* parent)
    : QObject{parent}
{
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

    if (!rect.isNull())
    {
        mScreenRect = rect.translated(mScreenRect.topLeft());
    }

    QDBusInterface screenshotPortal("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Screenshot");

    if (screenshotPortal.isValid())
    {
        QMap<QString, QVariant> options;
        options["handle_token"] = createRequestToken();
        QDBusReply<QDBusObjectPath> reply = screenshotPortal.call("Screenshot", "", options);
        QDBusObjectPath objectPath = reply.value();
        QString path = objectPath.path();
        QDBusConnection::sessionBus().connect(
                            "",
                            path,
                            "org.freedesktop.portal.Request",
                            "Response",
                            "ua{sv}",
                            this,
                            SLOT(handleScreenshotResponse(uint,QMap<QString,QVariant>)));
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
    QUrl uri(results["uri"].toUrl());
    QFile file(uri.toLocalFile());

    if (!file.exists())
    {
        qDebug() << "Screenshot image file does not exist";
        emit screenGrabbed(QPixmap{});
        return;
    }

    QPixmap pixmap{file.fileName()};
    file.remove();

    // cut requested screen
    QPixmap screenshot = pixmap.copy(mScreenRect);

    emit screenGrabbed(screenshot);
}

void UBDesktopPortal::handleCreateSessionResponse(uint response, const QVariantMap& results)
{
    if (response != 0)
    {
        qWarning() << "Failed to create session: " << response << results;
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
    options["persist_mode"] = PERSISTENT;   // restore token valid across application restart

    auto restoreToken = UBSettings::settings()->value("App/ScreenCastRestoreToken");

    if (restoreToken.isValid())
    {
        options["restore_token"] = restoreToken;
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
    Q_UNUSED(results);

    // Show annotation drawing view in desktop mode after portal dialog was closed
    showGlassPane(true);

    if (response != 0)
    {
        // The system Desktop dialog was canceled
        qDebug() << "Failed to start or cancel dialog: " << response;
        emit screenCastAborted();
        return;
    }

    // save restore token
    UBSettings::settings()->setValue("App/ScreenCastRestoreToken", results.value("restore_token"));

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

        mRequestPath = "/org/freedesktop/portal/desktop/request/" + mScreencastPortal->connection().baseService().remove(0, 1).replace('.', '_') + "/";
        qDebug() << "request path" << mRequestPath;
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
