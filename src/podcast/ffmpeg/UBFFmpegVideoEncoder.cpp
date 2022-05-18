/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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

#include "UBFFmpegVideoEncoder.h"

// Due to the whole FFmpeg / libAV silliness, we have to support libavresample instead
// of libswresapmle on some platforms, as well as now-obsolete function names
#if LIBAVFORMAT_VERSION_MICRO < 100
    #define swr_alloc               avresample_alloc_context
    #define swr_init                avresample_open
    #define swr_get_out_samples     avresample_get_out_samples
    #define swr_free                avresample_free
    #define av_opt_set_sample_fmt   av_opt_set_int

    #define av_frame_alloc          avcodec_alloc_frame
    #define av_frame_free           avcodec_free_frame
    #define av_packet_unref         av_free_packet

    #define AV_ERROR_MAX_STRING_SIZE    64
    #define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)

    uint8_t* audio_samples_buffer; // used by processAudio because av_frame_get_buffer doesn't exist in this version

    int avformat_alloc_output_context2(AVFormatContext **avctx, AVOutputFormat *oformat,
                                       const char *format, const char *filename)
    {
        AVFormatContext *s = avformat_alloc_context();
        int ret = 0;
         
        *avctx = nullptr;
        if (!s)
            goto nomem;

        if (!oformat) {
            if (format) {
                oformat = av_guess_format(format, nullptr, nullptr);
                if (!oformat) {
                    av_log(s, AV_LOG_ERROR, "Requested output format '%s' is not a suitable output format\n", format);
                    ret = AVERROR(EINVAL);
                    goto error;
                }
            } else {
                oformat = av_guess_format(nullptr, filename, nullptr);
                if (!oformat) {
                    ret = AVERROR(EINVAL);
                    av_log(s, AV_LOG_ERROR, "Unable to find a suitable output format for '%s'\n",
                    filename);
                    goto error;
                }
            }
        }

        s->oformat = oformat;
        if (s->oformat->priv_data_size > 0) {
            s->priv_data = av_mallocz(s->oformat->priv_data_size);
            if (!s->priv_data)
                goto nomem;
            if (s->oformat->priv_class) {
                *(const AVClass**)s->priv_data= s->oformat->priv_class;
                av_opt_set_defaults(s->priv_data);
            }
        } else
            s->priv_data = nullptr;

        if (filename)
            av_strlcpy(s->filename, filename, sizeof(s->filename));

        *avctx = s;
        return 0;

    nomem:
        av_log(s, AV_LOG_ERROR, "Out of memory\n");
        ret = AVERROR(ENOMEM);
    error:
        avformat_free_context(s);
        return ret;
    }


    int av_samples_alloc_array_and_samples(uint8_t ***audio_data, int *linesize, int nb_channels,
                                           int nb_samples, enum AVSampleFormat sample_fmt, int align)
    {
        int ret, nb_planes = av_sample_fmt_is_planar(sample_fmt) ? nb_channels : 1;
         
        *audio_data = (uint8_t**) av_malloc(sizeof(*audio_data) * nb_planes);
        if (!*audio_data)
            return AVERROR(ENOMEM);
        ret = av_samples_alloc(*audio_data, linesize, nb_channels,
        nb_samples, sample_fmt, align);
        if (ret < 0)
            av_freep(audio_data);
        return ret;
    }

    int swr_convert(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in, int in_count)
    {
        return avresample_convert(s, out, 0, out_count, const_cast<uint8_t **>(in), 0, in_count);
    }


#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,55,0)
     void av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb)
     {
         if (pkt->pts != AV_NOPTS_VALUE)
             pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
         if (pkt->dts != AV_NOPTS_VALUE)
             pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
         if (pkt->duration > 0)
             pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
         if (pkt->convergence_duration > 0)
             pkt->convergence_duration = av_rescale_q(pkt->convergence_duration, src_tb, dst_tb);
     }
#endif

#if defined(LIBAVRESAMPLE_VERSION_INT) && LIBAVRESAMPLE_VERSION_INT < AV_VERSION_INT(1,3,0)
    //#include <libavresample/internal.h>
    int avresample_get_out_samples(AVAudioResampleContext *avr, int in_nb_samples)
    {
        int64_t samples = avresample_get_delay(avr) + (int64_t)in_nb_samples;
        /*
        if (avr->resample_needed) {
            samples = av_rescale_rnd(samples,
                                     avr->out_sample_rate,
                                     avr->in_sample_rate,
                                     AV_ROUND_UP);
        }
        */
        samples += avresample_available(avr);
        if (samples > INT_MAX)
            return AVERROR(EINVAL);
        return samples;
    }

