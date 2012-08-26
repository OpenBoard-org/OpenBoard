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


#include "UBHttpFileDownloader.h"

#include "network/UBNetworkAccessManager.h"

#include "core/memcheck.h"

UBHttpFileDownloader::UBHttpFileDownloader(QObject *parent)
    : QObject(parent)
{
        // NOOP
}


UBHttpFileDownloader::~UBHttpFileDownloader()
{
        // NOOP
}


void UBHttpFileDownloader::download(const QList<QUrl>& urls, const QList<QFile*>& files)
{
        mUrlsToDownload = urls;
        mFilesToSave = files;

        mSuccess = true;

        downloadNext();
}


void UBHttpFileDownloader::downloadNext()
{
        if (mUrlsToDownload.length() > 0 && mFilesToSave.length() > 0)
        {
                QUrl url = mUrlsToDownload.takeFirst();
                mCurrentFile = mFilesToSave.takeFirst();

        if (!mCurrentFile->open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qCritical() << "cannot open " << mCurrentFile << " for writing ...";
            return;
        }

                UBNetworkAccessManager * nam = UBNetworkAccessManager::defaultAccessManager();
                QNetworkRequest request(url);
            mReply = nam->get(request); // UB 4.2 TODO who owns and delete the reply ?

            connect(mReply, SIGNAL(finished ()), this, SLOT(finished ()));
                connect(mReply, SIGNAL(error ( QNetworkReply::NetworkError )),
                                this, SLOT(error ( QNetworkReply::NetworkError)));
                connect(mReply, SIGNAL(downloadProgress ( qint64 , qint64  )),
                                this, SLOT(downloadProgress(qint64 , qint64)));

                connect(mReply, SIGNAL(readyRead ()), this, SLOT( readyRead()));

        }
        else
        {
                emit finished(mSuccess);
        }

}


void UBHttpFileDownloader::downloadProgress (qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);
        //qDebug() << bytesReceived << "/" << bytesTotal;
}


void UBHttpFileDownloader::error ( QNetworkReply::NetworkError code )
{
    qDebug() << "UBHttpFileDownloader::error : code=" << code;
    mSuccess = mSuccess && (code == QNetworkReply::NoError);

}

void UBHttpFileDownloader::finished ()
{
        mCurrentFile->flush();
        mCurrentFile->close();

        if (mReply)
                delete mReply;

        downloadNext();
}



void UBHttpFileDownloader::readyRead()
{
        mCurrentFile->write(mReply->readAll());
}
