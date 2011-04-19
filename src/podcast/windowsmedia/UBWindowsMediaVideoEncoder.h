/*
 * UBWindowsMediaVideoEncoder.h
 *
 *  Created on: 7 sept. 2009
 *      Author: Luc
 */

#ifndef UBWINDOWSMEDIAVIDEOENCODER_H_
#define UBWINDOWSMEDIAVIDEOENCODER_H_


#include <QtGui>
#include "podcast/UBAbstractVideoEncoder.h"

#include "UBWindowsMediaFile.h"
#include "UBWaveRecorder.h"


class UBWindowsMediaVideoEncoder : public UBAbstractVideoEncoder
{
    Q_OBJECT;

    public:
        UBWindowsMediaVideoEncoder(QObject* pParent = 0);

        virtual ~UBWindowsMediaVideoEncoder();

        virtual bool start();
        virtual bool pause();
        virtual bool unpause();
        virtual bool stop();

        virtual bool canPause() { return true;};

        virtual void newPixmap(const QImage& pPix, long timestamp);
        virtual void newChapter(const QString& pLabel, long timestamp);

        virtual QString videoFileExtension() const
        {
            return "wmv";
        }

        virtual QString lastErrorMessage()
        {
            return mLastErrorMessage;
        }

        virtual void setRecordAudio(bool pRecordAudio);

    private slots:

        void processAudioBuffer(WAVEHDR*, long);

    private:
        QPointer<UBWindowsMediaFile> mWMVideo;
        QPointer<UBWaveRecorder> mWaveRecorder;

        QString mLastErrorMessage;

        bool mRecordAudio;
        bool mIsPaused;

        quint8 mLastAudioLevel;
        bool mIsRecording;

};

#endif /* UBWINDOWSMEDIAVIDEOENCODER_H_ */
