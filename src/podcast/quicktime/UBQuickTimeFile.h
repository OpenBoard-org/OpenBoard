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




#ifndef UBQUICKTIMEFILE_H_
#define UBQUICKTIMEFILE_H_

#include <QtCore>

#include <ApplicationServices/ApplicationServices.h>
#include <QuickTime/QuickTime.h>
#include <AudioToolbox/AudioToolbox.h>

#include "UBAudioQueueRecorder.h"

class UBQuickTimeFile : public QThread
{
    Q_OBJECT;

    public:
        UBQuickTimeFile(QObject * pParent = 0);
        virtual ~UBQuickTimeFile();

        bool init(const QString& videoFileName, const QString& profileData
                , int pFramesPerSecond, const QSize& pFrameSize
                , bool recordAudio = true, const QString& audioDeviceName = QString("Default"));

        void stop();

        CVPixelBufferRef newPixelBuffer();

        bool isCompressionSessionRunning()
        {
            return mCompressionSessionRunning;
        }

        QString lastErrorMessage() const
        {
            return mLastErrorMessage;
        }

        struct VideoFrame
        {
            CVPixelBufferRef buffer;
            long timestamp;
        };

        static QQueue<VideoFrame> frameQueue;
        static QMutex frameQueueMutex;
        static QWaitCondition frameBufferNotEmpty;

    signals:
        void audioLevelChanged(quint8 level);
        void compressionSessionStarted();

    protected:
        void run();

    private slots:

        void appendAudioBuffer(void* pBuffer, long pLength, int inNumberPacketDescriptions
                        , const AudioStreamPacketDescription* inPacketDescs);

    private:

        static OSStatus addEncodedFrameToMovie(void *encodedFrameOutputRefCon,
                                                 ICMCompressionSessionRef session,
                                                 OSStatus err,
                                                 ICMEncodedFrameRef encodedFrame,
                                                 void *reserved);

        void appendVideoFrame(CVPixelBufferRef pixelBuffer, long msTimeStamp);

        void addEncodedFrame(ICMEncodedFrameRef encodedFrame, OSStatus err);

        bool createCompressionSession();
        bool closeCompressionSession();
        bool createMovie();

        bool createVideoMedia();
        bool createAudioMedia();

        void setLastErrorMessage(const QString& error);

        bool flushPendingFrames();

        ICMCompressionSessionRef mVideoCompressionSession;

        Media mVideoMedia;
        Media mSoundMedia;
        Track mVideoOutputTrack;
        Track mSoundOutputTrack;

        volatile CVPixelBufferPoolRef mCVPixelBufferPool;

        SoundDescriptionHandle mSoundDescription;

        Movie mOutputMovie;
        DataHandler mOutputMovieDataHandler;

        int mFramesPerSecond;
        QSize mFrameSize;
        QString mVideoFileName;
        long mTimeScale;

        bool mRecordAudio;

        QString mLastErrorMessage;

        AudioStreamBasicDescription mAudioDataFormat;

        QPointer<UBAudioQueueRecorder> mWaveRecorder;

        CodecQ mSpatialQuality;

        volatile bool mSouldStopCompression;
        volatile bool mCompressionSessionRunning;

        QString mAudioRecordingDeviceName;
        volatile int mPendingFrames;
};

#endif /* UBQUICKTIMEFILE_H_ */
