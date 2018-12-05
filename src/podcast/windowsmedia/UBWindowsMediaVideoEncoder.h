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
