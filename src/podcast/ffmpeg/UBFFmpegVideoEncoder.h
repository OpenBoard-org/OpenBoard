#ifndef UBFFMPEGVIDEOENCODER_H
#define UBFFMPEGVIDEOENCODER_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/opt.h>
    #include <libavutil/mathematics.h>
    #include <libswscale/swscale.h>
}

#include <atomic>
#include <stdio.h>

#include <QtCore>
#include <QImage>

#include "podcast/UBAbstractVideoEncoder.h"

class UBFFmpegVideoEncoderWorker;
class UBPodcastController;

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
    void finishEncoding();

private:

    struct ImageFrame
    {
        QImage image;
        long timestamp; // unit: ms
    };

    AVFrame* convertFrame(ImageFrame frame);
    bool init();

    // Queue for any pixmap that might be sent before the encoder is ready
    QQueue<ImageFrame> mPendingFrames;

    QString mLastErrorMessage;
    bool mShouldRecordAudio;

    QThread* mVideoEncoderThread;
    UBFFmpegVideoEncoderWorker* mVideoWorker;

    // Muxer
    AVFormatContext* mOutputFormatContext;
    int mTimebase;

    // Video
    AVStream* mVideoStream;
    struct SwsContext * mSwsContext;

    // Audio
    AVStream* mAudioStream;


    FILE * mFile;

};


class UBFFmpegVideoEncoderWorker : public QObject
{
    Q_OBJECT

    friend class UBFFmpegVideoEncoder;

public:
    UBFFmpegVideoEncoderWorker(UBFFmpegVideoEncoder* controller);
    ~UBFFmpegVideoEncoderWorker();

    bool isRunning() { return mIsRunning; }

    void queueFrame(AVFrame* frame);

public slots:
    void runEncoding();
    void stopEncoding();

signals:
    void encodingFinished();
    void error(QString message);


private:
    void writeLatestVideoFrame();

    UBFFmpegVideoEncoder* mController;

    // std::atomic is C++11. This won't work with msvc2010, so a
    // newer compiler must be used if this is to be used on Windows
    std::atomic<bool> mStopRequested;
    std::atomic<bool> mIsRunning;

    QQueue<AVFrame*> mFrameQueue;
    QMutex mFrameQueueMutex;
    QWaitCondition mWaitCondition;

    AVPacket* mPacket;
};

#endif // UBFFMPEGVIDEOENCODER_H
