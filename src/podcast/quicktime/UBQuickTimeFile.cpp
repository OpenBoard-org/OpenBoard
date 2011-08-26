/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
 

#include "UBQuickTimeFile.h"

#include <AudioToolbox/AudioToolbox.h>

#include "UBAudioQueueRecorder.h"
#include <QtGui>

QQueue<UBQuickTimeFile::VideoFrame> UBQuickTimeFile::frameQueue;
QMutex UBQuickTimeFile::frameQueueMutex;
QWaitCondition UBQuickTimeFile::frameBufferNotEmpty;


UBQuickTimeFile::UBQuickTimeFile(QObject * pParent)
    : QThread(pParent)
    , mVideoCompressionSession(0)
    , mVideoMedia(0)
    , mSoundMedia(0)
    , mVideoOutputTrack(0)
    , mSoundOutputTrack(0)
    , mCVPixelBufferPool(0)
    , mOutputMovie(0)
    , mFramesPerSecond(-1)
    , mTimeScale(100)
    , mRecordAudio(true)
    , mWaveRecorder(0)
    , mSouldStopCompression(false)
    , mCompressionSessionRunning(false)
    , mPendingFrames(0)
{
    // NOOP
}


bool UBQuickTimeFile::init(const QString& pVideoFileName, const QString& pProfileData, int pFramesPerSecond
                , const QSize& pFrameSize, bool pRecordAudio, const QString& audioRecordingDevice)
{
    mFrameSize = pFrameSize;
    mFramesPerSecond = pFramesPerSecond;
    mVideoFileName = pVideoFileName;
    mRecordAudio = pRecordAudio && QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5; // Audio Queue are available in 10.5 +;

    if (mRecordAudio)
        mAudioRecordingDeviceName = audioRecordingDevice;
    else
        mAudioRecordingDeviceName = "";

    if (pProfileData.toLower() == "lossless")
        mSpatialQuality = codecLosslessQuality;
    if (pProfileData.toLower() == "high")
        mSpatialQuality = codecHighQuality;
    else if (pProfileData.toLower() == "normal")
        mSpatialQuality = codecNormalQuality;
    else if (pProfileData.toLower() == "low")
        mSpatialQuality = codecLowQuality;
    else
        mSpatialQuality = codecHighQuality;

    qDebug() << "Quality " << pProfileData << mSpatialQuality;

    return true;

}


void UBQuickTimeFile::run()
{
    EnterMoviesOnThread(kCSAcceptThreadSafeComponentsOnlyMode);

    mSouldStopCompression = false;
    mPendingFrames = 0;

    createCompressionSession();

    mCompressionSessionRunning = true;
    emit compressionSessionStarted();

    while(!mSouldStopCompression)
    {
        frameQueueMutex.lock();
        //qDebug() << "run .... wait" << QTime::currentTime();

        frameBufferNotEmpty.wait(&UBQuickTimeFile::frameQueueMutex);

        //qDebug() << "awakend ..." << QTime::currentTime();
        if (!frameQueue.isEmpty())
        {
            QQueue<VideoFrame> localQueue = frameQueue;
            frameQueue.clear();

            frameQueueMutex.unlock();

            while (!localQueue.isEmpty())
            {
                VideoFrame frame = localQueue.dequeue();
                appendVideoFrame(frame.buffer, frame.timestamp);
            }
        }
        else
        {
            frameQueueMutex.unlock();
        }
    }

    flushPendingFrames();
}


