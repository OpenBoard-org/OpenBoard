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

#include <QFile>

#include "core/UBApplication.h"

UBBackgroundLoader::UBBackgroundLoader(QObject* parent)
    : QThread{parent}
{
}

UBBackgroundLoader::UBBackgroundLoader(QList<std::pair<int, QString>> paths, QObject* parent)
    : QThread{parent}
{
    mPaths.insert(mPaths.cend(), paths.constBegin(), paths.constEnd());
    mPathCounter.release(paths.size());
}

UBBackgroundLoader::~UBBackgroundLoader()
{
    abort();
    wait();
}

bool UBBackgroundLoader::isResultAvailable()
{
    QMutexLocker lock{&mMutex};
    return !mResults.empty();
}

std::pair<int, QByteArray> UBBackgroundLoader::takeResult()
{
    QMutexLocker lock{&mMutex};

    if (mResults.empty())
    {
        return {};
    }

    const auto result = mResults.front();
    mResults.pop_front();
    return result;
}

void UBBackgroundLoader::start()
{
    mRunning = true;
    QThread::start();
}

void UBBackgroundLoader::addPaths(QList<std::pair<int, QString>> paths)
{
    QMutexLocker lock{&mMutex};
    mPaths.insert(mPaths.cend(), paths.constBegin(), paths.constEnd());
    mPathCounter.release(paths.size());
}

void UBBackgroundLoader::abort()
{
    mRunning = false;
    mPathCounter.release();
}

void UBBackgroundLoader::run()
{
    while (mRunning && !UBApplication::isClosing)
    {
        mPathCounter.acquire();

        if (mRunning && !UBApplication::isClosing)
        {
            std::pair<int, QString> path;

            {
                QMutexLocker lock{&mMutex};
                path = mPaths.front();
                mPaths.pop_front();
            }

            QFile file{path.second};
            QByteArray result;

            if (file.open(QFile::ReadOnly))
            {
                result = file.readAll();
                file.close();
            }

            {
                QMutexLocker lock{&mMutex};
                mResults.push_back({path.first, result});
            }

            emit resultAvailable(path.first, result);
        }
    }

    quit();
}
