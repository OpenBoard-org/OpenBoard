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




#include "UBServerXMLHttpRequest.h"

#include <QtCore>
#include <QtNetwork>

#include "network/UBNetworkAccessManager.h"

#include "core/memcheck.h"

UBServerXMLHttpRequest::UBServerXMLHttpRequest(UBNetworkAccessManager* pNam, const QString& contentType)
    : QObject(pNam)
    , mHasError(false)
    , mIsRunning(false)
    , mContentType(contentType)
    , mReply(0)
    , mNam(pNam)
    , mIsVerbose(false)
    , mRedirectionCount(0)
{
    // NOOP
}


UBServerXMLHttpRequest::~UBServerXMLHttpRequest()
{
    delete mReply;
}


void UBServerXMLHttpRequest::addHeader(const QString& key, const QString& value)
{
    mExtraHeaders.insert(key, value);
}


void UBServerXMLHttpRequest::post(const QUrl& pUrl, const QByteArray& payload)
{
    if (mIsRunning)
    {
        qDebug() << this << "already running ...";
        return;
    }

    mIsRunning = true;

    mReplyContent.clear();
    mHasError = false;

    QNetworkRequest request(pUrl);

    request.setHeader(QNetworkRequest::ContentTypeHeader, mContentType);
    //request.setRawHeader("ACCEPT", "application/xml");
    request.setHeader(QNetworkRequest::ContentLengthHeader, payload.length());

    foreach(QString key, mExtraHeaders.keys())
    {
        request.setRawHeader(key.toUtf8(), mExtraHeaders.value(key).toUtf8());

        if (mIsVerbose)
            qDebug() << (key  + ":" + mExtraHeaders.value(key));
    }

    if (mIsVerbose)
    {
        qDebug() << "posting" << payload.length() / 1024 << "KB of data to " << pUrl;
        qDebug() << "posting" << payload.length() << "B of data to " << pUrl;
        qDebug() << "posting -------------------------------------\n";
        qDebug() << QString::fromUtf8(payload);
    }

    mPostPayload = payload;

    mReply = mNam->post(request, payload); // UB 4.2 TODO who owns and delete the reply ?

    if (mIsVerbose)
    {
        qDebug() << "posted --------------------------------------\n";
    }

    connectReply();
}


void UBServerXMLHttpRequest::put(const QUrl& pUrl, const QByteArray& payload)
{
    if (mIsRunning)
    {
        qDebug() << this << "already running ...";
        return;
    }

    mIsRunning = true;

    mReplyContent.clear();
    mHasError = false;

    QNetworkRequest request(pUrl);

    request.setHeader(QNetworkRequest::ContentTypeHeader, mContentType);
    request.setRawHeader("ACCEPT", "application/xml");
    request.setHeader(QNetworkRequest::ContentLengthHeader, payload.length());

    foreach(QString key, mExtraHeaders.keys())
    {
        request.setRawHeader(key.toUtf8(), mExtraHeaders.value(key).toUtf8());
    }

    if (mIsVerbose)
        qDebug() << "putting" << payload.length() / 1024 << "KB of data to " << pUrl;

    mPutPayload = payload;

    mReply = mNam->put(request, payload); // UB 4.2 TODO who owns and delete the reply ?

    connectReply();
}


void UBServerXMLHttpRequest::get(const QUrl& pUrl)
{
    if (mIsRunning)
    {
        qDebug() << this << "already running ...";
        return;
    }

    mIsRunning = true;

    mReplyContent.clear();
    mHasError = false;

    QNetworkRequest request(pUrl);

    request.setRawHeader("ACCEPT", "application/xml");

    foreach(QString key, mExtraHeaders.keys())
    {
        request.setRawHeader(key.toUtf8(), mExtraHeaders.value(key).toUtf8());
    }

    if (mIsVerbose)
        qDebug() << "getting from" << pUrl;

    mReply = mNam->get(request); // UB 4.2 TODO who owns and delete the reply ?

    connectReply();
}