bool UBQuickTimeFile::createCompressionSession()
{
    CodecType codecType = kH264CodecType;

    CFStringRef keys[] = {kCVPixelBufferPixelFormatTypeKey, kCVPixelBufferWidthKey, kCVPixelBufferHeightKey};

    int width = mFrameSize.width();
    int height = mFrameSize.height();
    int pixelFormat = k32BGRAPixelFormat;

    CFTypeRef values[] =
    {
        (CFTypeRef)CFNumberCreate(0, kCFNumberIntType, (void*)&pixelFormat),
        (CFTypeRef)CFNumberCreate(0, kCFNumberIntType, (void*)&width),
        (CFTypeRef)CFNumberCreate(0, kCFNumberIntType, (void*)&height)
    };

    CFDictionaryRef pixelBufferAttributes = CFDictionaryCreate(kCFAllocatorDefault
            , (const void **)keys, (const void **)values, 3, 0, 0);

    if(!pixelBufferAttributes)
    {
        setLastErrorMessage("Could not create CV buffer pool pixel buffer attributes");
        return false;
    }

    OSStatus err = noErr;
    ICMEncodedFrameOutputRecord encodedFrameOutputRecord = {0};
    ICMCompressionSessionOptionsRef sessionOptions = 0;

    err = ICMCompressionSessionOptionsCreate(0, &sessionOptions);
    if(err)
    {
        setLastErrorMessage(QString("ICMCompressionSessionOptionsCreate() failed %1").arg(err));
        goto bail;
    }

    // We must set this flag to enable P or B frames.
    err = ICMCompressionSessionOptionsSetAllowTemporalCompression(sessionOptions, true);
    if(err)
    {
        setLastErrorMessage(QString("ICMCompressionSessionOptionsSetAllowTemporalCompression() failed %1").arg(err));
        goto bail;
    }

    // We must set this flag to enable B frames.
    err = ICMCompressionSessionOptionsSetAllowFrameReordering(sessionOptions, true);
    if(err)
    {
        setLastErrorMessage(QString("ICMCompressionSessionOptionsSetAllowFrameReordering() failed %1").arg(err));
        goto bail;
    }

    // Set the maximum key frame interval, also known as the key frame rate.
    err = ICMCompressionSessionOptionsSetMaxKeyFrameInterval(sessionOptions, mFramesPerSecond);
    if(err)
    {
        setLastErrorMessage(QString("ICMCompressionSessionOptionsSetMaxKeyFrameInterval() failed %1").arg(err));
        goto bail;
    }

    // This allows the compressor more flexibility (ie, dropping and coalescing frames).
    err = ICMCompressionSessionOptionsSetAllowFrameTimeChanges(sessionOptions, true);
    if(err)
    {
        setLastErrorMessage(QString("ICMCompressionSessionOptionsSetAllowFrameTimeChanges() failed %1").arg(err));
        goto bail;
    }

    // Set the average quality.
    err = ICMCompressionSessionOptionsSetProperty(sessionOptions,
                                                      kQTPropertyClass_ICMCompressionSessionOptions,
                                                      kICMCompressionSessionOptionsPropertyID_Quality,
                                                      sizeof(mSpatialQuality),
                                                      &mSpatialQuality);
    if(err)
    {
        setLastErrorMessage(QString("ICMCompressionSessionOptionsSetProperty(Quality) failed %1").arg(err));
        goto bail;
    }

    //qDebug() << "available quality" << mSpatialQuality;

    encodedFrameOutputRecord.encodedFrameOutputCallback = addEncodedFrameToMovie;
    encodedFrameOutputRecord.encodedFrameOutputRefCon = this;
    encodedFrameOutputRecord.frameDataAllocator = 0;

    err = ICMCompressionSessionCreate(0, mFrameSize.width(), mFrameSize.height(), codecType, mTimeScale,
                                                                      sessionOptions, pixelBufferAttributes, &encodedFrameOutputRecord, &mVideoCompressionSession);
    if(err)
    {
        setLastErrorMessage(QString("ICMCompressionSessionCreate() failed %1").arg(err));
        goto bail;
    }

    mCVPixelBufferPool = ICMCompressionSessionGetPixelBufferPool(mVideoCompressionSession);

    if(!mCVPixelBufferPool)
    {
        setLastErrorMessage("ICMCompressionSessionGetPixelBufferPool() failed.");
        err = !noErr;
        goto bail;
    }

    if(mRecordAudio)
    {
        mWaveRecorder = new UBAudioQueueRecorder();

        if(mWaveRecorder->init(mAudioRecordingDeviceName))
        {
            connect(mWaveRecorder, SIGNAL(newWaveBuffer(void*, long, int , const AudioStreamPacketDescription*))
                                    , this, SLOT(appendAudioBuffer(void*, long, int, const AudioStreamPacketDescription*)));

            connect(mWaveRecorder, SIGNAL(audioLevelChanged(quint8)), this, SIGNAL(audioLevelChanged(quint8)));
        }
        else
        {
            setLastErrorMessage(mWaveRecorder->lastErrorMessage());
            mWaveRecorder->deleteLater();
        }
    }

    createMovie();

bail:
    ICMCompressionSessionOptionsRelease(sessionOptions);
    sessionOptions = 0;

    CFRelease(pixelBufferAttributes);

    return err == noErr;
}


