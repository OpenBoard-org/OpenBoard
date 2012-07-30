/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
