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
 * a worker thread handles the actual encoding and writing of frames.
 *
 */
UBFFmpegVideoEncoder::UBFFmpegVideoEncoder(QObject* parent)
    : UBAbstractVideoEncoder(parent)
    , mOutputFormatContext(NULL)
    , mSwsContext(NULL)
    , mShouldRecordAudio(true)
    , mAudioInput(NULL)
    , mSwrContext(NULL)
    , mAudioOutBuffer(NULL)
    , mAudioSampleRate(44100)
    , mAudioFrameCount(0)
{
    if (mShouldRecordAudio) {
        mAudioInput = new UBMicrophoneInput();

        connect(mAudioInput, SIGNAL(audioLevelChanged(quint8)),
                this, SIGNAL(audioLevelChanged(quint8)));

        connect(mAudioInput, SIGNAL(dataAvailable(QByteArray)),
                this, SLOT(onAudioAvailable(QByteArray)));
    }

    mVideoTimebase = 100 * framesPerSecond();
    qDebug() <<  "timebase: " << mVideoTimebase;

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

    if (mAudioInput)
        delete mAudioInput;
}

void UBFFmpegVideoEncoder::setLastErrorMessage(const QString& pMessage)
{
    qDebug() << "FFmpeg video encoder:" << pMessage;
    mLastErrorMessage = pMessage;
}


bool UBFFmpegVideoEncoder::start()
{
    bool initialized = init();

    if (initialized) {
        mVideoEncoderThread->start();
        if (mShouldRecordAudio)
            mAudioInput->start();
    }

    return initialized;
}

bool UBFFmpegVideoEncoder::stop()
{
    qDebug() << "Video encoder: stop requested";

    mVideoWorker->stopEncoding();

    if (mShouldRecordAudio)
        mAudioInput->stop();

    return true;
}

bool UBFFmpegVideoEncoder::init()
{
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
    mVideoStream = avformat_new_stream(mOutputFormatContext, 0);

    AVCodec * videoCodec = avcodec_find_encoder(mOutputFormatContext->oformat->video_codec);
    if (!videoCodec) {
        setLastErrorMessage("Video codec not found");
        return false;
    }

    AVCodecContext* c = avcodec_alloc_context3(videoCodec);

    c->bit_rate = videoBitsPerSecond();
    c->width = videoSize().width();
    c->height = videoSize().height();
    c->time_base = {1, mVideoTimebase};
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
    if (mShouldRecordAudio) {

        // Microphone input
        if (!mAudioInput->init()) {
            setLastErrorMessage("Couldn't initialize audio input");
            return false;
        }


        int inChannelCount = mAudioInput->channelCount();
        int inSampleRate = mAudioInput->sampleRate();
        int inSampleSize = mAudioInput->sampleSize();

        qDebug() << "inChannelCount = " << inChannelCount;
        qDebug() << "inSampleRate = " << inSampleRate;
        qDebug() << "inSampleSize = " << inSampleSize;

        // Codec
        AVCodec * audioCodec = avcodec_find_encoder(mOutputFormatContext->oformat->audio_codec);

        if (!audioCodec) {
            setLastErrorMessage("Audio codec not found");
            return false;
        }

        mAudioStream = avformat_new_stream(mOutputFormatContext, audioCodec);
        mAudioStream->id = mOutputFormatContext->nb_streams-1;

        c = mAudioStream->codec;

        c->bit_rate = 96000;
        c->sample_fmt = audioCodec->sample_fmts[0]; // FLTP by default for AAC
        c->sample_rate = mAudioSampleRate;
        c->channels = 2;
        c->channel_layout = av_get_default_channel_layout(c->channels);
        c->profile = FF_PROFILE_AAC_MAIN;
        c->time_base = {1, mAudioSampleRate};

        if (mOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        ret = avcodec_open2(c, audioCodec, NULL);

        if (ret < 0) {
            setLastErrorMessage(QString("Couldn't open audio codec: ") + avErrorToQString(ret));
            return false;
        }

        // Resampling / format converting context
        mSwrContext = swr_alloc();
        if (!mSwrContext) {
            setLastErrorMessage("Could not allocate resampler context");
            return false;
        }

        av_opt_set_int(mSwrContext, "in_channel_count", inChannelCount, 0);
        av_opt_set_int(mSwrContext, "in_sample_rate", inSampleRate, 0);
        av_opt_set_sample_fmt(mSwrContext, "in_sample_fmt", (AVSampleFormat)mAudioInput->sampleFormat(), 0);
        av_opt_set_int(mSwrContext, "out_channel_count", c->channels, 0);
        av_opt_set_int(mSwrContext, "out_sample_rate", c->sample_rate, 0);
        av_opt_set_sample_fmt(mSwrContext, "out_sample_fmt", c->sample_fmt, 0);

        ret = swr_init(mSwrContext);
        if (ret < 0) {
            setLastErrorMessage(QString("Couldn't initialize the resampling context: ") + avErrorToQString(ret));
            return false;
        }

        // Buffer for resampled/converted audio
        mAudioOutBuffer = av_audio_fifo_alloc(c->sample_fmt, c->channels, c->frame_size);
    }


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

/**
 * This function should be called every time a new "screenshot" is ready.
 * The image is converted to the right format and sent to the encoder.
 */
void UBFFmpegVideoEncoder::newPixmap(const QImage &pImage, long timestamp)
{
    // really necessary?
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
            AVFrame* avFrame = convertImageFrame(mPendingFrames.dequeue());
            if (avFrame)
                mVideoWorker->queueFrame(avFrame);
        }

        // note: if converting the frame turns out to be too slow to do here, it
        // can always be done from the worker thread (in that case,
        // the worker's queue would contain ImageFrames rather than AVFrames)

        AVFrame* avFrame = convertImageFrame({pImage, timestamp});
        if (avFrame)
            mVideoWorker->queueFrame(avFrame);

        // signal the worker that frames are available
        mVideoWorker->mWaitCondition.wakeAll();
    }
}