void UBQuickTimeFile::stop()
{
    mSouldStopCompression = true;
}

bool UBQuickTimeFile::flushPendingFrames()
{
    mCompressionSessionRunning = false;

    if (mWaveRecorder)
    {
        mWaveRecorder->close();
        mWaveRecorder->deleteLater();
    }

    //Flush pending frames in compression session
    OSStatus err = ICMCompressionSessionCompleteFrames(mVideoCompressionSession, true, 0, 0);
    if (err)
    {
        setLastErrorMessage(QString("ICMCompressionSessionCompleteFrames() failed %1").arg(err));
        return false;
    }

    return true;
}


bool UBQuickTimeFile::closeCompressionSession()
{
    OSStatus err = noErr;

    if (mVideoMedia)
    {
        // End the media sample-adding session.
        err = EndMediaEdits(mVideoMedia);
        if (err)
        {
            setLastErrorMessage(QString("EndMediaEdits(mVideoMedia) failed %1").arg(err));
            return false;
        }

        // Make sure things are extra neat.
        ExtendMediaDecodeDurationToDisplayEndTime(mVideoMedia, 0);

        // Insert the stuff we added into the track, at the end.
        Track videoTrack = GetMediaTrack(mVideoMedia);

        err = InsertMediaIntoTrack(videoTrack,
                                       GetTrackDuration(videoTrack),
                                       0, GetMediaDisplayDuration(mVideoMedia),
                                       fixed1);
        mVideoMedia = 0;

        if (err)
        {
            setLastErrorMessage(QString("InsertMediaIntoTrack() failed %1").arg(err));
            return false;
        }

        if (mSoundMedia)
        {
            err = EndMediaEdits(mSoundMedia);
            if(err)
            {
                setLastErrorMessage(QString("EndMediaEdits(mAudioMedia) failed %1").arg(err));
                return false;
            }

            Track soundTrack = GetMediaTrack(mSoundMedia);

            err = InsertMediaIntoTrack(soundTrack,
                                           GetTrackDuration(soundTrack),
                                           0, GetMediaDisplayDuration(mSoundMedia),
                                           fixed1);

            mSoundMedia = 0;

            if (err)
            {
                setLastErrorMessage(QString("InsertMediaIntoTrack(mAudioMedia) failed %1").arg(err));
            }

            TimeValue soundTrackDuration = GetTrackDuration(soundTrack);
            TimeValue videoTrackDuration = GetTrackDuration(videoTrack);

            if (soundTrackDuration > videoTrackDuration)
            {
                qDebug() << "Sound track is longer then video track" << soundTrackDuration << ">" << videoTrackDuration;
                DeleteTrackSegment(soundTrack, videoTrackDuration, soundTrackDuration - videoTrackDuration);
            }

            DisposeHandle((Handle)mSoundDescription);
        }
    }

    // Write the movie header to the file.
    err = AddMovieToStorage(mOutputMovie, mOutputMovieDataHandler);
    if (err)
    {
        setLastErrorMessage(QString("AddMovieToStorage() failed %1").arg(err));
        return false;
    }

    err = UpdateMovieInStorage(mOutputMovie, mOutputMovieDataHandler);
    if (err)
    {
        setLastErrorMessage(QString("UpdateMovieInStorage() failed %1").arg(err));
        return false;
    }

    err = CloseMovieStorage(mOutputMovieDataHandler);
    if (err)
    {
        setLastErrorMessage(QString("CloseMovieStorage() failed %1").arg(err));
        return false;
    }
        
    CVPixelBufferPoolRelease(mCVPixelBufferPool);
    mCVPixelBufferPool = 0;

    mOutputMovie = 0;
    mOutputMovieDataHandler = 0;
    mVideoCompressionSession = 0;

    ExitMoviesOnThread();

    return true;
}


