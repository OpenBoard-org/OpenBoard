/*
 * UBServerXMLHttpRequest.h
 *
 *  Created on: 1 mai 2009
 *      Author: Luc
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
