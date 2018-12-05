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




#ifndef UBSERVERXMLHTTPREQUEST_H_
#define UBSERVERXMLHTTPREQUEST_H_

#include <QtCore>
#include <QtNetwork>

class UBNetworkAccessManager;

class UBServerXMLHttpRequest : public QObject
{
    Q_OBJECT;

    public:
        UBServerXMLHttpRequest(UBNetworkAccessManager* pNam, const QString& contentType = "application/xml");

        virtual ~UBServerXMLHttpRequest();

        void post(const QUrl& pUrl, const QByteArray& payload);
        void put(const QUrl& pUrl, const QByteArray& payload);
        void get(const QUrl& pUrl);

        void syntheticRubyOnRailsDelete(const QUrl& pUrl);

        void addHeader(const QString&, const QString&);

        void setVerbose(bool pVerbose);

   private:
        void connectReply();

   private slots:

        void downloadProgress (qint64 bytesReceived, qint64 bytesTotal);
        void error (QNetworkReply::NetworkError code);
        void finished ();
        void uploadProgress ( qint64 bytesSent, qint64 bytesTotal );
        void readyRead();

    signals:

        void finished(bool, const QByteArray&);
        void progress(qint64 bytesSent, qint64 bytesTotal);

    private:

        bool mHasError;
        bool mIsRunning;

        QString mContentType;
        QNetworkReply* mReply;

        QByteArray mReplyContent;

        QMap<QString, QString> mExtraHeaders;

        UBNetworkAccessManager* mNam;

        bool mIsVerbose;

        int mRedirectionCount;

        QByteArray mPutPayload;
        QByteArray mPostPayload;

};


#endif /* UBSERVERXMLHTTPREQUEST_H_ */
