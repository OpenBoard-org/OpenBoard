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




#ifndef UBQUICKTIMEVIDEOENCODER_H_
#define UBQUICKTIMEVIDEOENCODER_H_


#include <QtGui>
#include "podcast/UBAbstractVideoEncoder.h"


#include "UBQuickTimeFile.h"
#include "UBAudioQueueRecorder.h"


class UBQuickTimeVideoEncoder : public UBAbstractVideoEncoder
{
    Q_OBJECT;

    public:
        UBQuickTimeVideoEncoder(QObject* pParent = 0);

        virtual ~UBQuickTimeVideoEncoder();

        virtual bool start();
        virtual bool stop();

        virtual void newPixmap(const QImage& pImage, long timestamp);

        virtual QString videoFileExtension() const
        {
            return "mov";
        }

        void setLastErrorMessage(const QString& pMessage)
        {
            qDebug() << "UBQuickTimeVideoEncoder :" << pMessage;
            mLastErrorMessage = pMessage;
        }

        virtual QString lastErrorMessage()
        {
            return mLastErrorMessage;
        }

        virtual void setRecordAudio(bool pRecordAudio);

    private slots:

       void compressionFinished();

    private:

        struct ImageFrame
        {
           QImage image;
           long timestamp;
        };

        void encodeFrame(const QImage& image, long timestamp);

        QList<ImageFrame> mPendingImageFrames;

        UBQuickTimeFile mQuickTimeCompressionSession;

        QString mLastErrorMessage;

        bool mShouldRecordAudio;

};

#endif /* UBQUICKTIMEVIDEOENCODER_H_ */
