/*
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




#include "UBQuickTimeFile.h"

#include <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

#include "UBAudioQueueRecorder.h"
#include <QtGui>

#include "core/memcheck.h"

QQueue<UBQuickTimeFile::VideoFrame> UBQuickTimeFile::frameQueue;
QMutex UBQuickTimeFile::frameQueueMutex;
QWaitCondition UBQuickTimeFile::frameBufferNotEmpty;

UBQuickTimeFile::UBQuickTimeFile(QObject * pParent)
    : QThread(pParent)
    , mVideoWriter(0)
    , mVideoWriterInput(0)
    , mAdaptor(0)
    , mTimeScale(1000)
    , mRecordAudio(true)
    , mShouldStopCompression(false)
    , mCompressionSessionRunning(false)
{
}


UBQuickTimeFile::~UBQuickTimeFile()
{
    // destruction of mWaveRecorder is handled by endSession()

}

bool UBQuickTimeFile::init(const QString& pVideoFileName, const QString& pProfileData, int pFramesPerSecond
                , const QSize& pFrameSize, bool pRecordAudio, const QString& audioRecordingDevice)
{
    mFrameSize = pFrameSize;
    mVideoFileName = pVideoFileName;
    mRecordAudio = pRecordAudio;

    if (mRecordAudio)
        mAudioRecordingDeviceName = audioRecordingDevice;
    else
        mAudioRecordingDeviceName = "";


    qDebug() << "UBQuickTimeFile created; video size: " << pFrameSize.width() << " x " << pFrameSize.height();

    return true;

}


void UBQuickTimeFile::run()
{
    mShouldStopCompression = false;

    if (!beginSession())
        return;

    mCompressionSessionRunning = true;
    emit compressionSessionStarted();

    do {
        // Video
        frameQueueMutex.lock();

        frameBufferNotEmpty.wait(&UBQuickTimeFile::frameQueueMutex);

        if (!frameQueue.isEmpty()) {
            QQueue<VideoFrame> localQueue = frameQueue;
            frameQueue.clear();

            frameQueueMutex.unlock();

            while (!localQueue.isEmpty()) {
                if ([mVideoWriterInput isReadyForMoreMediaData]) {
                    VideoFrame frame = localQueue.dequeue();
                    appendVideoFrame(frame.buffer, frame.timestamp);
                }
                else
                    usleep(10000);
            }
        }
        else
            frameQueueMutex.unlock();

    } while(!mShouldStopCompression);

    endSession();

}

/**
  * \brief Begin the recording session; initialize the audio/video writer
  * \return true if the session was initialized successfully
  *
  * This function initializes the AVAssetWriter and associated video and audio inputs.
  * Video is encoded as H264; audio is encoded as AAC.
  */
