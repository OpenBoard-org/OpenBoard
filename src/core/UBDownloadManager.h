/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
#ifndef UBDOWNLOADMANAGER_H
#define UBDOWNLOADMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>

#include "UBDownloadThread.h"

#include "network/UBHttpGet.h"

#define     SIMULTANEOUS_DOWNLOAD       2   // Maximum 5 because of QNetworkAccessManager limitation!!!

typedef struct
{
    QString name;
    int id;
    int totalSize;
    int currentSize;
    QString url;
    bool modal;
    QPointF pos;        // For board drop only
    QSize size;         // For board drop only
    bool isBackground;  // For board drop only
}sDownloadFileDesc;

class UBDownloadHttpFile : public UBHttpGet
{
    Q_OBJECT
public:
    UBDownloadHttpFile(int fileId, QObject* parent=0);
    ~UBDownloadHttpFile();

signals:
    void downloadProgress(int id, qint64 current,qint64 total);
    void downloadFinished(int id, bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void downloadError(int id);

private slots:
    void onDownloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    int mId;
};

class UBDownloadManager : public QObject
{
    Q_OBJECT
public:
    UBDownloadManager(QObject* parent=0, const char* name="UBDownloadManager");
    ~UBDownloadManager();

    static UBDownloadManager* downloadManager();
    void addFileToDownload(sDownloadFileDesc desc);
    QVector<sDownloadFileDesc> currentDownloads();
    QVector<sDownloadFileDesc> pendingDownloads();
    void cancelDownloads();

signals:
    void fileAddedToDownload();
    void downloadUpdated(int id, qint64 crnt, qint64 total);
    void downloadFinished(int id);
    void downloadModalFinished();
    void addDownloadedFileToBoard(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void cancelAllDownloads();

private slots:
    void onUpdateDownloadLists();
    void onDownloadProgress(int id, qint64 received, qint64 total);
    void onDownloadFinished(int id, bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void onDownloadError(int id);

private:
    void init();
    void updateDownloadOrder();
    void updateFileCurrentSize(int id, qint64 received=-1, qint64 total=-1);
    void startFileDownload(sDownloadFileDesc desc);
    void checkIfModalRemains();

    /** The current downloads */
    QVector<sDownloadFileDesc> mCrntDL;
    /** The pending downloads */
    QVector<sDownloadFileDesc> mPendingDL;
    /** Pending download mutex */
    QMutex mMutex;
    /** The last file ID */
    int mLastID;
    /** The current download availability (-1 = free, otherwise the file ID is recorded)*/
    QVector<int> mDLAvailability;
    /** A map containing the replies of the GET operations */
    QMap<int, QNetworkReply*> mReplies;
};

#endif // UBDOWNLOADMANAGER_H
