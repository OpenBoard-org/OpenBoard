/*
 * UBYouTubePublisher.h
 *
 *  Created on: 22 sept. 2009
 *      Author: Luc
 */

#ifndef UBYOUTUBEPUBLISHER_H_
#define UBYOUTUBEPUBLISHER_H_

#include <QtGui>

#include "ui_youTubePublishingDialog.h"

class UBServerXMLHttpRequest;

class UBYouTubePublisher : public QObject
{
    Q_OBJECT;

    public:
        UBYouTubePublisher(QObject* pParent = 0);
        virtual ~UBYouTubePublisher();

    public slots:

        void uploadVideo(const QString& videoFilePath);

    signals:

        void postClientLoginRequestFinished(bool success, const QString& authToken);

    private slots:

         void postClientLoginRequest(const QString& userName, const QString& password);
         void postClientLoginResponse(bool success, const QByteArray& pPayload);

         void postVideoUploadRequest();
         void postVideoUploadResponse(bool success, const QByteArray& pPayload);
         void progress(qint64 bytesSent, qint64 bytesTotal);

    private:

        QString youtubeMetadata();

        static const QString sYouTubeDeveloperKey;

        QString mVideoFilePath;
        QString mAuthToken;
        QString mUserName;

        QString mTitle;
        QString mDescription;
        QString mKeywords;
        QStringList mCategories;

        UBServerXMLHttpRequest *mAuthRequest;
        UBServerXMLHttpRequest *mUploadRequest;

};


class UBYouTubePublishingDialog : public QDialog, public Ui::YouTubePublishingDialog
{
    Q_OBJECT;

    public:
        UBYouTubePublishingDialog(const QString& videoFilePath, QWidget *parent = 0);
        ~UBYouTubePublishingDialog(){};

    private:
        QMap<QString, QString> categories();

    private slots:
        void updateUIState(const QString& = QString(""));

};

#endif /* UBYOUTUBEPUBLISHER_H_ */
