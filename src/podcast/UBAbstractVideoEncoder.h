/*
 * UBAbstractVideoEncoder.h
 *
 *  Created on: 3 sept. 2009
 *      Author: Luc
 */

#ifndef UBABSTRACTVIDEOENCODER_H_
#define UBABSTRACTVIDEOENCODER_H_

#include <QtCore>

class UBAbstractVideoEncoder : public QObject
{
    Q_OBJECT;

    public:
        UBAbstractVideoEncoder(QObject *pParent = 0);
        virtual ~UBAbstractVideoEncoder();

        virtual bool start() = 0;

        virtual bool stop() = 0;

        virtual bool pause() { return false;}

        virtual bool unpause() { return false;}

        virtual bool canPause() { return false;}

        virtual void newPixmap(const QImage& pImage, long timestamp) = 0;

        virtual void newChapter(const QString& pLabel, long timestamp);

        void setFramesPerSecond(int pFps)
        {
            mFramesPerSecond = pFps;
        }

        int framesPerSecond() const
        {
            return mFramesPerSecond;
        }

        virtual QString videoFileExtension() const = 0;

        virtual void setVideoFileName(const QString& pFileName)
        {
            mVideoFileName = pFileName;
        }

        virtual QString videoFileName()
        {
            return mVideoFileName;
        }

        virtual void setVideoSize(const QSize& pSize)
        {
            mVideoSize = pSize;
        }

        virtual QSize videoSize() const
        {
            return mVideoSize;
        }

        virtual long videoBitsPerSecond() const
        {
            return mVideoBitsPerSecond;
        }

        virtual void setVideoBitsPerSecond(long pVideoBitsPerSecond)
        {
            mVideoBitsPerSecond = pVideoBitsPerSecond;
        }

        virtual QString lastErrorMessage() = 0;

        virtual void setAudioRecordingDevice(const QString pAudioRecordingDevice)
        {
            mAudioRecordingDevice = pAudioRecordingDevice;
        }

        virtual QString audioRecordingDevice()
        {
            return mAudioRecordingDevice;
        }

        virtual void setRecordAudio(bool pRecordAudio) = 0;

    signals:

        void encodingStatus(const QString& pStatus);

        void encodingFinished(bool ok);

        void audioLevelChanged(quint8 level);

    private:

        int mFramesPerSecond;

        QString mVideoFileName;

        QSize mVideoSize;

        long mVideoBitsPerSecond;

        QString mAudioRecordingDevice;

};

#endif /* UBABSTRACTVIDEOENCODER_H_ */
