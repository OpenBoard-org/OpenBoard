/*
 * UBHttpFileDownloader.h
 *
 *  Created on: 28 avr. 2009
 *      Author: Luc
 */

#ifndef UBHTTPFILEDOWNLOADER_H_
#define UBHTTPFILEDOWNLOADER_H_

#include <QtCore>
#include <QtNetwork>

class UBHttpFileDownloader : public QObject
{
        Q_OBJECT;

    public:
        UBHttpFileDownloader(QObject *parent = 0);

        virtual ~UBHttpFileDownloader();

        void download(const QList<QUrl>& urls, const QList<QFile*>& files);

    signals:

        void finished(bool success);

    private:

        void downloadNext();

        QList<QUrl> mUrlsToDownload;
        QList<QFile*> mFilesToSave;

        QNetworkReply* mReply;
        QFile* mCurrentFile;
        bool mSuccess;

    private slots:

        void downloadProgress (qint64 bytesReceived, qint64 bytesTotal);

        void error ( QNetworkReply::NetworkError code );

        void finished ();

        void readyRead();

};

#endif /* UBHTTPFILEDOWNLOADER_H_ */
