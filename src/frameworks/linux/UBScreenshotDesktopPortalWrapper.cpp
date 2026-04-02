/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBScreenshotDesktopPortalWrapper.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QGuiApplication>
#include <QClipboard>
#include <QFile>
#include <QScreen>
#include <QTimer>
#include <QDebug>

#include "frameworks/linux/UBDesktopPortalTokenGenerator.h"


UBScreenshotDesktopPortalWrapper::UBScreenshotDesktopPortalWrapper(QObject* parent)
    : QObject{parent}
{
}

UBScreenshotDesktopPortalWrapper::~UBScreenshotDesktopPortalWrapper()
{
}

void UBScreenshotDesktopPortalWrapper::grabScreen(QScreen* screen, bool interactive)
{
    mScreenRect = screen->geometry();
    mInteractiveScreenshot = interactive;

    qDebug() << "DesktopPortal: grabScreen"
             << "screen" << (screen ? screen->name() : QStringLiteral("<null>"))
             << "geom" << mScreenRect
             << "interactive" << interactive;

    QDBusInterface screenshotPortal("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Screenshot");

    if (screenshotPortal.isValid())
    {
        QMap<QString, QVariant> options;
        options["handle_token"] = UBDesktopPortalTokenGenerator::generateToken();
        options["interactive"] = mInteractiveScreenshot; // required by some portals (e.g. GNOME)
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

void UBScreenshotDesktopPortalWrapper::handleScreenshotResponse(uint code, const QVariantMap& results)
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
            // NOTE have to disable this again.
            // On KDE, the screenshot contains all screens, and I have to cut the desired screen out of this.
            // What happens for non-interactive screenshots on GNOME? Is this important at all?
//            targetRect.moveTo(0, 0); // portal images are local to the grab
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

QPixmap UBScreenshotDesktopPortalWrapper::loadScreenshotFromUri(const QUrl& uri) const
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

QPixmap UBScreenshotDesktopPortalWrapper::readClipboardScreenshot() const
{
    const auto clipboard = QGuiApplication::clipboard();

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
