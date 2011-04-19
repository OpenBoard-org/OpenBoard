/*
 * UBQuickTimeVideoEncoder.h
 *
 *  Created on: 10 sept. 2009
 *      Author: Luc
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
