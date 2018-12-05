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
