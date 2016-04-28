#include "UBFFmpegVideoEncoder.h"

// Future proofing
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

QString avErrorToQString(int errnum)
{
    char error[AV_ERROR_MAX_STRING_SIZE];
    av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, errnum);

    return QString(error);
}

/**
 * @brief Constructor for the ffmpeg video encoder
 *
 *
 * This class provides an interface between the screencast controller and the ffmpeg
 * back-end. It initializes the audio and video encoders and frees them when done;
 * worker threads handle the actual encoding of frames.
 *
 */
UBFFmpegVideoEncoder::UBFFmpegVideoEncoder(QObject* parent)
    : UBAbstractVideoEncoder(parent)
    , mOutputFormatContext(NULL)
    , mSwsContext(NULL)
    , mFile(NULL)
{

    mTimebase = 100 * framesPerSecond();
    qDebug() <<  "timebase: " << mTimebase;

    mVideoEncoderThread = new QThread;
    mVideoWorker = new UBFFmpegVideoEncoderWorker(this);
    mVideoWorker->moveToThread(mVideoEncoderThread);

    connect(mVideoWorker, SIGNAL(error(QString)),
            this, SLOT(setLastErrorMessage(QString)));

    connect(mVideoEncoderThread, SIGNAL(started()),
            mVideoWorker, SLOT(runEncoding()));

    connect(mVideoWorker, SIGNAL(encodingFinished()),
            mVideoEncoderThread, SLOT(quit()));

    connect(mVideoEncoderThread, SIGNAL(finished()),
            this, SLOT(finishEncoding()));
}

UBFFmpegVideoEncoder::~UBFFmpegVideoEncoder()
{
    if (mVideoWorker)
        delete mVideoWorker;

    if (mVideoEncoderThread)
        delete mVideoEncoderThread;

}

void UBFFmpegVideoEncoder::setLastErrorMessage(const QString& pMessage)
{
    qDebug() << "FFmpeg video encoder:" << pMessage;
    mLastErrorMessage = pMessage;
}

bool UBFFmpegVideoEncoder::start()
{
    bool initialized = init();

    if (initialized)
        mVideoEncoderThread->start();

    return initialized;
}

bool UBFFmpegVideoEncoder::stop()
{
    qDebug() << "Video encoder: stop requested";

    mVideoWorker->stopEncoding();

    return true;
}

