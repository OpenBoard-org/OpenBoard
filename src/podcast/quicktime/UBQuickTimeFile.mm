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




#include "UBQuickTimeFile.h"

#include <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

#include "UBAudioQueueRecorder.h"
#include <QtGui>

#include "core/memcheck.h"

QWaitCondition UBQuickTimeFile::frameBufferNotEmpty;

UBQuickTimeFile::UBQuickTimeFile(QObject * pParent)
    : QThread(pParent)
    , mVideoWriter(0)
    , mVideoWriterInput(0)
    , mAdaptor(0)
    , mAudioWriterInput(0)
    , mWaveRecorder(0)
    , mTimeScale(1000)
    , mRecordAudio(true)
    , mShouldStopCompression(false)
    , mCompressionSessionRunning(false)
{
    mVideoDispatchQueue = dispatch_queue_create("org.oef.VideoDispatchQueue", NULL);
    mAudioDispatchQueue = dispatch_queue_create("org.oef.AudioDispatchQueue", NULL);
}


UBQuickTimeFile::~UBQuickTimeFile()
{
}

bool UBQuickTimeFile::init(const QString& pVideoFileName, const QString& pProfileData, int pFramesPerSecond
                , const QSize& pFrameSize, bool pRecordAudio, const QString& audioRecordingDevice)
{
    Q_UNUSED(pProfileData);
    Q_UNUSED(pFramesPerSecond);

    mFrameSize = pFrameSize;
    mVideoFileName = pVideoFileName;
    mRecordAudio = pRecordAudio;
    //mRecordAudio = false;

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

    [mVideoWriterInput requestMediaDataWhenReadyOnQueue:mVideoDispatchQueue
        usingBlock:^{
            frameQueueMutex.lock();
            //frameBufferNotEmpty.wait(&UBQuickTimeFile::frameQueueMutex); // TODO: monitor performance with and without this

            if (!mShouldStopCompression && 
                !frameQueue.isEmpty() &&
                [mVideoWriterInput isReadyForMoreMediaData]) 
            {
                // in this case the last few frames may be dropped if the queue isn't empty...
                    VideoFrame frame = frameQueue.dequeue();
                    appendFrameToVideo(frame.buffer, frame.timestamp);
            }

            frameQueueMutex.unlock();
            
    }];
    
    if (mRecordAudio) {
        [mAudioWriterInput requestMediaDataWhenReadyOnQueue:mAudioDispatchQueue
            usingBlock:^{
                audioQueueMutex.lock();
                if (!audioQueue.isEmpty() && 
                    [mAudioWriterInput isReadyForMoreMediaData]) 
                {
                        appendSampleBuffer(audioQueue.dequeue());
                }
                audioQueueMutex.unlock();

            }];
    }

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
                    this, &UBQuickTimeFile::enqueueAudioBuffer);

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
            //AVChannelLayoutKey    : channelLayoutAsData,
            AVNumberOfChannelsKey : [NSNumber numberWithUnsignedInteger:1]
        };

        mAudioWriterInput = [[AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeAudio
                                               outputSettings:compressionAudioSettings] retain];

        NSCParameterAssert([mVideoWriter canAddInput:mAudioWriterInput]);
        [mVideoWriter addInput:mAudioWriterInput];
    }


    // Begin the writing session
    bool canStartWriting = [mVideoWriter startWriting];
    [mVideoWriter startSessionAtSourceTime:CMTimeMake(0, mTimeScale)];

    mStartTime = CFAbsoluteTimeGetCurrent(); // used for audio timestamp calculation
    mLastFrameTimestamp = CMTimeMake(0, mTimeScale);

    return (mVideoWriter != nil) && (mVideoWriterInput != nil) && canStartWriting;
}

/**
 * \brief Close the recording sesion and finish writing the video file
 */
