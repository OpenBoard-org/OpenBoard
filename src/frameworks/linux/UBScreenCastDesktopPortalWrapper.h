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


#pragma once

#include <QObject>
#include <QVariantMap>
#include <QPixmap>

// forward
class QDBusInterface;

class UBScreenCastDesktopPortalWrapper : public QObject
{
    Q_OBJECT

public:
    typedef struct
    {
        uint node_id;
        QVariantMap map;
    } Stream;
    typedef QList<Stream> Streams;

    explicit UBScreenCastDesktopPortalWrapper(QObject* parent = nullptr);
    virtual ~UBScreenCastDesktopPortalWrapper();

public slots:
    void startScreenCast(bool withCursor);
    void stopScreenCast();

signals:
    void streamStarted(int fd, int nodeId);
    void screenCastAborted();
    void showGlassPane(bool show);

private slots:
    void handleCreateSessionResponse(uint response, const QVariantMap& results);
    void handleSelectSourcesResponse(uint response, const QVariantMap& results);
    void handleStartResponse(uint response, const QVariantMap& results);

private:
    QDBusInterface* screencastPortal();

private:
    bool mWithCursor{false};
    QString mSession;
    QString mRequestPath;
    uint mScreencastPortalVersion{0};
    bool mSupportsPersistentScreencast{false};
    QDBusInterface* mScreencastPortal{nullptr};
};
