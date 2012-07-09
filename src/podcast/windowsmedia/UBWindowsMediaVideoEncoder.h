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