bool UBQuickTimeFile::beginSession()
{
    NSError *outError;
    NSString * outputPath = [[NSString alloc] initWithUTF8String: mVideoFileName.toUtf8().data()];
    NSURL * outputUrl = [[NSURL alloc] initFileURLWithPath: outputPath];

    if (!outputUrl) {
        qDebug() << "Podcast video URL invalid; not recording";
        return false;
    }

    // Create and check the assetWriter
    mVideoWriter = [[AVAssetWriter assetWriterWithURL:outputUrl
                                   fileType:AVFileTypeQuickTimeMovie
                                   error:&outError] retain];
    NSCParameterAssert(mVideoWriter);

    mVideoWriter.movieTimeScale = mTimeScale;



    // Video
    //

    int frameWidth = mFrameSize.width();
    int frameHeight = mFrameSize.height();

    // Create the input and check it
    NSDictionary * videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:
                                                AVVideoCodecH264, AVVideoCodecKey,
                                                [NSNumber numberWithInt:frameWidth], AVVideoWidthKey,
                                                [NSNumber numberWithInt:frameHeight], AVVideoHeightKey,
                                                nil];


    mVideoWriterInput = [[AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo
                                            outputSettings:videoSettings] retain];
    NSCParameterAssert(mVideoWriterInput);



    // Pixel Buffer Adaptor. This makes it possible to pass CVPixelBuffers to the WriterInput
    NSDictionary* pixelBufSettings = [NSDictionary dictionaryWithObjectsAndKeys:
                                         [NSNumber numberWithInt:kCVPixelFormatType_32BGRA], kCVPixelBufferPixelFormatTypeKey,
                                         [NSNumber numberWithInt: frameWidth], kCVPixelBufferWidthKey,
                                         [NSNumber numberWithInt: frameHeight], kCVPixelBufferHeightKey,
                                         nil];

    mAdaptor = [[AVAssetWriterInputPixelBufferAdaptor
                        assetWriterInputPixelBufferAdaptorWithAssetWriterInput:mVideoWriterInput
                        sourcePixelBufferAttributes:pixelBufSettings] retain];

    NSCParameterAssert([mVideoWriter canAddInput:mVideoWriterInput]);
    [mVideoWriter addInput:mVideoWriterInput];



    // Audio
    //

    if(mRecordAudio) {
        mWaveRecorder = new UBAudioQueueRecorder();

        // Get the audio format description from mWaveRecorder
        CMAudioFormatDescriptionCreate(kCFAllocatorDefault, mWaveRecorder->audioFormat(),
                                       0, NULL, 0, NULL, NULL,
                                       &mAudioFormatDescription);

        if(mWaveRecorder->init(mAudioRecordingDeviceName)) {
            connect(mWaveRecorder, &UBAudioQueueRecorder::newWaveBuffer,
                    this, &UBQuickTimeFile::appendAudioBuffer);

            connect(mWaveRecorder, SIGNAL(audioLevelChanged(quint8)),
                    this, SIGNAL(audioLevelChanged(quint8)));
        }
        else {
            setLastErrorMessage(mWaveRecorder->lastErrorMessage());
            mWaveRecorder->deleteLater();
            mRecordAudio = false;
        }

        // Audio is mono, and compressed to AAC at 128kbps

        AudioChannelLayout audioChannelLayout = {
            .mChannelLayoutTag = kAudioChannelLayoutTag_Mono,
            .mChannelBitmap = 0,
            .mNumberChannelDescriptions = 0
        };

        NSData *channelLayoutAsData = [NSData dataWithBytes:&audioChannelLayout
                                              length:offsetof(AudioChannelLayout, mChannelDescriptions)];

        NSDictionary * compressionAudioSettings = @{
            AVFormatIDKey         : [NSNumber numberWithUnsignedInt:kAudioFormatMPEG4AAC],
            AVEncoderBitRateKey   : [NSNumber numberWithInteger:128000],
            AVSampleRateKey       : [NSNumber numberWithInteger:44100],
            AVChannelLayoutKey    : channelLayoutAsData,
            AVNumberOfChannelsKey : [NSNumber numberWithUnsignedInteger:1]
        };

        mAudioWriterInput = [[AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeAudio
                                               outputSettings:compressionAudioSettings] retain];

        NSCParameterAssert([mVideoWriter canAddInput:mAudioWriterInput]);
        [mVideoWriter addInput:mAudioWriterInput];

        qDebug() << "audio writer input created and added";
    }


    // Begin the writing session
    bool canStartWriting = [mVideoWriter startWriting];
    [mVideoWriter startSessionAtSourceTime:CMTimeMake(0, mTimeScale)];

    mStartTime = CFAbsoluteTimeGetCurrent(); // used for audio timestamp calculation


    return (mVideoWriter != nil) && (mVideoWriterInput != nil) && canStartWriting;
}

/**
 * \brief Close the recording sesion and finish writing the video file
 */
void UBQuickTimeFile::endSession()
{
    [mVideoWriterInput markAsFinished];
    [mVideoWriter finishWritingWithCompletionHandler:^{}];

    [mAdaptor release];
    [mVideoWriterInput release];
    [mVideoWriter release];
    [mAudioWriterInput release];

    mAdaptor = nil;
    mVideoWriterInput = nil;
    mVideoWriter = nil;
    mAudioWriterInput = nil;

    if (mWaveRecorder) {
        mWaveRecorder->close();
        mWaveRecorder->deleteLater();
    }

}

/**
 * \brief Request the recording to stop
 */
void UBQuickTimeFile::stop()
{
    mShouldStopCompression = true;
}


