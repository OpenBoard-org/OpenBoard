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
        void updatePersistanceEnableState();

    private slots:
        void updateUIState(const QString& = QString(""));
        void updateCredentialPersistenceState();

};

#endif /* UBYOUTUBEPUBLISHER_H_ */