void UBQuickTimeFile::endSession()
{
    //qDebug() << "Ending session";


    [mVideoWriterInput markAsFinished];
    if (mAudioWriterInput != 0)
        [mAudioWriterInput markAsFinished];

    [mVideoWriter finishWritingWithCompletionHandler:^{
        [mAdaptor release];
        [mVideoWriterInput release];

        if (mAudioWriterInput != 0)
            [mAudioWriterInput release];
        
        [mVideoWriter release];

        mAdaptor = nil;
        mVideoWriterInput = nil;
        mVideoWriter = nil;
        mAudioWriterInput = nil;

        if (mWaveRecorder) {
            mWaveRecorder->close();
            mWaveRecorder->deleteLater();
        }


        emit compressionFinished();
    }];

}

/**
 * \brief Request the recording to stop
 */
void UBQuickTimeFile::stop()
{
    //qDebug() << "requested end of recording";
    mShouldStopCompression = true;


    frameQueueMutex.lock();
    audioQueueMutex.lock();
    endSession();
    frameQueueMutex.unlock();
    audioQueueMutex.unlock();
}


/**
 * \brief Create and return a CVPixelBufferRef 
 *
 * The CVPixelBuffer is created from the input adaptor's CVPixelBufferPool
 */
CVPixelBufferRef UBQuickTimeFile::newPixelBuffer()
{
    CVPixelBufferRef pixelBuffer = 0;

    CVReturn result = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, mAdaptor.pixelBufferPool, &pixelBuffer);

    if (result != kCVReturnSuccess) {
        setLastErrorMessage("Could not retrieve CV buffer from pool (error " + QString::number(result) + ")");
        return 0;
    }

    return pixelBuffer;
}

void UBQuickTimeFile::enqueueVideoFrame(VideoFrame frame)
{
    frameQueueMutex.lock();
    frameQueue.enqueue(frame);
    frameQueueMutex.unlock();
}

/**
 * \brief Add a frame to the pixel buffer adaptor
 * \param pixelBuffer The CVPixelBufferRef (video frame) to add to the movie
 * \param msTimeStamp Timestamp, in milliseconds, of the frame
 */
void UBQuickTimeFile::appendFrameToVideo(CVPixelBufferRef pixelBuffer, long msTimeStamp)
{
    //qDebug() << "appending video frame";
    CMTime t = CMTimeMake((msTimeStamp * mTimeScale / 1000.0), mTimeScale);

    // The timestamp must be both valid and larger than the previous frame's timestamp
    if (CMTIME_IS_VALID(t) && CMTimeCompare(t, mLastFrameTimestamp) == 1) {

        bool added = [mAdaptor appendPixelBuffer: pixelBuffer
                            withPresentationTime: t];
        if (!added)
            setLastErrorMessage(QString("Could not encode frame at time %1").arg(msTimeStamp));

        mLastFrameTimestamp = t;
    }

    else {
        qDebug() << "Frame dropped; timestamp was smaller or equal to previous frame's timestamp of: "
                 << mLastFrameTimestamp.value << "/" << mLastFrameTimestamp.timescale;
    }

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
void UBQuickTimeFile::enqueueAudioBuffer(void* pBuffer,
                                        long pLength)
{
    
    if(!mRecordAudio || mShouldStopCompression)
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

    //qDebug() << "enqueueAudioBuffer, timeStamp = " << timeStamp.value << " / " << timeStamp.timescale;

    
    audioQueueMutex.lock();
    audioQueue.enqueue(sampleBuffer);
    audioQueueMutex.unlock();
    
    //qDebug() << "buffer enqueued";
    


}


bool UBQuickTimeFile::appendSampleBuffer(CMSampleBufferRef sampleBuffer)
{
    bool success = [mAudioWriterInput appendSampleBuffer:sampleBuffer];
    
    if (!success)
        setLastErrorMessage(QString("Failed to append sample buffer to audio input"));
    

    CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);

    CFRelease(sampleBuffer);
    CFRelease(blockBuffer);

    return success;
}


/**
 * \brief Print an error message to the terminal, and store it
 */
void UBQuickTimeFile::setLastErrorMessage(const QString& error)
{
    mLastErrorMessage = error;
    qWarning() << "UBQuickTimeFile error" << error;
}
