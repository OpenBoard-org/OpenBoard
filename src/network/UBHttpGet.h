/*
 * UBHttpGet.h
 *
 *  Created on: Sep 19, 2008
 *      Author: luc
 */

#ifndef UBHTTPGET_H_
#define UBHTTPGET_H_

#include <QtCore>
#include <QtNetwork>


class UBHttpGet : public QObject
{

    Q_OBJECT;

    public:
        UBHttpGet(QObject* parent = 0);
        virtual ~UBHttpGet();

        void get(QUrl pUrl, QPointF pPoint = QPointF(0, 0), QSize pSize = QSize(0, 0), bool isBackground = false);

    signals:

        void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader
                , QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);

    private slots:

        void readyRead();
        void requestFinished();
        void downloadProgressed(qint64 bytesReceived, qint64 bytesTotal);

    private:

        QByteArray mDownloadedBytes;
        QNetworkReply* mReply;
        QPointF mPos;
        QSize mSize;

        bool mIsBackground;
        int mRequestID;
        int mRedirectionCount;
        bool mIsSelfAborting;
};

#endif /* UBHTTPGET_H_ */

