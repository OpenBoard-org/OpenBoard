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

#include "UBHttpGet.h"

#include <QtNetwork>

#include "network/UBNetworkAccessManager.h"

#include "core/memcheck.h"

UBHttpGet::UBHttpGet(QObject* parent)
    : QObject(parent)
    , mReply(0)
    , mIsBackground(false)
    , mRedirectionCount(0)
    , mIsSelfAborting(false)
{
    // NOOP
}


UBHttpGet::~UBHttpGet()
{
        if (mReply)
    {
        mIsSelfAborting = true;
        mReply->abort();
                delete mReply;
    }
}


QNetworkReply* UBHttpGet::get(QUrl pUrl, QPointF pPos, QSize pSize, bool isBackground)
{
    mPos = pPos;
    mSize = pSize;
    mIsBackground = isBackground;

    if (mReply)
        delete mReply;

    UBNetworkAccessManager * nam = UBNetworkAccessManager::defaultAccessManager();
    mReply = nam->get(QNetworkRequest(pUrl)); //mReply deleted by this destructor

    mDownloadedBytes.clear();

    connect(mReply, SIGNAL(finished()), this, SLOT(requestFinished()));
    connect(mReply, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(mReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgressed(qint64, qint64)));

    return mReply;
}


void UBHttpGet::readyRead()
{
        if (mReply)
                mDownloadedBytes += mReply->readAll();
}


void UBHttpGet::requestFinished()
{
    if (!mReply || mIsSelfAborting)
    {
        return;
    }

    if (mReply->error() != QNetworkReply::NoError)
    {
        qWarning() << mReply->url().toString() << "get finished with error : " << mReply->error();

        mDownloadedBytes.clear();

        mRedirectionCount = 0;

        emit downloadFinished(false, mReply->url(), mReply->errorString(), mDownloadedBytes, mPos, mSize, mIsBackground);
    }
    else
    {

        qDebug() << mReply->url().toString() << "http get finished ...";

        if (mReply->header(QNetworkRequest::LocationHeader).isValid() && mRedirectionCount < 10)
        {
            mRedirectionCount++;
            get(mReply->header(QNetworkRequest::LocationHeader).toUrl(), mPos, mSize, mIsBackground);

            return;
        }

        mRedirectionCount = 0;

        emit downloadFinished(true, mReply->url(), mReply->header(QNetworkRequest::ContentTypeHeader).toString(),
                        mDownloadedBytes, mPos, mSize, mIsBackground);
    }

}

void UBHttpGet::downloadProgressed(qint64 bytesReceived, qint64 bytesTotal)
{
//    qDebug() << "received: " << bytesReceived << ", / " << bytesTotal << " bytes";
    if (-1 != bytesTotal)
    {
        emit downloadProgress(bytesReceived, bytesTotal);
    }
}