#endif

//-------------------------------------------------------------------------
// Utility functions
//-------------------------------------------------------------------------

QString avErrorToQString(int errnum)
{
    char error[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(errnum, error, AV_ERROR_MAX_STRING_SIZE);

    return QString(error);
}

/**
 * @brief Write a given frame to the audio stream or, if a null frame is passed, flush the stream.
 *
 * @param frame An AVFrame to be written to the stream, or nullptr to flush the stream
 * @param packet A (reusable) packet, used to temporarily store frame data
 * @param stream The stream to write to
 * @param outputFormatContext The output format context
 */
void writeFrame(AVFrame *frame, AVPacket *packet, AVStream *stream, AVCodecContext* c, AVFormatContext *outputFormatContext)
{
    int ret;

#if LIBAVFORMAT_VERSION_MAJOR < 58
    int gotOutput;
    av_init_packet(packet);

    do {
        if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            ret = avcodec_encode_audio2(stream->codec, packet, frame, &gotOutput);
        else
            ret = avcodec_encode_video2(stream->codec, packet, frame, &gotOutput);

        if (ret < 0)
            qWarning() << "Couldn't encode audio frame: " << avErrorToQString(ret);

        else if (gotOutput) {
            AVRational codecTimebase = stream->codec->time_base;
            AVRational streamVideoTimebase = stream->time_base;

            av_packet_rescale_ts(packet, codecTimebase, streamVideoTimebase);
            packet->stream_index = stream->index;

            av_interleaved_write_frame(outputFormatContext, packet);
            av_packet_unref(packet);
        }

    } while (gotOutput && !frame);
#else
    // send the frame to the encoder
    ret = avcodec_send_frame(c, frame);

    while (ret >= 0) {
        ret = avcodec_receive_packet(c, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            qWarning() << "Couldn't encode audio frame: " << avErrorToQString(ret);
        }

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(packet, c->time_base, stream->time_base);
        packet->stream_index = stream->index;

        /* Write the compressed frame to the media file. */
        ret = av_interleaved_write_frame(outputFormatContext, packet);
        /* pkt is now blank (av_interleaved_write_frame() takes ownership of
         * its contents and resets pkt), so that no unreferencing is necessary.
         * This would be different if one used av_write_frame(). */
    }
#endif
}

void flushStream(AVPacket *packet, AVStream *stream, AVCodecContext* c, AVFormatContext *outputFormatContext)
{
    writeFrame(nullptr, packet, stream, c, outputFormatContext);
}

//-------------------------------------------------------------------------
// UBFFmpegVideoEncoder
//-------------------------------------------------------------------------

UBFFmpegVideoEncoder::UBFFmpegVideoEncoder(QObject* parent)
    : UBAbstractVideoEncoder(parent)
    , mOutputFormatContext(nullptr)
    , mSwsContext(nullptr)
    , mShouldRecordAudio(true)
    , mAudioInput(nullptr)
    , mSwrContext(nullptr)
    , mAudioOutBuffer(nullptr)
    , mAudioSampleRate(44100)
    , mAudioFrameCount(0)
{

    mVideoTimebase = 100 * framesPerSecond();

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
    qWarning() << "FFmpeg video encoder:" << pMessage;
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
#if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
    avcodec_register_all();
#endif

    AVDictionary * options = nullptr;
    int ret;

    // Output format and context
    // --------------------------------------
    if (avformat_alloc_output_context2(&mOutputFormatContext, nullptr,
                                       "mp4", nullptr) < 0)
    {
        setLastErrorMessage("Couldn't allocate video format context");
        return false;
    }

    // The default codecs for mp4 are h264 and aac, we use those


    // Video codec and context
    // -------------------------------------
    mVideoStream = avformat_new_stream(mOutputFormatContext, nullptr);
    if (!mVideoStream) {
        setLastErrorMessage("Could not allocate stream");
        return false;
    }

    auto videoCodec = avcodec_find_encoder(mOutputFormatContext->oformat->video_codec);
    if (!videoCodec) {
        setLastErrorMessage("Video codec not found");
        return false;
    }

    AVCodecContext* c = avcodec_alloc_context3(videoCodec);
    if (!c) {
        setLastErrorMessage("Could not allocate encoding context");
        return false;
    }

    c->codec_id = mOutputFormatContext->oformat->video_codec;
    c->bit_rate = videoBitsPerSecond();
    c->width = videoSize().width();
    c->height = videoSize().height();
    c->time_base = {1, mVideoTimebase};
    c->gop_size = 10;
    c->max_b_frames = 0;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (mOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    mVideoStream->time_base = c->time_base;

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

    mVideoCodecContext = c;

    // copy the stream parameters to the muxer
    ret = avcodec_parameters_from_context(mVideoStream->codecpar, c);
    if (ret < 0) {
        setLastErrorMessage("Could not copy the stream parameters");
        return false;
    }

    // Source images are RGB32, and should be converted to YUV for h264 video
    mSwsContext = sws_getCachedContext(mSwsContext,
                                       c->width, c->height, AV_PIX_FMT_RGB32,
                                       c->width, c->height, c->pix_fmt,
                                       SWS_BICUBIC, nullptr, nullptr, nullptr);

    // Audio codec and context
    // -------------------------------------
    if (mShouldRecordAudio) {

        // Microphone input

        mAudioInput = new UBMicrophoneInput();

        connect(mAudioInput, SIGNAL(audioLevelChanged(quint8)),
                this, SIGNAL(audioLevelChanged(quint8)));

        connect(mAudioInput, SIGNAL(dataAvailable(QByteArray)),
                this, SLOT(onAudioAvailable(QByteArray)));

        mAudioInput->setInputDevice(audioRecordingDevice());

        if (!mAudioInput->init()) {
            setLastErrorMessage("Couldn't initialize audio input");
            return false;
        }

        int inChannelCount = mAudioInput->channelCount();
        int inSampleRate = mAudioInput->sampleRate();

        // Codec

        auto audioCodec = avcodec_find_encoder(mOutputFormatContext->oformat->audio_codec);

        if (!audioCodec) {
            setLastErrorMessage("Audio codec not found");
            return false;
        }

        mAudioStream = avformat_new_stream(mOutputFormatContext, audioCodec);
        if (!mAudioStream) {
            setLastErrorMessage("Could not allocate stream");
            return false;
        }

        mAudioStream->id = mOutputFormatContext->nb_streams-1;

        c = avcodec_alloc_context3(audioCodec);
        if (!c) {
            setLastErrorMessage("Could not allocate encoding context");
            return false;
        }

        c->bit_rate = 96000;
        c->sample_fmt  = audioCodec->sample_fmts ? audioCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;// FLTP by default for AAC
        c->sample_rate = mAudioSampleRate;

#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 25, 100)
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        c->channels  = av_get_channel_layout_nb_channels(c->channel_layout);
#else
        av_channel_layout_copy(&c->ch_layout, &(AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO);
#endif

        //deprecated on ffmpeg 4
        c->strict_std_compliance = -2;// Enable use of experimental codec

        //https://trac.ffmpeg.org/wiki/Encode/H.264#Profile
        //Omit this unless your target device only supports a certain profile
        //(see https://trac.ffmpeg.org/wiki/Encode/H.264#Compatibility).
        //c->profile = FF_PROFILE_AAC_MAIN;

        c->time_base = { 1, c->sample_rate };

        if (mOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        ret = avcodec_open2(c, audioCodec, nullptr);

        if (ret < 0) {
            setLastErrorMessage(QString("Couldn't open audio codec: ") + avErrorToQString(ret));
            return false;
        }

        mAudioCodecContext = c;

        // copy the stream parameters to the muxer
        ret = avcodec_parameters_from_context(mAudioStream->codecpar, c);
        if (ret < 0) {
            setLastErrorMessage("Could not copy the stream parameters");
            return false;
        }

        // The input (raw sound from the microphone) may not match the codec's sampling rate,
        // sample format or number of channels; we use libswresample to convert and resample it
        mSwrContext = swr_alloc();
        if (!mSwrContext) {
            setLastErrorMessage("Could not allocate resampler context");
            return false;
        }

        av_opt_set_int(mSwrContext, "in_sample_rate", inSampleRate, 0);
        av_opt_set_sample_fmt(mSwrContext, "in_sample_fmt", (AVSampleFormat)mAudioInput->sampleFormat(), 0);
        av_opt_set_int(mSwrContext, "out_sample_rate", c->sample_rate, 0);
        av_opt_set_sample_fmt(mSwrContext, "out_sample_fmt", c->sample_fmt, 0);

#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 25, 100)
        av_opt_set_int(mSwrContext, "in_channel_count", inChannelCount, 0);
        av_opt_set_int(mSwrContext, "in_channel_layout", av_get_default_channel_layout(inChannelCount), 0);
        av_opt_set_int(mSwrContext, "out_channel_count", c->channels, 0);
        av_opt_set_int(mSwrContext, "out_channel_layout", c->channel_layout, 0);
#else
        AVChannelLayout inChannelLayout;
        av_channel_layout_default(&inChannelLayout, inChannelCount);
        av_opt_set_chlayout  (mSwrContext, "in_chlayout", &inChannelLayout, 0);
        av_opt_set_chlayout  (mSwrContext, "out_chlayout", &c->ch_layout, 0);
#endif

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
    ret = avformat_write_header(mOutputFormatContext, nullptr);

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
    if (!mVideoWorker->isRunning()) {
        qDebug() << "Encoder worker thread not running. Queuing frame.";
        mPendingFrames.enqueue({pImage, timestamp});
    }

    else {
        // First send any queued frames, then the latest one
        while (!mPendingFrames.isEmpty()) {
            AVFrame* avFrame = convertImageFrame(mPendingFrames.dequeue());
            if (avFrame)
                mVideoWorker->queueVideoFrame(avFrame);
        }

        // note: if converting the frame turns out to be too slow to do here, it
        // can always be done from the worker thread (in that case,
        // the worker's queue would contain ImageFrames rather than AVFrames)

        AVFrame* avFrame = convertImageFrame({pImage, timestamp});
        if (avFrame)
            mVideoWorker->queueVideoFrame(avFrame);

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

    avFrame->format = mVideoCodecContext->pix_fmt;
    avFrame->width = mVideoCodecContext->width;
    avFrame->height = mVideoCodecContext->height;
    avFrame->pts = mVideoTimebase * frame.timestamp / 1000;

    const uchar * rgbImage = frame.image.bits();

    const int in_linesize[1] = { frame.image.bytesPerLine() };

    // Allocate the output image
    if (av_image_alloc(avFrame->data, avFrame->linesize, mVideoCodecContext->width,
                       mVideoCodecContext->height, mVideoCodecContext->pix_fmt, 32) < 0)
    {
        qWarning() << "Couldn't allocate image";
        return nullptr;
    }

    sws_scale(mSwsContext,
              (const uint8_t* const*)&rgbImage,
              in_linesize,
              0,
              mVideoCodecContext->height,
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
    AVCodecContext* codecContext = mAudioCodecContext;

    const char * inSamples = data.constData();

    // The number of samples (per channel) in the input
    int inSamplesCount = data.size() / ((mAudioInput->sampleSize() / 8) * mAudioInput->channelCount());

    // The number of samples we will get after conversion
    int outSamplesCount = swr_get_out_samples(mSwrContext, inSamplesCount);

    // Allocate output samples
    uint8_t ** outSamples = nullptr;
    int outSamplesLineSize;

    ret = av_samples_alloc_array_and_samples(&outSamples, &outSamplesLineSize,
                                             codecContext->channels, outSamplesCount,
                                             codecContext->sample_fmt, 0);
    if (ret < 0) {
        qWarning() << "Could not allocate audio samples" << avErrorToQString(ret);
        return;
    }

    // Convert to destination format

    ret = swr_convert(mSwrContext,
                      outSamples, outSamplesCount,
                      (const uint8_t **)&inSamples, inSamplesCount);

    if (ret < 0) {
        qWarning() << "Error converting audio samples: " << avErrorToQString(ret);
        return;
    }

    // Append the converted samples to the out buffer.
    ret = av_audio_fifo_write(mAudioOutBuffer, (void**)outSamples, outSamplesCount);
    if (ret < 0) {
        qWarning() << "Could not write to FIFO queue: " << avErrorToQString(ret);
        return;
    }

    // Keep the data queued until next call if the encoder thread isn't running
    if (!mVideoWorker->isRunning())
        return;

    bool framesAdded = false;
    while (av_audio_fifo_size(mAudioOutBuffer) > codecContext->frame_size) {

        AVFrame * avFrame = av_frame_alloc();
        avFrame->nb_samples = codecContext->frame_size;

#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 25, 100)
        avFrame->channel_layout = codecContext->channel_layout;
#else
        av_channel_layout_copy(&avFrame->ch_layout, &codecContext->ch_layout);
#endif

        avFrame->format = codecContext->sample_fmt;
        avFrame->sample_rate = codecContext->sample_rate;
        avFrame->pts = mAudioFrameCount;

#if LIBAVFORMAT_VERSION_MICRO < 100
        int buffer_size = av_samples_get_buffer_size(nullptr, codecContext->channels, codecContext->frame_size, codecContext->sample_fmt, 0);
        audio_samples_buffer = (uint8_t*)av_malloc(buffer_size);
        if (!audio_samples_buffer) {
            qWarning() << "Couldn't allocate samples for audio frame: " << avErrorToQString(ret);
            break;
        }

        ret = avcodec_fill_audio_frame(avFrame,
                                       codecContext->channels,
                                       codecContext->sample_fmt,
                                       (const uint8_t*)audio_samples_buffer,
                                       buffer_size,
                                       0);

#else
        ret = av_frame_get_buffer(avFrame, 0);
#endif
        if (ret < 0) {
            qWarning() << "Couldn't allocate frame: " << avErrorToQString(ret);
            break;
        }

        ret = av_audio_fifo_read(mAudioOutBuffer, (void**)avFrame->data, codecContext->frame_size);
        if (ret < 0)
            qWarning() << "Could not read from FIFO queue: " << avErrorToQString(ret);

        else {
            mAudioFrameCount += codecContext->frame_size;

            mVideoWorker->queueAudioFrame(avFrame);
            framesAdded = true;
        }
    }

    if (framesAdded)
        mVideoWorker->mWaitCondition.wakeAll();
}

void UBFFmpegVideoEncoder::finishEncoding()
{
    qDebug() << "VideoEncoder::finishEncoding called";

    flushStream(mVideoWorker->mVideoPacket, mVideoStream, mVideoCodecContext, mOutputFormatContext);

    if (mShouldRecordAudio)
        flushStream(mVideoWorker->mAudioPacket, mAudioStream, mAudioCodecContext, mOutputFormatContext);

    av_write_trailer(mOutputFormatContext);
    avio_close(mOutputFormatContext->pb);

    avcodec_close(mVideoCodecContext);
    sws_freeContext(mSwsContext);

    if (mShouldRecordAudio) {
        avcodec_close(mAudioCodecContext);
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
    mVideoPacket = av_packet_alloc();
    mAudioPacket = av_packet_alloc();
}

UBFFmpegVideoEncoderWorker::~UBFFmpegVideoEncoderWorker()
{
    if (mVideoPacket)
        av_packet_free(&mVideoPacket);

    if (mAudioPacket)
        av_packet_free(&mAudioPacket);
}

void UBFFmpegVideoEncoderWorker::stopEncoding()
{
    qDebug() << "Video worker: stop requested";
    mStopRequested = true;
    mWaitCondition.wakeAll();
}

void UBFFmpegVideoEncoderWorker::queueVideoFrame(AVFrame* frame)
{
    if (frame) {
        mFrameQueueMutex.lock();
        mImageQueue.enqueue(frame);
        mFrameQueueMutex.unlock();
    }
}

void UBFFmpegVideoEncoderWorker::queueAudioFrame(AVFrame* frame)
{
    if (frame) {
        mFrameQueueMutex.lock();
        mAudioQueue.enqueue(frame);
        mFrameQueueMutex.unlock();
    }
}

/**
 * The main encoding function. Takes the queued frames and
 * writes them to the video and audio streams
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
    writeFrame(frame, mVideoPacket, mController->mVideoStream, mController->mVideoCodecContext, mController->mOutputFormatContext);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
}

void UBFFmpegVideoEncoderWorker::writeLatestAudioFrame()
{
    AVFrame *frame = mAudioQueue.dequeue();
    writeFrame(frame, mAudioPacket, mController->mAudioStream, mController->mAudioCodecContext, mController->mOutputFormatContext);
    av_frame_free(&frame);

#if LIBAVFORMAT_VERSION_MICRO < 100
    if (audio_samples_buffer) {
        av_free(audio_samples_buffer);
        av_freep(&frame->data[0]);
        audio_samples_buffer = nullptr;
    }
#endif
}
