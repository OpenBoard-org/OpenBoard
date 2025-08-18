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


#include "UBBlockingBuffer.h"

UBBlockingBuffer::UBBlockingBuffer(QObject* parent)
    : QThread{parent}
{
}

void UBBlockingBuffer::setWatcher(QFutureWatcher<std::pair<int, QByteArray>>* watcher)
{
    mWatcher = watcher;

    connect(mWatcher, &QFutureWatcher<std::pair<int, QByteArray>>::resultReadyAt, this, &UBBlockingBuffer::addResult,
            Qt::DirectConnection);
    connect(mWatcher, &QFutureWatcher<std::pair<int, QByteArray>>::finished, this, &UBBlockingBuffer::watcherFinished,
            Qt::DirectConnection);
}

void UBBlockingBuffer::addResult(int index)
{
    if (!isInterruptionRequested())
    {
        mAvailableSpace.acquire();

        if (mWatcher)
        {
            const auto result = mWatcher->resultAt(index);

            QMutexLocker lock{&mMutex};
            mBuffer.emplace_back(result.first, result.second);
            mAvailableResults.release();
        }
    }
}

void UBBlockingBuffer::resultProcessed(int index)
{
    mAvailableSpace.release();
}

void UBBlockingBuffer::watcherFinished()
{
    if (!isInterruptionRequested())
    {
        // add a "finished" marker to the buffer
        mAvailableSpace.acquire();
        QMutexLocker lock{&mMutex};
        mBuffer.emplace_back(-1, QByteArray{});
        mAvailableResults.release();
    }
}

void UBBlockingBuffer::halt()
{
    requestInterruption();

    mAvailableResults.release();
    mAvailableSpace.release(999); // last results and finish marker
    mWatcher = nullptr;
}

void UBBlockingBuffer::run()
{
    while (true)
    {
        mAvailableResults.acquire();
        QMutexLocker lock{&mMutex};

        if (isInterruptionRequested())
        {
            return;
        }

        const auto result = mBuffer.front();
        mBuffer.pop_front();

        if (result.first >= 0)
        {
            emit resultAvailable(result.first, result.second);
        }
        else
        {
            emit finished();
        }
    }
}
