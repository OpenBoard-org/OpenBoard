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


#pragma once

#include <QObject>

#include <pipewire/pipewire.h>
#include <spa/param/video/format.h>


class UBPipewireSink : public QObject
{
    Q_OBJECT

public:
    explicit UBPipewireSink(QObject* parent = nullptr);
    ~UBPipewireSink();

public slots:
    bool start(int fd, int nodeId);

signals:
    void gotImage(QImage image);
    void streamingInterrupted();

private:
    void process();
    void streamStateChanged(enum pw_stream_state old, enum pw_stream_state state, const char* error);
    void streamParamChanged(uint32_t id, const struct spa_pod* param);

private:
    pw_thread_loop* mLoop{nullptr};
    pw_context* mContext{nullptr};
    pw_stream* mStream{nullptr};
    struct spa_video_info mFormat{};
    struct spa_hook mHook{};
};

