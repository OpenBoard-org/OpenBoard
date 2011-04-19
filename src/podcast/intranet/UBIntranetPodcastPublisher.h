/*
 * UBUnilPublisher.h
 *
 *  Created on: 23 avr. 2010
 *      Author: Luc
 */

#ifndef UBUNILPUBLISHER_H_
#define UBUNILPUBLISHER_H_

#include <QtGui>

#include "ui_intranetPodcastPublishingDialog.h"

class UBServerXMLHttpRequest;

class UBIntranetPodcastPublisher : public QObject
{
    Q_OBJECT;

    public:
        UBIntranetPodcastPublisher(QObject* pParent = 0);
        virtual ~UBIntranetPodcastPublisher();

    public slots:

        void publishVideo(const QString& videoFilePath, long durationInMs);

        void postVideoPublishingRequest();
        void postVideoPublishingResponse(bool success, const QByteArray& pPayload);
        void progress(qint64 bytesSent, qint64 bytesTotal);

    private:

        QString metadata();

        QString mVideoFilePath;
        QString mTitle;
        QString mDescription;
        QString mAuthor;
        long mVideoDurationInMs;

        UBServerXMLHttpRequest *mPublishingRequest;

};


class UBIntranetPodcastPublishingDialog : public QDialog, public Ui::IntranetPodcastPublishingDialog
{
    Q_OBJECT;

    public:
        UBIntranetPodcastPublishingDialog(const QString& videoFilePath, QWidget *parent = 0);
        ~UBIntranetPodcastPublishingDialog(){};

    private slots:
        void updateUIState(const QString& = QString(""));

};

#endif /* UBUNILPUBLISHER_H_ */
