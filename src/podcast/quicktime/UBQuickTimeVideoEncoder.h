/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
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
