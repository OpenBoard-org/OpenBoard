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




#ifndef UBQUICKTIMEFILE_H_
#define UBQUICKTIMEFILE_H_

#include <QtCore>

#include <CoreVideo/CoreVideo.h>
#include <CoreMedia/CoreMedia.h>

#include "UBAudioQueueRecorder.h"



// Trick to get around the fact that the C++ compiler doesn't
// like Objective C code.

#ifdef __OBJC__ // defined by the Objective C compiler
    @class AVAssetWriter;
    @class AVAssetWriterInput;
    @class AVAssetWriterInputPixelBufferAdaptor;

    typedef AVAssetWriter* AssetWriterPTR;
    typedef AVAssetWriterInput* AssetWriterInputPTR;
    typedef AVAssetWriterInputPixelBufferAdaptor* AssetWriterInputAdaptorPTR;
#else
    typedef void* AssetWriterPTR;
    typedef void* AssetWriterInputPTR;
    typedef void* AssetWriterInputAdaptorPTR;
#endif

class UBQuickTimeFile : public QThread
{
    Q_OBJECT;

    public:
        struct VideoFrame
        {
            CVPixelBufferRef buffer;
            long timestamp;
        };

        static QWaitCondition frameBufferNotEmpty;

        UBQuickTimeFile(QObject * pParent = 0);
        virtual ~UBQuickTimeFile();

        bool init(const QString& videoFileName, const QString& profileData
                , int pFramesPerSecond, const QSize& pFrameSize
                , bool recordAudio = true, const QString& audioDeviceName = QString("Default"));

        void stop();

        CVPixelBufferRef newPixelBuffer();
        void enqueueVideoFrame(VideoFrame frame);

        bool isCompressionSessionRunning() { return mCompressionSessionRunning; }
        QString lastErrorMessage() const { return mLastErrorMessage; }

    signals:
        void audioLevelChanged(quint8 level);
        void compressionSessionStarted();
        void compressionFinished();

    protected:
        void run();

    private slots:
        void enqueueAudioBuffer(void* pBuffer, long pLength);

    private:
        QString mLastErrorMessage;

        // Format information
        QString mVideoFileName;
        QSize mFrameSize;
        long mTimeScale;
        bool mRecordAudio;
        QString mAudioRecordingDeviceName;

        // Video/audio encoders and associated objects
        AssetWriterPTR mVideoWriter;
        AssetWriterInputPTR mVideoWriterInput;
        AssetWriterInputAdaptorPTR mAdaptor;
        AssetWriterInputPTR mAudioWriterInput;

        // Audio recorder
        QPointer<UBAudioQueueRecorder> mWaveRecorder;
        CMAudioFormatDescriptionRef mAudioFormatDescription;

        // Variables used during encoding
        CFAbsoluteTime mStartTime;
        CMTime mLastFrameTimestamp;

        volatile bool mShouldStopCompression;
        volatile bool mCompressionSessionRunning;

        // Dispatch queues to handle passing data to the A/V encoders
        dispatch_queue_t mVideoDispatchQueue;
        dispatch_queue_t mAudioDispatchQueue;

        // Queues for frames and audio buffers to be encoded
        QQueue<VideoFrame> frameQueue;
        QQueue<CMSampleBufferRef> audioQueue;

        QMutex frameQueueMutex;
        QMutex audioQueueMutex;
        QMutex audioWriterMutex;


        // Private functions

        void setLastErrorMessage(const QString& error);

        bool beginSession();
        void endSession();

        void appendFrameToVideo(CVPixelBufferRef pixelBuffer, long msTimeStamp);
        bool appendSampleBuffer(CMSampleBufferRef sampleBuffer);

};

#endif /* UBQUICKTIMEFILE_H_ */
