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
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
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

signals:

    void encodingFinished(bool ok);

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
