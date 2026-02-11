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


#include "UBPipewireSink.h"

#include <QDebug>
#include <QImage>

#include <spa/debug/types.h>
#include <spa/param/format-utils.h>
#include <spa/param/video/raw-utils.h>
#include <spa/pod/builder.h>

#include <fcntl.h>


UBPipewireSink::UBPipewireSink(QObject* parent)
    : QObject{parent}
{
}

UBPipewireSink::~UBPipewireSink()
{
    qDebug() << "UBPipewireSink: stop threaded loop";
    pw_thread_loop_stop(mLoop);
    pw_thread_loop_destroy(mLoop);
}

bool UBPipewireSink::start(int fd, int nodeId)
{
    // stream event handler struct
    // NOTE unused entries added for compatibility with some compilers
    static const struct pw_stream_events streamEvents = {
        .version = PW_VERSION_STREAM_EVENTS,
        .destroy = nullptr,
        .state_changed = [](void* userdata, enum pw_stream_state old, enum pw_stream_state state, const char* error){
            static_cast<UBPipewireSink*>(userdata)->streamStateChanged(old, state, error);
        },
        .control_info = nullptr,
        .io_changed = nullptr,
        .param_changed = [](void* userdata, uint32_t id, const struct spa_pod* param){
            static_cast<UBPipewireSink*>(userdata)->streamParamChanged(id, param);
        },
        .add_buffer = nullptr,
        .remove_buffer = nullptr,
        .process = [](void* userdata){
            static_cast<UBPipewireSink*>(userdata)->process();
        },
        .drained = nullptr,
        .command = nullptr,
        .trigger_done = nullptr
    };

    qDebug() << "UBPipewireSink: start" << fd << nodeId;

    // Initialize PipeWire
    pw_init(nullptr, nullptr);

    mLoop = pw_thread_loop_new(nullptr, nullptr);

    if (!mLoop)
    {
        qWarning() << "UBPipewireSink: unable to create main loop";
        return false;
    }

    pw_thread_loop_lock(mLoop);
    pw_thread_loop_start(mLoop);

    auto loop = pw_thread_loop_get_loop(mLoop);

    mContext = pw_context_new(loop, nullptr, 0);

    if (!mContext)
    {
        qWarning() << "UBPipewireSink: unable to create context";
        pw_thread_loop_destroy(mLoop);
        return false;
    }

    // connect context, duplicate fd before use, see
    // https://chromium.googlesource.com/external/webrtc/+/master/modules/desktop_capture/linux/wayland/shared_screencast_stream.cc#457
    // and https://doc.qt.io/qt-6/qdbusunixfiledescriptor.html#fileDescriptor
    const auto fd2 = fcntl(fd, F_DUPFD_CLOEXEC, 0);
    auto core = pw_context_connect_fd(mContext, fd2, nullptr, 0);

    if (!core)
    {
        qWarning() << "Cannot connect" << errno;
        return false;
    }

    // Create stream
    struct pw_properties* props;
    props = pw_properties_new(
                PW_KEY_MEDIA_TYPE, "Video",
                PW_KEY_MEDIA_CATEGORY, "Capture",
                PW_KEY_MEDIA_ROLE, "Screen",
                nullptr);

    mStream = pw_stream_new(core, "capture", props);

    if (!mStream)
    {
        qWarning() << "UBPipewireSink: unable to create stream";
        pw_context_destroy(mContext);
        pw_thread_loop_destroy(mLoop);
        return false;
    }

    pw_stream_add_listener(mStream, &mHook, &streamEvents, this);

    // Set up stream parameters
    const struct spa_pod* params[1];
    uint8_t buffer[1024];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    const auto rDef = SPA_RECTANGLE(1024, 768);
    const auto rMin = SPA_RECTANGLE(1, 1);
    const auto rMax = SPA_RECTANGLE(4096, 4096);
    const auto fDef = SPA_FRACTION(25, 1);
    const auto fMin = SPA_FRACTION(0, 1);
    const auto fMax = SPA_FRACTION(1000, 1);

    auto p = spa_pod_builder_add_object(
        &b,
        SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
        SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
        SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
        SPA_FORMAT_VIDEO_format, SPA_POD_CHOICE_ENUM_Id(7,
            SPA_VIDEO_FORMAT_ARGB,
            SPA_VIDEO_FORMAT_xRGB,
            SPA_VIDEO_FORMAT_RGB,
            SPA_VIDEO_FORMAT_RGBA,
            SPA_VIDEO_FORMAT_RGBx,
            SPA_VIDEO_FORMAT_BGRA,
            SPA_VIDEO_FORMAT_BGRx),
        SPA_FORMAT_VIDEO_size, SPA_POD_CHOICE_RANGE_Rectangle(&rDef, &rMin, &rMax),
        SPA_FORMAT_VIDEO_framerate, SPA_POD_CHOICE_RANGE_Fraction(&fDef, &fMin, &fMax));

    params[0] = static_cast<struct spa_pod*>(p);

    // connect stream
    // NOTE connection by nodeId is deprecated. But portal only provides this.
    auto res =
        pw_stream_connect(mStream, PW_DIRECTION_INPUT, nodeId,
                          pw_stream_flags(PW_STREAM_FLAG_AUTOCONNECT | // try to automatically connect this stream
                                          PW_STREAM_FLAG_MAP_BUFFERS), // mmap the buffer data for us
                          params, 1);

    if (res < 0)
    {
        qWarning() << "UBPipewireSink: unable to connect stream" << res;
    }

    pw_thread_loop_unlock(mLoop);

    return true;
}

