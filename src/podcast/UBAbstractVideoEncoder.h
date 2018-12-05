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