OSStatus UBQuickTimeFile::addEncodedFrameToMovie(void *encodedFrameOutputRefCon,
                                                 ICMCompressionSessionRef session,
                                                 OSStatus err,
                                                 ICMEncodedFrameRef encodedFrame,
                                                 void *reserved)
{
    Q_UNUSED(session);
    Q_UNUSED(reserved);

    UBQuickTimeFile *quickTimeFile = (UBQuickTimeFile *)encodedFrameOutputRefCon;

    if(quickTimeFile)
        quickTimeFile->addEncodedFrame(encodedFrame, err);

    return noErr;
}


void  UBQuickTimeFile::addEncodedFrame(ICMEncodedFrameRef encodedFrame, OSStatus frameErr)
{
    mPendingFrames--;

    //qDebug() << "addEncodedFrame" << mSouldStopCompression << mPendingFrames;

    if(frameErr == noErr)
    {
        if (mVideoMedia)
        {
            OSStatus err = AddMediaSampleFromEncodedFrame(mVideoMedia, encodedFrame, 0);

            if(err)
            {
                    setLastErrorMessage(QString("AddMediaSampleFromEncodedFrame() failed %1").arg(err));
            }
        }
    }
    else
    {
        setLastErrorMessage(QString("addEncodedFrame received an error %1").arg(frameErr));
    }

    if (mSouldStopCompression && mPendingFrames == 0)
    {
        closeCompressionSession();
    }
}


bool UBQuickTimeFile::createMovie()
{
    if(!mOutputMovie)
    {
        OSStatus err = noErr;

        Handle dataRef;
        OSType dataRefType;

        CFStringRef filePath = CFStringCreateWithCString(0, mVideoFileName.toUtf8().constData(), kCFStringEncodingUTF8);

        QTNewDataReferenceFromFullPathCFString(filePath, kQTPOSIXPathStyle, 0, &dataRef, &dataRefType);

        err = CreateMovieStorage(dataRef, dataRefType, 'TVOD', 0, createMovieFileDeleteCurFile, &mOutputMovieDataHandler, &mOutputMovie);
        if(err)
        {
            setLastErrorMessage(QString("CreateMovieStorage() failed %1").arg(err));
            return false;
        }

        mVideoOutputTrack = NewMovieTrack(mOutputMovie, X2Fix(mFrameSize.width()), X2Fix(mFrameSize.height()), 0);
        err = GetMoviesError();

        if( err )
        {
            setLastErrorMessage(QString("NewMovieTrack(Video) failed %1").arg(err));
            return false;
        }

        if(!createVideoMedia())
                return false;

        if(mRecordAudio)
        {
            mSoundOutputTrack = NewMovieTrack(mOutputMovie, 0, 0, kFullVolume);
            err = GetMoviesError();

            if(err)
            {
                setLastErrorMessage(QString("NewMovieTrack(Sound) failed %1").arg(err));
                return false;
            }

            if(!createAudioMedia())
                return false;
        }
    }

    return true;
}


bool UBQuickTimeFile::createVideoMedia()
{
    mVideoMedia = NewTrackMedia(mVideoOutputTrack, VideoMediaType, mTimeScale, 0, 0);
    OSStatus err = GetMoviesError();

    if (err)
    {
        setLastErrorMessage(QString("NewTrackMedia(VideoMediaType) failed %1").arg(err));
        return false;
    }

    err = BeginMediaEdits(mVideoMedia);
    if (err)
    {
        setLastErrorMessage(QString("BeginMediaEdits(VideoMediaType) failed %1").arg(err));
        return false;
    }

    return true;
}