void UBPipewireSink::process()
{
    struct pw_buffer* b;
    struct spa_buffer* buf;

    if ((b = pw_stream_dequeue_buffer(mStream)) == nullptr)
    {
        pw_log_warn("out of buffers: %m");
        return;
    }

    buf = b->buffer;
    if (buf->datas[0].data == NULL)
        return;

    // create image from frame data
    auto data = static_cast<const uchar*>(buf->datas[0].data);
    auto width = static_cast<int>(mFormat.info.raw.size.width);
    auto height = static_cast<int>(mFormat.info.raw.size.height);
    auto stride = buf->datas[0].chunk->stride;

    QImage image;
    bool swapRgb{false};

    switch (mFormat.info.raw.format)
    {
    case SPA_VIDEO_FORMAT_ARGB:
        image = QImage(data, width, height, stride, QImage::Format_ARGB32);
        break;

    case SPA_VIDEO_FORMAT_xRGB:
        image = QImage(data, width, height, stride, QImage::Format_RGB32);
        break;

    case SPA_VIDEO_FORMAT_RGB:
        image = QImage(data, width, height, stride, QImage::Format_RGB888);
        break;

    case SPA_VIDEO_FORMAT_RGBA:
        image = QImage(data, width, height, stride, QImage::Format_RGBA8888);
        break;

    case SPA_VIDEO_FORMAT_RGBx:
        image = QImage(data, width, height, stride, QImage::Format_RGBX8888);
        break;

    case SPA_VIDEO_FORMAT_BGRA:
        image = QImage(data, width, height, stride, QImage::Format_RGBA8888);
        swapRgb = true;
        break;

    case SPA_VIDEO_FORMAT_BGRx:
        image = QImage(data, width, height, stride, QImage::Format_RGBX8888);
        swapRgb = true;
        break;

    default:
        qWarning() << "UBPipewireSink: unsupported image format";
    }

    if (swapRgb)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        image.rgbSwap();
#else
        image = image.rgbSwapped();
#endif
    }

    emit gotImage(image);

    pw_stream_queue_buffer(mStream, b);
}

void UBPipewireSink::streamStateChanged(pw_stream_state old, pw_stream_state state, const char* error)
{
    qDebug() << "UBPipewireSink: state changed" << state << error;

    if (old == PW_STREAM_STATE_STREAMING)
    {
        emit streamingInterrupted();
    }
}

void UBPipewireSink::streamParamChanged(uint32_t id, const spa_pod* param)
{
    if (param == NULL || id != SPA_PARAM_Format)
        return;

    if (spa_format_parse(param, &mFormat.media_type, &mFormat.media_subtype) < 0)
        return;

    if (mFormat.media_type != SPA_MEDIA_TYPE_video || mFormat.media_subtype != SPA_MEDIA_SUBTYPE_raw)
        return;

    if (spa_format_video_raw_parse(param, &mFormat.info.raw) < 0)
        return;

    qDebug() << "got video format:";
    qDebug() << "  format:" << mFormat.info.raw.format
             << spa_debug_type_find_name(spa_type_video_format, mFormat.info.raw.format);
    qDebug() << "  size:" << mFormat.info.raw.size.width << mFormat.info.raw.size.height;
    qDebug() << "  framerate:" << mFormat.info.raw.framerate.num << "/" << mFormat.info.raw.framerate.denom;
}