/**
 * \brief Create a CVPixelBufferRef from the input adaptor's CVPixelBufferPool
 */
CVPixelBufferRef UBQuickTimeFile::newPixelBuffer()
{
    CVPixelBufferRef pixelBuffer = 0;

    if(CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, mAdaptor.pixelBufferPool, &pixelBuffer) != kCVReturnSuccess)
    {
        setLastErrorMessage("Could not retrieve CV buffer from pool");
        return 0;
    }

    return pixelBuffer;
}


/**
 * \brief Add a frame to the pixel buffer adaptor
 * \param pixelBuffer The CVPixelBufferRef (video frame) to add to the movie
 * \param msTimeStamp Timestamp, in milliseconds, of the frame
 */
void UBQuickTimeFile::appendVideoFrame(CVPixelBufferRef pixelBuffer, long msTimeStamp)
{
    CMTime t = CMTimeMake((msTimeStamp * mTimeScale / 1000.0), mTimeScale);

    bool added = [mAdaptor appendPixelBuffer: pixelBuffer
                        withPresentationTime: t];

    if (!added)
        setLastErrorMessage(QString("Could not encode frame at time %1").arg(msTimeStamp));


    CVPixelBufferRelease(pixelBuffer);
}




/**
 * \brief Append an AudioQueue Buffer to the audio AVAssetWriterInput
 * \param pBuffer The AudioQueueBufferRef to add. Must be uncompressed (LPCM).
 * \param pLength The length of the buffer, in Bytes
 *
 * This function serves as an interface between the low-level audio stream
 * (implemented in the UBAudioQueueRecorder class) and the recording, handled
 * by the AVAssetWriterInput instance mAudioWriterInput.
 */
void UBQuickTimeFile::appendAudioBuffer(void* pBuffer,
                                        long pLength)
{
    if(!mRecordAudio)
        return;


    // CMSampleBuffers require a CMBlockBuffer to hold the media data; we
    // create a blockBuffer here from the AudioQueueBuffer's data.
    CMBlockBufferRef blockBuffer;
    CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                       pBuffer,
                                       pLength,
                                       kCFAllocatorNull,
                                       NULL,
                                       0,
                                       pLength,
                                       kCMBlockBufferAssureMemoryNowFlag,
                                       &blockBuffer);


    // Timestamp of current sample
    CFAbsoluteTime currentTime = CFAbsoluteTimeGetCurrent();
    CFTimeInterval elapsedTime = currentTime - mStartTime;
    CMTime timeStamp = CMTimeMake(elapsedTime * mTimeScale, mTimeScale);

    // Number of samples in the buffer
    long nSamples = pLength / mWaveRecorder->audioFormat()->mBytesPerFrame;

    CMSampleBufferRef sampleBuffer;
    CMAudioSampleBufferCreateWithPacketDescriptions(kCFAllocatorDefault,
                                                    blockBuffer,
                                                    true,
                                                    NULL,
                                                    NULL,
                                                    mAudioFormatDescription,
                                                    nSamples,
                                                    timeStamp,
                                                    NULL,
                                                    &sampleBuffer);


    // Wait until the AssetWriterInput is ready, but no more than 100ms
    // (bit of a duct tape solution; cleaner solution would be to use a QQueue,
    // similar to the VideoWriter)
    int waitTime = 0;
    while(![mAudioWriterInput isReadyForMoreMediaData] && waitTime < 100) {
        waitTime += 10;
        usleep(10000);
    }

    if ([mAudioWriterInput isReadyForMoreMediaData]) {
        if(![mAudioWriterInput appendSampleBuffer:sampleBuffer])
            setLastErrorMessage(QString("Failed to append sample buffer to audio input"));
    }
    else
        setLastErrorMessage(QString("AudioWriterInput not ready. Buffer dropped."));



    CFRelease(sampleBuffer);
    CFRelease(blockBuffer);

    // The audioQueueBuffers are all freed when UBAudioQueueRecorder::close() is called
}

/**
 * \brief Print an error message to the terminal, and store it
 */
void UBQuickTimeFile::setLastErrorMessage(const QString& error)
{
    mLastErrorMessage = error;
    qWarning() << "UBQuickTimeFile error" << error;
}