void UBServerXMLHttpRequest::syntheticRubyOnRailsDelete(const QUrl& pUrl)
{
    if (mIsRunning)
    {
        qDebug() << this << "already running ...";
        return;
    }

    mIsRunning = true;

    mReplyContent.clear();
    mHasError = false;

    QNetworkRequest request(pUrl);

    request.setHeader(QNetworkRequest::ContentTypeHeader, mContentType);
    request.setRawHeader("ACCEPT", "application/xml");
    request.setRawHeader("X_HTTP_METHOD_OVERRIDE", "delete");

    foreach(QString key, mExtraHeaders.keys())
    {
        request.setRawHeader(key.toUtf8(), mExtraHeaders.value(key).toUtf8());
    }

    if (mIsVerbose)
        qDebug() << "deleting to" << pUrl;

    QString fakePayload;
    fakePayload += "<fake/>";

    mReply = mNam->post(request, fakePayload.toUtf8()); // UB 4.2 TODO who owns and delete the reply ?

    connectReply();

}


void UBServerXMLHttpRequest::connectReply()
{
    if (mReply)
    {
        connect(mReply, SIGNAL(finished ()), this, SLOT(finished ()));
        connect(mReply, SIGNAL(error ( QNetworkReply::NetworkError )), this, SLOT(error ( QNetworkReply::NetworkError)));
        connect(mReply, SIGNAL(downloadProgress ( qint64 , qint64  )), this, SLOT(downloadProgress ( qint64 , qint64  )));
        connect(mReply, SIGNAL(uploadProgress ( qint64 , qint64  )), this, SLOT(uploadProgress ( qint64 , qint64  )));
        connect(mReply, SIGNAL(readyRead (   )), this, SLOT( readyRead( )));
    }
}


void UBServerXMLHttpRequest::downloadProgress (qint64 bytesReceived, qint64 bytesTotal)
{
    if (mIsVerbose && bytesTotal > 1024)
    {
        qDebug () << "downloadProgress" << (bytesReceived * 100 / bytesTotal)
            << "% (" << bytesReceived / 1024 << "/" << bytesTotal / 1024 << "KB)";
    }

    emit progress(bytesReceived, bytesTotal);
}


void UBServerXMLHttpRequest::error (QNetworkReply::NetworkError code)
{
    if (mIsVerbose)
    {
        qDebug() << "receive error";
    }

    if (code != QNetworkReply::NoError)
    {
        //qDebug() << "error code: " << code;
        mHasError = true;
    }
}


void UBServerXMLHttpRequest::finished()
{
    if (mIsVerbose)
    {
        if (mReplyContent.length() < 1024)
            qDebug() << mReplyContent;
        else
            qDebug() << "processed" << mReplyContent.length() / 1024 << "KB";
    }

    if (mReply->header(QNetworkRequest::LocationHeader).isValid() && mRedirectionCount < 10)
    {
        mIsRunning = false;
        mRedirectionCount++;

        if (mReply->operation() == QNetworkAccessManager::GetOperation)
        {
            get(mReply->header(QNetworkRequest::LocationHeader).toUrl());
            return;
        }
        else if (mReply->operation() == QNetworkAccessManager::PutOperation)
        {
            put(mReply->header(QNetworkRequest::LocationHeader).toUrl(), mPutPayload);
        }
        else if (mReply->operation() == QNetworkAccessManager::PostOperation)
        {
            post(mReply->header(QNetworkRequest::LocationHeader).toUrl(), mPostPayload);
        }
        else if (mReply->operation() == QNetworkAccessManager::HeadOperation)
        {
            //TODO UB 4.x
        }
    }

    mRedirectionCount = 0;

    emit finished(!mHasError, mReplyContent);

    mReply->deleteLater();
    mReply = 0;

    deleteLater();
}


void UBServerXMLHttpRequest::uploadProgress ( qint64 bytesSent, qint64 bytesTotal )
{
    if (mIsVerbose && bytesTotal > 0)
    {
        qDebug() << "receive data";
        qDebug() << "uploadProgress" << (bytesSent * 100 / bytesTotal) << "% (" << bytesSent / 1024 << "/" << bytesTotal / 1024 << "KB)";
    }

    emit progress(bytesSent, bytesTotal);
}


void UBServerXMLHttpRequest::readyRead()
{
    mReplyContent.append(mReply->readAll());

    if (mIsVerbose)
        qDebug() << "reply has been read";

}


void UBServerXMLHttpRequest::setVerbose(bool pVerbose)
{
    mIsVerbose = pVerbose;
}

