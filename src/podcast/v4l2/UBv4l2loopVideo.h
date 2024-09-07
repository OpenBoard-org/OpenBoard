/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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

#ifndef UBV4L2LOOPVIDEO_H
#define UBV4L2LOOPVIDEO_H

#include <atomic>

#include <QtCore>
#include <QImage>

#include "podcast/UBAbstractVideoEncoder.h"

class UBFFmpegVideoEncoderWorker;
class UBPodcastController;

/**
 * This class provides an interface between the podcast controller and the ffmpeg
 * back-end.
 * It includes all the necessary objects and methods to record video (muxer, audio and
 * video streams and encoders, etc) from inputs consisting of raw PCM audio and raw RGBA
 * images.
 *
 */

#include <linux/videodev2.h>

class UBv4l2loopVideoEncoder : public UBAbstractVideoEncoder
{
    Q_OBJECT

public:
    UBv4l2loopVideoEncoder(QObject* parent = NULL);
    virtual ~UBv4l2loopVideoEncoder();

    bool start();
    bool stop();

    void newPixmap(const QImage& pImage, long timestamp);

    QString videoFileExtension() const { return "novideo"; }

    QString lastErrorMessage() { return mLastErrorMessage; }

    void setRecordAudio(bool pRecordAudio __attribute__((unused))) {  }

private slots:

    void setLastErrorMessage(const QString& pMessage);
    void onAudioAvailable(QByteArray data);
    void finishEncoding();

private:
    const uint32_t pixelformat = V4L2_PIX_FMT_BGR32;

    bool update_format(bool check=false);
    struct v4l2_format format;

    int fd;

    QString mLastErrorMessage;
};

#endif // UBV4L2LOOPVIDEO_H
