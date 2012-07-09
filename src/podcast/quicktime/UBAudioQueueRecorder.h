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

#ifndef UBAUDIOQUEUERECORDER_H_
#define UBAUDIOQUEUERECORDER_H_

#include <QtCore>

#include <ApplicationServices/ApplicationServices.h>
#include <AudioToolbox/AudioToolbox.h>

class UBAudioQueueRecorder : public QObject
{
    Q_OBJECT;

    public:
        UBAudioQueueRecorder(QObject* pObject = 0);
        virtual ~UBAudioQueueRecorder();

        bool init(const QString& waveInDeviceName = QString(""));
        bool close();

        static QStringList waveInDevices();

        static AudioDeviceID deviceIDFromDeviceName(const QString& name);
        static QString deviceNameFromDeviceID(AudioDeviceID id);
        static QString deviceUIDFromDeviceID(AudioDeviceID id);
        static QList<AudioDeviceID> inputDeviceIDs();
        static AudioDeviceID defaultInputDeviceID();

        QString lastErrorMessage()
        {
            return mLastErrorMessage;
        }

        static AudioStreamBasicDescription audioFormat()
        {
            return sAudioFormat;
        }

    signals:

        void newWaveBuffer(void* pBuffer, long pLength, int inNumberPacketDescriptions, const AudioStreamPacketDescription *inPacketDescs);

        void audioLevelChanged(quint8 level);

    private:
        static void audioQueueInputCallback (void *inUserData, AudioQueueRef inAQ,
            AudioQueueBufferRef inBuffer, const AudioTimeStamp *inStartTime,
            UInt32 inNumberPacketDescriptions, const AudioStreamPacketDescription *inPacketDescs);

        void emitNewWaveBuffer(AudioQueueBufferRef pBuffer, int inNumberPacketDescriptions, const AudioStreamPacketDescription *inPacketDescs);

        void emitAudioLevelChanged(quint8 level);

        void setLastErrorMessage(const QString& pLastErrorMessage)
        {
            mLastErrorMessage = pLastErrorMessage;
            qWarning() << "UBAudioQueueRecorder error:" << mLastErrorMessage;
        }

        bool isRecording() const
        {
            return mIsRecording;
        }

        AudioQueueRef mQueue;
        QList<AudioQueueBufferRef> mBuffers;
        QString mLastErrorMessage;
        volatile bool mIsRecording;
        int mBufferLengthInMs;
        volatile long mMsTimeStamp;
        UInt32 mSampleBufferSize;

        static AudioStreamBasicDescription sAudioFormat;
};

#endif /* UBAUDIOQUEUERECORDER_H_ */
