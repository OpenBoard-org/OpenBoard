/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBHTTPGET_H_
#define UBHTTPGET_H_

#include <QtCore>
#include <QtNetwork>
#include <QDropEvent>

class UBHttpGet : public QObject
{

    Q_OBJECT

    public:
        UBHttpGet(QObject* parent = 0);
        virtual ~UBHttpGet();

        QNetworkReply* get(QUrl pUrl, QPointF pPoint = QPointF(0, 0), QSize pSize = QSize(0, 0), bool isBackground = false);
//        QNetworkReply* get(const sDownloadFileDesc &downlinfo);

    signals:

        void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader
                , QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
//        void downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData
//                              , sDownloadFileDesc downlInfo);

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
//        sDownloadFileDesc mDownloadInfo;
};

#endif /* UBHTTPGET_H_ */