/**
 * Convert a frame consisting of a QImage and timestamp to an AVFrame
 * with the right pixel format and PTS
 */
AVFrame* UBFFmpegVideoEncoder::convertImageFrame(ImageFrame frame)
{
    AVFrame* avFrame = av_frame_alloc();

    avFrame->format = mVideoStream->codec->pix_fmt;
    avFrame->width = mVideoStream->codec->width;
    avFrame->height = mVideoStream->codec->height;
    avFrame->pts = mVideoTimebase * frame.timestamp / 1000;

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

void UBFFmpegVideoEncoder::onAudioAvailable(QByteArray data)
{
    if (!data.isEmpty())
        processAudio(data);
}

/**
* Resample and convert audio to match the encoder's settings and queue the
* output. If enough output data is available, it is packaged into AVFrames and
* sent to the encoder thread.
*/
void UBFFmpegVideoEncoder::processAudio(QByteArray &data)
{
    int ret;
    AVCodecContext* codecContext = mAudioStream->codec;

    const char * inSamples = data.constData();

    // The number of samples (per channel) in the input
    int inSamplesCount = data.size() / ((mAudioInput->sampleSize() / 8) * mAudioInput->channelCount());

    // The number of samples we will get after conversion
    int outSamplesCount = swr_get_out_samples(mSwrContext, inSamplesCount);

    // Allocate output samples
    uint8_t ** outSamples = NULL;
    int outSamplesLineSize;

    ret = av_samples_alloc_array_and_samples(&outSamples, &outSamplesLineSize,
                                             codecContext->channels, outSamplesCount,
                                             codecContext->sample_fmt, 0);
    if (ret < 0) {
        qDebug() << "Could not allocate audio samples" << avErrorToQString(ret);
        return;
    }

    // Convert to destination format
    ret = swr_convert(mSwrContext,
                      outSamples, outSamplesCount,
                      (const uint8_t **)&inSamples, inSamplesCount);
    if (ret < 0) {
        qDebug() << "Error converting audio samples: " << avErrorToQString(ret);
        return;
    }

    // Append the converted samples to the out buffer.
    ret = av_audio_fifo_write(mAudioOutBuffer, (void**)outSamples, outSamplesCount);
    if (ret < 0) {
        qDebug() << "Could not write to FIFO queue: " << avErrorToQString(ret);
        return;
    }

    // Keep the data queued until next call if the encoder thread isn't running
    if (!mVideoWorker->isRunning())
        return;

    bool framesAdded = false;
    while (av_audio_fifo_size(mAudioOutBuffer) > codecContext->frame_size) {
        AVFrame * avFrame = av_frame_alloc();
        avFrame->nb_samples = codecContext->frame_size;
        avFrame->channel_layout = codecContext->channel_layout;
        avFrame->format = codecContext->sample_fmt;
        avFrame->sample_rate = codecContext->sample_rate;
        avFrame->pts = mAudioFrameCount;

        ret = av_frame_get_buffer(avFrame, 0);
        if (ret < 0) {
            qDebug() << "Couldn't allocate frame: " << avErrorToQString(ret);
            break;
        }

        ret = av_audio_fifo_read(mAudioOutBuffer, (void**)avFrame->data, codecContext->frame_size);
        if (ret < 0)
            qDebug() << "Could not read from FIFO queue: " << avErrorToQString(ret);

        else {
            mAudioFrameCount += codecContext->frame_size;

            mVideoWorker->queueAudio(avFrame);
            framesAdded = true;
        }
    }

    if (framesAdded)
        mVideoWorker->mWaitCondition.wakeAll();
}

void UBFFmpegVideoEncoder::finishEncoding()
{
    qDebug() << "VideoEncoder::finishEncoding called";

    // Some frames may not be encoded, so we call avcodec_encode_video2 until they're all done

    int gotOutput;
    do {
        // TODO: get rid of duplicated code (videoWorker does almost exactly this during encoding)

        AVPacket* packet = mVideoWorker->mVideoPacket;

        if (avcodec_encode_video2(mVideoStream->codec, packet, NULL, &gotOutput) < 0) {
            setLastErrorMessage("Couldn't encode frame to video");
            continue;
        }
        if (gotOutput) {
            AVRational codecTimebase = mVideoStream->codec->time_base;
            AVRational streamVideoTimebase = mVideoStream->time_base;

            av_packet_rescale_ts(packet, codecTimebase, streamVideoTimebase);
            packet->stream_index = mVideoStream->index;

            av_interleaved_write_frame(mOutputFormatContext, packet);
            av_packet_unref(packet);
        }
    } while (gotOutput);

    if (mShouldRecordAudio) {

        int gotOutput, ret;
        do {

            AVPacket* packet = mVideoWorker->mAudioPacket;

            ret = avcodec_encode_audio2(mAudioStream->codec, packet, NULL, &gotOutput);
            if (ret < 0)
                setLastErrorMessage("Couldn't encode frame to audio");

            else if (gotOutput) {
                AVRational codecTimebase = mAudioStream->codec->time_base;
                AVRational streamVideoTimebase = mAudioStream->time_base;

                av_packet_rescale_ts(packet, codecTimebase, streamVideoTimebase);
                packet->stream_index = mAudioStream->index;

                av_interleaved_write_frame(mOutputFormatContext, packet);
                av_packet_unref(packet);
            }
        } while (gotOutput);

    }

    av_write_trailer(mOutputFormatContext);

    avio_close(mOutputFormatContext->pb);
    avcodec_close(mVideoStream->codec);
    sws_freeContext(mSwsContext);

    if (mShouldRecordAudio) {
        avcodec_close(mAudioStream->codec);
        swr_free(&mSwrContext);
    }

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
    mVideoPacket = new AVPacket();
    mAudioPacket = new AVPacket();
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
    if (frame) {
        mFrameQueueMutex.lock();
        mImageQueue.enqueue(frame);
        mFrameQueueMutex.unlock();
    }
}

void UBFFmpegVideoEncoderWorker::queueAudio(AVFrame* frame)
{
    if (frame) {
        mFrameQueueMutex.lock();
        mAudioQueue.enqueue(frame);
        mFrameQueueMutex.unlock();
    }
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

        while (!mImageQueue.isEmpty()) {
            writeLatestVideoFrame();
        }

        while (!mAudioQueue.isEmpty()) {
            writeLatestAudioFrame();
        }

        mFrameQueueMutex.unlock();
    }

    emit encodingFinished();
}

