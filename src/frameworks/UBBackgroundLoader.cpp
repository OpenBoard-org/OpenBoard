/*
 * Copyright (C) 2015-2025 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBBackgroundLoader.h"

#include <QDebug>
#include <QFile>
#include <QFutureWatcher>
#include <QPixmap>
#include <QtConcurrentMap>

#include "frameworks/UBBlockingBuffer.h"

UBBackgroundLoader::UBBackgroundLoader(ResultType resultType, QObject* parent)
    : QObject{parent}
    , mResultType{resultType}
{
    mWatcher = new QFutureWatcher<std::pair<int, QVariant>>;
    mWatcher->setPendingResultsLimit(2);
    mWatcherThread = new QThread{this};
    mWatcher->moveToThread(mWatcherThread);

    mBlockingBuffer = new UBBlockingBuffer{this};
    mBlockingBuffer->setWatcher(mWatcher);

    connect(mBlockingBuffer, &UBBlockingBuffer::resultAvailable, this, &UBBackgroundLoader::resultAvailable);
    connect(mBlockingBuffer, &UBBlockingBuffer::finished, this, &UBBackgroundLoader::finished);

    connect(mWatcher, &QFutureWatcher<std::pair<int, QByteArray>>::finished, mWatcherThread, &QThread::quit);

    mBlockingBuffer->start();
    mWatcherThread->start();
}

UBBackgroundLoader::~UBBackgroundLoader()
{
    qDebug() << "Destruct UBBackgroundLoader";
    mWatcher->cancel();
    mWatcherThread->quit();
    mBlockingBuffer->halt();
    mWatcherThread->wait();
    mBlockingBuffer->wait();
    abort();

    delete mWatcher;
}

void UBBackgroundLoader::load(const QList<std::pair<int, QString>>& paths, int maxBytes, std::function<void(int,QString)> preCheck)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    mFuture = QtConcurrent::mapped(paths, ReadData{mResultType, maxBytes, preCheck});
#else
    // use a separate thread pool for each loader so that tasks are interwoven between loaders
    auto threadPool = new QThreadPool{this};
    threadPool->setMaxThreadCount(4);
    mFuture = QtConcurrent::mapped(threadPool, paths, ReadData{mResultType, maxBytes, preCheck});
#endif

    mWatcher->setFuture(mFuture);

    qDebug() << "UBBackgroundLoader: Start loading" << paths.at(0).second;
}

void UBBackgroundLoader::abort()
{
    mFuture.cancel();
}

void UBBackgroundLoader::waitForFinished()
{
    mFuture.waitForFinished();
}

void UBBackgroundLoader::setKeepAlive(std::shared_ptr<void> keepAlive)
{
    mKeepAlive = keepAlive;
}

void UBBackgroundLoader::resultProcessed()
{
    mBlockingBuffer->resultProcessed();
}

UBBackgroundLoader::ReadData::ReadData(ResultType resultType, int maxBytes, std::function<void (int, QString)> preCheck)
    : mResultType{resultType}
    , mMaxBytes{maxBytes}
    , mPreCheck{preCheck}
{
}

UBBackgroundLoader::ReadData::result_type UBBackgroundLoader::ReadData::operator()(const std::pair<int, QString>& path)
{
    if (mPreCheck)
    {
        mPreCheck(path.first, path.second);
    }

    QFile file{path.second};
    QVariant result;

    if (file.open(QFile::ReadOnly))
    {
        QByteArray data;

        if (mMaxBytes < 0)
        {
            data = file.readAll();
        }
        else
        {
            data = file.read(mMaxBytes);
        }

        file.close();

        switch (mResultType)
        {
        case ByteArray:
            result = data;
            break;

        case Pixmap:
        {
            QPixmap pixmap;
            pixmap.loadFromData(data);
            result = pixmap;
            break;
        }
        }
    }

    return {path.first, result};
}
