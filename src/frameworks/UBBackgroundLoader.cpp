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
#include <QtConcurrentMap>


UBBackgroundLoader::UBBackgroundLoader(const QList<std::pair<int, QString>>& paths, int maxBytes, QObject* parent)
    : QObject{parent}
    , mCount{paths.count()}
{
    mFuture = QtConcurrent::mapped(paths, ReadData{maxBytes});
}

UBBackgroundLoader::UBBackgroundLoader(const QList<std::pair<int, QString>>& paths, QObject* parent)
    : UBBackgroundLoader{paths, -1, parent}
{
}

UBBackgroundLoader::~UBBackgroundLoader()
{
    abort();
}

bool UBBackgroundLoader::isIdle()
{
    return mFuture.isFinished() && mIndex == mFuture.resultCount();
}

bool UBBackgroundLoader::isResultAvailable()
{
    return mFuture.isResultReadyAt(mIndex);
}

std::pair<int, QByteArray> UBBackgroundLoader::takeResult()
{
    return mFuture.resultAt(mIndex++);
}

void UBBackgroundLoader::abort()
{
    mFuture.cancel();
}

UBBackgroundLoader::ReadData::ReadData(int maxBytes)
    : mMaxBytes{maxBytes}
{
}

UBBackgroundLoader::ReadData::result_type UBBackgroundLoader::ReadData::operator()(const std::pair<int, QString>& path)
{
    QFile file{path.second};
    QByteArray result;

    if (file.open(QFile::ReadOnly))
    {
        if (mMaxBytes < 0)
        {
            result = file.readAll();
        }
        else
        {
            result = file.read(mMaxBytes);
        }

        file.close();
    }

    return {path.first, result};
}
