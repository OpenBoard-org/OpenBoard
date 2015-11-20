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
    , mCVPixelBufferPool(0)
    , mFramesPerSecond(-1)
    , mTimeScale(100)
    , mRecordAudio(true)
    , mShouldStopCompression(false)
    , mCompressionSessionRunning(false)
    , mPendingFrames(0)
{
    // NOOP
}


UBQuickTimeFile::~UBQuickTimeFile()
{
    // NOOP
}

bool UBQuickTimeFile::init(const QString& pVideoFileName, const QString& pProfileData, int pFramesPerSecond
                , const QSize& pFrameSize, bool pRecordAudio, const QString& audioRecordingDevice)
{
    mFrameSize = pFrameSize;
    mFramesPerSecond = pFramesPerSecond;
    mVideoFileName = pVideoFileName;
    mRecordAudio = pRecordAudio;
    mSpatialQuality = pProfileData;

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
    mPendingFrames = 0;

    if (!beginSession())
        return;

    mCompressionSessionRunning = true;
    emit compressionSessionStarted();

    do { 
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
  * \brief Initialize the AVAssetWriter, which handles writing the media to file
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

    
    
    // Add the input(s) to the assetWriter
    NSCParameterAssert([mVideoWriter canAddInput:mVideoWriterInput]);
    [mVideoWriter addInput:mVideoWriterInput];


    // begin the writing session
    bool canStartWriting = [mVideoWriter startWriting];
    [mVideoWriter startSessionAtSourceTime:CMTimeMake(0, mTimeScale)];

    // return true if everything was created and started successfully
    return (mVideoWriter != nil) && (mVideoWriterInput != nil) && canStartWriting;
}

/**
 * \brief Close the recording sesion and finish writing the video file
 */
void UBQuickTimeFile::endSession()
{
    [mVideoWriterInput markAsFinished];
    bool success = [mVideoWriter finishWriting];

    [mAdaptor release];
    [mVideoWriterInput release];
    [mVideoWriter release];

    mAdaptor = nil;
    mVideoWriterInput = nil;
    mVideoWriter = nil;
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
 */
void UBQuickTimeFile::appendVideoFrame(CVPixelBufferRef pixelBuffer, long msTimeStamp)
{
    //qDebug() << "adding video frame at time: " << msTimeStamp;

    CMTime t = CMTimeMake((msTimeStamp * mTimeScale / 1000.0), mTimeScale); 

    bool added = [mAdaptor appendPixelBuffer: pixelBuffer
                        withPresentationTime: t];

    if (!added) 
        setLastErrorMessage(QString("Could not encode frame at time %1").arg(msTimeStamp));


    CVPixelBufferRelease(pixelBuffer);
}

void UBQuickTimeFile::setLastErrorMessage(const QString& error)
{
    mLastErrorMessage = error;
    qWarning() << "UBQuickTimeFile error" << error;
}



