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

#ifndef UBFFMPEGVIDEOENCODER_H
#define UBFFMPEGVIDEOENCODER_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libavutil/audio_fifo.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/time.h>
    #include <libavutil/avstring.h>
    #include <libswscale/swscale.h>

// Due to the whole ffmpeg / libAV silliness, we have to support libavresample on some platforms
// Should libswresample be used with libavformat.*.*.100 ? compiles fine but crashes in swr_convert (internal call of soxr_create causes a SIGSEV (EXC_BAD_ACCESS))
// Bad config ?
// With libavresample, no crash, but the sound is inaudible (lot of noise)
#if LIBAVFORMAT_VERSION_MICRO >= 100
    #include <libswresample/swresample.h>
#else
    #include <libavresample/avresample.h>
    #define SwrContext AVAudioResampleContext 
#endif

}

#include <atomic>

#include <QtCore>
#include <QImage>

#include "podcast/UBAbstractVideoEncoder.h"
#include "podcast/ffmpeg/UBMicrophoneInput.h"

class UBFFmpegVideoEncoderWorker;
class UBPodcastController;

/**
 * This class provides an interface between the podcast controller and the ffmpeg
 * back-end.
 * It includes all the necessary objects and methods to record video (muxer, audio and
 * video streams and encoders, etc) from inputs consisting of raw PCM audio and raw RGBA
 * images.
 *
 * A worker thread is used to encode and write the audio and video on-the-fly.
 */

class UBFFmpegVideoEncoder : public UBAbstractVideoEncoder
{
    Q_OBJECT

    friend class UBFFmpegVideoEncoderWorker;

public:

    UBFFmpegVideoEncoder(QObject* parent = NULL);
    virtual ~UBFFmpegVideoEncoder();

    bool start();
    bool stop();

    void newPixmap(const QImage& pImage, long timestamp);

    QString videoFileExtension() const { return "mp4"; }

    QString lastErrorMessage() { return mLastErrorMessage; }

    void setRecordAudio(bool pRecordAudio) { mShouldRecordAudio = pRecordAudio; }

private slots:

    void setLastErrorMessage(const QString& pMessage);
    void onAudioAvailable(QByteArray data);
    void finishEncoding();

private:

    struct ImageFrame
    {
        QImage image;
        long timestamp; // unit: ms
    };

    AVFrame* convertImageFrame(ImageFrame frame);
    AVFrame* convertAudio(QByteArray data);
    void processAudio(QByteArray& data);
    bool init();

    QString mLastErrorMessage;

    QThread* mVideoEncoderThread;
    UBFFmpegVideoEncoderWorker* mVideoWorker;

    // Muxer
    // ------------------------------------------
    AVFormatContext* mOutputFormatContext;
    AVStream* mVideoStream;
    AVStream* mAudioStream;

    // Video
    // ------------------------------------------
    QQueue<ImageFrame> mPendingFrames;
    struct SwsContext * mSwsContext;

    int mVideoTimebase;

    // Audio
    // ------------------------------------------
    bool mShouldRecordAudio;

    UBMicrophoneInput * mAudioInput;
    struct SwrContext * mSwrContext;
    /// Queue for audio that has been rescaled/converted but not encoded yet
    AVAudioFifo *mAudioOutBuffer;

    /// Sample rate for encoded audio
    int mAudioSampleRate;
    /// Total audio frames sent to encoder
    int mAudioFrameCount;
};


class UBFFmpegVideoEncoderWorker : public QObject
{
    Q_OBJECT

    friend class UBFFmpegVideoEncoder;

public:
    UBFFmpegVideoEncoderWorker(UBFFmpegVideoEncoder* controller);
    ~UBFFmpegVideoEncoderWorker();

    bool isRunning() { return mIsRunning; }

    void queueVideoFrame(AVFrame* frame);
    void queueAudioFrame(AVFrame* frame);

public slots:
    void runEncoding();
    void stopEncoding();

signals:
    void encodingFinished();
    void error(QString message);

private:
    void writeLatestVideoFrame();
    void writeLatestAudioFrame();

    UBFFmpegVideoEncoder* mController;

    // std::atomic is C++11. This won't work with msvc2010, so a
    // newer compiler must be used if this class is to be used on Windows
    std::atomic<bool> mStopRequested;
    std::atomic<bool> mIsRunning;

    QQueue<AVFrame*> mImageQueue;
    QQueue<AVFrame*> mAudioQueue;

    QMutex mFrameQueueMutex;
    QWaitCondition mWaitCondition;

    AVPacket* mVideoPacket;
    AVPacket* mAudioPacket;
};

#endif // UBFFMPEGVIDEOENCODER_H