bool UBFFmpegVideoEncoder::init()
{
    // Initialize ffmpeg lib
    av_register_all();
    avcodec_register_all();

    AVDictionary * options = NULL;
    int ret;

    // Output format and context
    // --------------------------------------

    if (avformat_alloc_output_context2(&mOutputFormatContext, NULL,
                                       "mp4", NULL) < 0)
    {
        setLastErrorMessage("Couldn't allocate video format context");
        return false;
    }

    // The default codecs for mp4 are h264 and aac, we use those


    // Video codec and context
    // -------------------------------------

    AVCodec * videoCodec = avcodec_find_encoder(mOutputFormatContext->oformat->video_codec);
    if (!videoCodec) {
        setLastErrorMessage("Video codec not found");
        return false;
    }

    mVideoStream = avformat_new_stream(mOutputFormatContext, 0);
    mVideoStream->time_base = {1, mTimebase};

    avcodec_get_context_defaults3(mVideoStream->codec, videoCodec);
    AVCodecContext* c = avcodec_alloc_context3(videoCodec);

    c->bit_rate = videoBitsPerSecond();
    c->width = videoSize().width();
    c->height = videoSize().height();
    c->time_base = {1, mTimebase};
    c->gop_size = 10;
    c->max_b_frames = 0;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (mOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /*
     * Supported pixel formats for h264 are:
     *   AV_PIX_FMT_YUV420P
     *   AV_PIX_FMT_YUV422P
     *   AV_PIX_FMT_YUV444P
     *   AV_PIX_FMT_YUVJ420P
    */

    av_dict_set(&options, "preset", "slow", 0);
    av_dict_set(&options, "crf", "20", 0);

    ret = avcodec_open2(c, videoCodec, &options);

    if (ret < 0) {
        setLastErrorMessage(QString("Couldn't open video codec: ") + avErrorToQString(ret));
        return false;
    }

    mVideoStream->codec = c;

    // Source images are RGB32, and should be converted to YUV for h264 video
    mSwsContext = sws_getCachedContext(mSwsContext,
                                       c->width, c->height, AV_PIX_FMT_RGB32,
                                       c->width, c->height, c->pix_fmt,
                                       SWS_BICUBIC, 0, 0, 0);

    // Audio codec and context
    // -------------------------------------
    /*
    AVCodec * audioCodec = avcodec_find_encoder(mOutputFormatContext->oformat->audio_codec);
    mAudioStream = avformat_new_stream(mOutputFormatContext, audioCodec);
    */


    // Open the output file
    ret = avio_open(&(mOutputFormatContext->pb), videoFileName().toStdString().c_str(), AVIO_FLAG_WRITE);

    if (ret < 0) {
        setLastErrorMessage(QString("Couldn't open video file for writing: ") + avErrorToQString(ret));
        return false;
    }

    // Write stream header
    ret = avformat_write_header(mOutputFormatContext, NULL);

    if (ret < 0) {
        setLastErrorMessage(QString("Couldn't write header to file: ") + avErrorToQString(ret));
        return false;
    }

    return true;
}
void UBFFmpegVideoEncoder::newPixmap(const QImage &pImage, long timestamp)
{
    static bool isFirstFrame = true;
    if (isFirstFrame) {
        timestamp = 0;
        isFirstFrame = false;
    }

    if (!mVideoWorker->isRunning()) {
        qDebug() << "Encoder worker thread not running. Queuing frame.";
        mPendingFrames.enqueue({pImage, timestamp});
    }

    else {
        // First send any queued frames, then the latest one
        while (!mPendingFrames.isEmpty()) {
            AVFrame* avFrame = convertFrame(mPendingFrames.dequeue());
            if (avFrame)
                mVideoWorker->queueFrame(avFrame);
        }

        // note: if converting the frame turns out to be too slow to do here, it
        // can always be done from the worker thread (in thta case,
        // the worker's queue would contain ImageFrames rather than AVFrames)

        AVFrame* avFrame = convertFrame({pImage, timestamp});
        if (avFrame)
            mVideoWorker->queueFrame(avFrame);

        // signal the worker that frames are available
        mVideoWorker->mWaitCondition.wakeAll();
    }
}

/** Convert a frame consisting of a QImage and timestamp to an AVFrame
 * with the right pixel format and PTS
 */
AVFrame* UBFFmpegVideoEncoder::convertFrame(ImageFrame frame)
{
    AVFrame* avFrame = av_frame_alloc();

    avFrame->format = mVideoStream->codec->pix_fmt;
    avFrame->width = mVideoStream->codec->width;
    avFrame->height = mVideoStream->codec->height;
    avFrame->pts = mTimebase * frame.timestamp / 1000;

    const uchar * rgbImage = frame.image.bits();

    const int in_linesize[1] = { frame.image.bytesPerLine() };

    // Allocate the output image
    if (av_image_alloc(avFrame->data, avFrame->linesize, mVideoStream->codec->width,
                       mVideoStream->codec->height, mVideoStream->codec->pix_fmt, 32) < 0)
    {
        setLastErrorMessage("Couldn't allocate image");
        return NULL;
    }

    sws_scale(mSwsContext,
              (const uint8_t* const*)&rgbImage,
              in_linesize,
              0,
              mVideoStream->codec->height,
              avFrame->data,
              avFrame->linesize);

    return avFrame;
}

void UBFFmpegVideoEncoder::finishEncoding()
{
    qDebug() << "VideoEncoder::finishEncoding called";

    // Some frames may not be encoded, so we call avcodec_encode_video2 until they're all done

    int gotOutput;
    do {
        // TODO: get rid of duplicated code (videoWorker does almost exactly this during encoding)

        AVPacket* packet = mVideoWorker->mPacket;

        if (avcodec_encode_video2(mVideoStream->codec, packet, NULL, &gotOutput) < 0) {
            setLastErrorMessage("Couldn't encode frame to video");
            continue;
        }
        if (gotOutput) {
            AVRational codecTimebase = mVideoStream->codec->time_base;
            AVRational streamTimebase = mVideoStream->time_base;

            av_packet_rescale_ts(packet, codecTimebase, streamTimebase);
            packet->stream_index = mVideoStream->index;

            av_interleaved_write_frame(mOutputFormatContext, packet);
            av_packet_unref(packet);
        }
    } while (gotOutput);

    av_write_trailer(mOutputFormatContext);

    avio_close(mOutputFormatContext->pb);
    avcodec_close(mVideoStream->codec);
    sws_freeContext(mSwsContext);
    avformat_free_context(mOutputFormatContext);

    emit encodingFinished(true);
}

//-------------------------------------------------------------------------
// Worker
//-------------------------------------------------------------------------

UBFFmpegVideoEncoderWorker::UBFFmpegVideoEncoderWorker(UBFFmpegVideoEncoder* controller)
    : mController(controller)
{
    mStopRequested = false;
    mIsRunning = false;
    mPacket = new AVPacket();
}

UBFFmpegVideoEncoderWorker::~UBFFmpegVideoEncoderWorker()
{}

void UBFFmpegVideoEncoderWorker::stopEncoding()
{
    qDebug() << "Video worker: stop requested";
    mStopRequested = true;
    mWaitCondition.wakeAll();
}

void UBFFmpegVideoEncoderWorker::queueFrame(AVFrame* frame)
{
    mFrameQueueMutex.lock();
    mFrameQueue.enqueue(frame);
    mFrameQueueMutex.unlock();
}

/**
 * The main encoding function. Takes the queued image frames and
 * assembles them into the video
 */
void UBFFmpegVideoEncoderWorker::runEncoding()
{
    mIsRunning = true;

    while (!mStopRequested) {
        mFrameQueueMutex.lock();
        mWaitCondition.wait(&mFrameQueueMutex);

        while (!mFrameQueue.isEmpty()) {
            writeLatestVideoFrame();
        }

        /*
        while (!mAudioQueue.isEmpty()) {
            writeLatestAudioFrame();
        }
        */

        mFrameQueueMutex.unlock();
    }

    emit encodingFinished();
}

void UBFFmpegVideoEncoderWorker::writeLatestVideoFrame()
{
    AVFrame* frame = mFrameQueue.dequeue();

    int gotOutput;
    av_init_packet(mPacket);
    mPacket->data = NULL;
    mPacket->size = 0;

    // qDebug() << "Encoding frame to video. Pts: " << frame->pts << "/" << mController->mTimebase;

    if (avcodec_encode_video2(mController->mVideoStream->codec, mPacket, frame, &gotOutput) < 0)
        emit error("Error encoding frame to video");

    if (gotOutput) {
        AVRational codecTimebase = mController->mVideoStream->codec->time_base;
        AVRational streamTimebase = mController->mVideoStream->time_base;


        // recalculate the timestamp to match the stream's timebase
        av_packet_rescale_ts(mPacket, codecTimebase, streamTimebase);
        mPacket->stream_index = mController->mVideoStream->index;

        // qDebug() << "Writing encoded packet to file; pts: " << mPacket->pts << "/" << streamTimebase.den;

        av_interleaved_write_frame(mController->mOutputFormatContext, mPacket);
        av_packet_unref(mPacket);
    }

    // Duct-tape solution. I assume there's a better way of doing this, but:
    // some players like VLC show a black screen until the second frame (which
    // can be several seconds after the first one). Simply duplicating the first frame
    // seems to solve this problem, and also allows the thumbnail to be generated.

    static bool firstRun = true;
    if (firstRun) {
        firstRun = false;
        frame->pts += 1;
        mFrameQueue.enqueue(frame); // only works when the queue is empty at this point. todo: clean this up!
    }
    else
        // free the frame
        av_frame_free(&frame);
}

