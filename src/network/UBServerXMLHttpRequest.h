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