void UBFFmpegVideoEncoderWorker::writeLatestVideoFrame()
{
    AVFrame* frame = mImageQueue.dequeue();

    int gotOutput;
    av_init_packet(mVideoPacket);
    mVideoPacket->data = NULL;
    mVideoPacket->size = 0;

    // qDebug() << "Encoding frame to video. Pts: " << frame->pts << "/" << mController->mVideoTimebase;

    if (avcodec_encode_video2(mController->mVideoStream->codec, mVideoPacket, frame, &gotOutput) < 0)
        emit error("Error encoding video frame");

    if (gotOutput) {
        AVRational codecTimebase = mController->mVideoStream->codec->time_base;
        AVRational streamVideoTimebase = mController->mVideoStream->time_base;


        // recalculate the timestamp to match the stream's timebase
        av_packet_rescale_ts(mVideoPacket, codecTimebase, streamVideoTimebase);
        mVideoPacket->stream_index = mController->mVideoStream->index;

        // qDebug() << "Writing encoded packet to file; pts: " << mVideoPacket->pts << "/" << streamVideoTimebase.den;

        av_interleaved_write_frame(mController->mOutputFormatContext, mVideoPacket);
        av_packet_unref(mVideoPacket);
    }

    // Duct-tape solution. I assume there's a better way of doing this, but:
    // some players like VLC show a black screen until the second frame (which
    // can be several seconds after the first one). Simply duplicating the first frame
    // seems to solve this problem, and also allows the thumbnail to be generated.

    static bool firstRun = true;
    if (firstRun) {
        firstRun = false;
        frame->pts += 1;
        mImageQueue.enqueue(frame); // only works when the queue is empty at this point. todo: clean this up!
    }
    else
        // free the frame
        av_frame_free(&frame);
}

void UBFFmpegVideoEncoderWorker::writeLatestAudioFrame()
{
    AVFrame *frame = mAudioQueue.dequeue();

    int gotOutput, ret;

    av_init_packet(mAudioPacket);
    mAudioPacket->data = NULL;
    mAudioPacket->size = 0;

    //qDebug() << "Encoding audio frame";

    ret = avcodec_encode_audio2(mController->mAudioStream->codec, mAudioPacket, frame, &gotOutput);
    if (ret < 0)
        emit error(QString("Error encoding audio frame: ") + avErrorToQString(ret));

    else if (gotOutput) {
        //qDebug() << "Writing audio frame to stream";

        AVRational codecTimebase = mController->mAudioStream->codec->time_base;
        AVRational streamVideoTimebase = mController->mAudioStream->time_base;

        av_packet_rescale_ts(mAudioPacket, codecTimebase, streamVideoTimebase);
        mAudioPacket->stream_index = mController->mAudioStream->index;

        av_interleaved_write_frame(mController->mOutputFormatContext, mAudioPacket);
        av_packet_unref(mAudioPacket);
    }

    av_frame_free(&frame);
}

