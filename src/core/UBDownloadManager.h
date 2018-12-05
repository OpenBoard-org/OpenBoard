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




#ifndef UBDOWNLOADMANAGER_H
#define UBDOWNLOADMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>
#include <QDropEvent>

#include "UBDownloadThread.h"

#include "network/UBHttpGet.h"

#define     SIMULTANEOUS_DOWNLOAD       2   // Maximum 5 because of QNetworkAccessManager limitation!!!

struct sDownloadFileDesc
{
    enum eDestinations {
        board //default for sDownloadFileDesc
        , library
        , graphicsWidget
    };
    //creating constructor to make sure to have default values
    sDownloadFileDesc() :
        dest(board)
      , id(0)
      , totalSize(0)
      , currentSize(0)
      , modal(false)
      , isBackground(false)
      , dropActions(Qt::IgnoreAction)
      , dropMouseButtons(Qt::NoButton)
      , dropModifiers(Qt::NoModifier)
    {;}

    eDestinations dest;
    QString name;
    int id;
    int totalSize;
    int currentSize;
    QString srcUrl;
    QString originalSrcUrl;
    QString contentTypeHeader;
    bool modal;
    QPointF pos;        // For board drop only
    QSize size;         // For board drop only
    bool isBackground;  // For board drop only

    QPoint dropPoint;    //For widget's Drop event
    Qt::DropActions dropActions; //For widget's Drop event
    Qt::MouseButtons dropMouseButtons; //For widget's Drop event
    Qt::KeyboardModifiers dropModifiers; //For widget's Drop event
};


class UBDownloadHttpFile : public UBHttpGet
{
    Q_OBJECT
public:
    UBDownloadHttpFile(int fileId, QObject* parent=0);
    ~UBDownloadHttpFile();

signals:
    void downloadProgress(int id, qint64 current,qint64 total);
    void downloadFinished(int id, bool pSuccess, QUrl sourceUrl, QUrl contentUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void downloadError(int id);

private slots:
    void onDownloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    int mId;
};

class UBAsyncLocalFileDownloader : public QThread
{
    Q_OBJECT
public:
    UBAsyncLocalFileDownloader(sDownloadFileDesc desc, QObject *parent = 0);

    UBAsyncLocalFileDownloader *download();    
    void run();
    void abort();

signals:
    void finished(QString srcUrl, QString resUrl);
    void signal_asyncCopyFinished(int id, bool pSuccess, QUrl sourceUrl, QUrl contentUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);


private:
    sDownloadFileDesc mDesc;
    bool m_bAborting;
    QString mFrom;
    QString mTo;
};

class UBDownloadManager : public QObject
{
    Q_OBJECT
public:
    UBDownloadManager(QObject* parent=0, const char* name="UBDownloadManager");
    ~UBDownloadManager();
    static UBDownloadManager* downloadManager();
    int addFileToDownload(sDownloadFileDesc desc);
    QVector<sDownloadFileDesc> currentDownloads();
    QVector<sDownloadFileDesc> pendingDownloads();
    void cancelDownloads();
    void cancelDownload(int id);

    static void destroy();

signals:
    void fileAddedToDownload();
    void downloadUpdated(int id, qint64 crnt, qint64 total);
    void downloadFinished(int id);
    void downloadFinished(bool pSuccess, int id, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData);
    void downloadFinished(bool pSuccess, sDownloadFileDesc desc, QByteArray pData);
    void downloadModalFinished();
    void addDownloadedFileToBoard(bool pSuccess, QUrl sourceUrl, QUrl contentUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void addDownloadedFileToLibrary(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QString pTitle);
    void cancelAllDownloads();
    void allDownloadsFinished();

private slots:
    void onUpdateDownloadLists();
    void onDownloadProgress(int id, qint64 received, qint64 total);
    void onDownloadFinished(int id, bool pSuccess, QUrl sourceUrl, QUrl contentUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void onDownloadError(int id);

private:
    void init();
    void updateDownloadOrder();
    void updateFileCurrentSize(int id, qint64 received=-1, qint64 total=-1);
    void startFileDownload(sDownloadFileDesc desc);
    void checkIfModalRemains();
    void finishDownloads(bool cancel=false);

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
    QMap<int, QObject*> mDownloads;
};

#endif // UBDOWNLOADMANAGER_H
