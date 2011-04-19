/*
 * UBWaveRecorder.h
 *
 *  Created on: 9 sept. 2009
 *      Author: Luc
 */

#ifndef UBWAVERECORDER_H_
#define UBWAVERECORDER_H_

#include <QtCore>

#include "Windows.h"

class UBWaveRecorder : public QObject
{
    Q_OBJECT;

    public:
        UBWaveRecorder(QObject * pParent = 0);
        virtual ~UBWaveRecorder();

        bool init(const QString& waveInDeviceName = QString(""));

        bool start();
        bool stop();

        bool close();

        QString lastErrorMessage()
        {
            return mLastErrorMessage;
        }

        long msTimeStamp(){
            mMsTimeStamp;
        }

        static QStringList waveInDevices();

    signals:

        void newWaveBuffer(WAVEHDR *buffer, long);

    private:

        static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

        void emitNewWaveBuffer(WAVEHDR * pBuffer);

        void setLastErrorMessage(QString pLastErrorMessage)
        {
            mLastErrorMessage = pLastErrorMessage;
            qWarning() << mLastErrorMessage;
        }

        bool isRecording() const
        {
            return mIsRecording;
        }

        QString mLastErrorMessage;

        HWAVEIN mWaveInDevice;

        QList<WAVEHDR*>  mWaveBuffers;

        volatile bool mIsRecording;
        volatile long mMsTimeStamp;

        int mBufferLengthInMs;

        QTime mRecordingStartTime;

        int mNbChannels;
        int mSampleRate;
        int mBitsPerSample;

};

#endif /* UBWAVERECORDER_H_ */
