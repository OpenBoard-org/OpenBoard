/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
