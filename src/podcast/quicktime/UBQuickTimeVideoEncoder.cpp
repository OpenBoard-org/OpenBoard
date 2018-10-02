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




#include "UBQuickTimeVideoEncoder.h"

#include <QuickTime/QuickTime.h>

#include <QtGui>

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "UBQuickTimeFile.h"

#include "core/memcheck.h"

UBQuickTimeVideoEncoder::UBQuickTimeVideoEncoder(QObject* pParent)
    : UBAbstractVideoEncoder(pParent)
    , mQuickTimeCompressionSession(0)
    , mShouldRecordAudio(true)

{
    // NOOP
}


UBQuickTimeVideoEncoder::~UBQuickTimeVideoEncoder()
{
    // NOOP
}


bool UBQuickTimeVideoEncoder::start()
{
    QString quality = UBSettings::settings()->podcastQuickTimeQuality->get().toString();

    if(!mQuickTimeCompressionSession.init(videoFileName(), quality, framesPerSecond(), videoSize()
                , mShouldRecordAudio, audioRecordingDevice()))
    {
        setLastErrorMessage("Cannot init QT compression session" + mQuickTimeCompressionSession.lastErrorMessage());
        return false;
    }

    connect(&mQuickTimeCompressionSession, SIGNAL(compressionFinished()), this, SLOT(compressionFinished()));
    connect(&mQuickTimeCompressionSession, SIGNAL(audioLevelChanged(quint8)), this, SIGNAL(audioLevelChanged(quint8)));

    mQuickTimeCompressionSession.start();

    return true;
}


bool UBQuickTimeVideoEncoder::stop()
{
    mQuickTimeCompressionSession.stop();

    UBQuickTimeFile::frameBufferNotEmpty.wakeAll();

    return true;
}


void UBQuickTimeVideoEncoder::compressionFinished()
{
        mLastErrorMessage = mQuickTimeCompressionSession.lastErrorMessage();

        emit encodingFinished(mLastErrorMessage.length() == 0);
}


void UBQuickTimeVideoEncoder::newPixmap(const QImage& pImage, long timestamp)
{
    //qDebug() << "New Frame at ms" << timestamp;

    if(mQuickTimeCompressionSession.isCompressionSessionRunning()) {
        if(mPendingImageFrames.length() > 0) {
            foreach(ImageFrame frame, mPendingImageFrames) {
                encodeFrame(frame.image, frame.timestamp);
            }

            mPendingImageFrames.clear();
        }

        encodeFrame(pImage, timestamp);

        UBQuickTimeFile::frameBufferNotEmpty.wakeAll();
    }
    else {
        qDebug() << "queuing frame, compressor not ready";

        ImageFrame frame;
        frame.image = pImage;
        frame.timestamp = timestamp;

        mPendingImageFrames << frame;
    }
}

/**
 * \brief Encode QImage into a video frame and add it to the UBQuickTimeFile's queue.
 *
 * This method retrieves the raw image from the supplied QImage, and uses memcpy to
 * dump it into a CVPixelBuffer, obtained through the UBQuickTimeFile member. The
 * pixel buffer, along with the timestamp, constitute a video frame which is added
 * to the member UBQuickTimeFile's queue.
 */
void UBQuickTimeVideoEncoder::encodeFrame(const QImage& pImage, long timestamp)
{
    Q_ASSERT(pImage.format() == QImage::QImage::Format_RGB32);  // <=> CVPixelBuffers / k32BGRAPixelFormat

    CVPixelBufferRef pixelBuffer = mQuickTimeCompressionSession.newPixelBuffer();
    if (!pixelBuffer)
    {
        setLastErrorMessage("Could not retreive a new pixel buffer");
        return;
    }

    if (CVPixelBufferLockBaseAddress(pixelBuffer, 0) != kCVReturnSuccess)
    {
        setLastErrorMessage("Could not lock pixel buffer base address");
        return;
    }

    void *pixelBufferAddress = CVPixelBufferGetBaseAddress(pixelBuffer);

    if (!pixelBufferAddress)
    {
        setLastErrorMessage("Could not get pixel buffer base address");
        return;
    }

    const uchar* imageBuffer = pImage.bits();

    memcpy((void*) pixelBufferAddress, imageBuffer, pImage.byteCount());

    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

    //qDebug() << "newVideoFrame - PixelBuffer @" << pixelBufferAddress << QTime::currentTime().toString("ss:zzz") << QThread::currentThread();

    UBQuickTimeFile::VideoFrame videoFrame;
    videoFrame.buffer = pixelBuffer;
    videoFrame.timestamp = timestamp;

    mQuickTimeCompressionSession.enqueueVideoFrame(videoFrame);
}


void UBQuickTimeVideoEncoder::setRecordAudio(bool pRecordAudio)
{
    mShouldRecordAudio = pRecordAudio;
}