bool UBQuickTimeFile::createAudioMedia()
{
    if(mRecordAudio)
    {
        mAudioDataFormat = UBAudioQueueRecorder::audioFormat();

        mSoundMedia = NewTrackMedia(mSoundOutputTrack, SoundMediaType, mAudioDataFormat.mSampleRate, 0, 0);
        OSStatus err = GetMoviesError();
        if(err)
        {
            setLastErrorMessage(QString("NewTrackMedia(AudioMediaType) failed %1").arg(err));
            return false;
        }

        err = BeginMediaEdits(mSoundMedia);
        if(err)
        {
            setLastErrorMessage(QString("BeginMediaEdits(AudioMediaType) failed %1").arg(err));
            return false;
        }

        err = QTSoundDescriptionCreate(&mAudioDataFormat, 0, 0, 0, 0,
                                                                        kQTSoundDescriptionKind_Movie_LowestPossibleVersion,
                                                                        &mSoundDescription);
        if (err)
        {
            setLastErrorMessage(QString("QTSoundDescriptionCreate() failed %1").arg(err));
            return false;
        }


        err = QTSoundDescriptionGetProperty(mSoundDescription, kQTPropertyClass_SoundDescription,
        kQTSoundDescriptionPropertyID_AudioStreamBasicDescription,
        sizeof(mAudioDataFormat), &mAudioDataFormat, 0);
        if (err)
        {
            setLastErrorMessage(QString("QTSoundDescriptionGetProperty() failed %1").arg(err));
            return false;
        }

    }

    return true;
}


UBQuickTimeFile::~UBQuickTimeFile()
{
    // NOOP
}


CVPixelBufferRef UBQuickTimeFile::newPixelBuffer()
{
    CVPixelBufferRef pixelBuffer = 0;

    if(CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, mCVPixelBufferPool, &pixelBuffer) != kCVReturnSuccess)
    {
        setLastErrorMessage("Could not retreive CV buffer from pool");
        return 0;
    }

    return pixelBuffer;
}


void UBQuickTimeFile::appendVideoFrame(CVPixelBufferRef pixelBuffer, long msTimeStamp)
{
    TimeValue64 msTimeStampScaled = msTimeStamp * mTimeScale / 1000;

    /*
    {
        CVPixelBufferLockBaseAddress(pixelBuffer, 0) ;
        void *pixelBufferAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
        qDebug() << "will comp newVideoFrame - PixelBuffer @" << pixelBufferAddress
         << QTime::currentTime().toString("ss:zzz") << QThread::currentThread();
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    }
    */

    OSStatus err = ICMCompressionSessionEncodeFrame(mVideoCompressionSession, pixelBuffer,
                             msTimeStampScaled, 0, kICMValidTime_DisplayTimeStampIsValid,
                             0, 0, 0);

    if (err == noErr)
    {
        mPendingFrames++;
    }
    else
    {
        setLastErrorMessage(QString("Could not encode frame %1").arg(err));
    }

    CVPixelBufferRelease(pixelBuffer);
}


void UBQuickTimeFile::appendAudioBuffer(void* pBuffer, long pLength, int inNumberPacketDescriptions, const AudioStreamPacketDescription* inPacketDescs)
{
    Q_UNUSED(pLength);
    //qDebug() << "appendAudioBuffer" << QThread::currentThread();

    if(mRecordAudio)
    {
        for (int i = 0; i < inNumberPacketDescriptions; i++)
        {
            OSStatus err = AddMediaSample2(mSoundMedia,
                                                (UInt8*)pBuffer + inPacketDescs[i].mStartOffset,
                                                inPacketDescs[i].mDataByteSize,
                                                mAudioDataFormat.mFramesPerPacket,
                                                0,
                                                (SampleDescriptionHandle)mSoundDescription,
                                                1,
                                                0,
                                                0);
            if (err)
            {
                setLastErrorMessage(QString("AddMediaSample2(soundMedia) failed %1").arg(err));
            }
        }
    }
    free((void*)inPacketDescs);
}


void UBQuickTimeFile::setLastErrorMessage(const QString& error)
{
    mLastErrorMessage = error;
    qWarning() << "UBQuickTimeFile error" << error;
}


