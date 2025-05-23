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


#pragma once

#include <QFuture>
#include <QObject>


class UBBackgroundLoader : public QObject
{
    Q_OBJECT

public:
    UBBackgroundLoader(const QList<std::pair<int, QString>>& paths, int maxBytes, QObject* parent = nullptr);
    UBBackgroundLoader(const QList<std::pair<int, QString>>& paths, QObject* parent = nullptr);
    virtual ~UBBackgroundLoader();

    bool isIdle();
    bool isResultAvailable();
    std::pair<int, QByteArray> takeResult();
    void abort();

private:
    struct ReadData
    {
        typedef std::pair<int, QByteArray> result_type;

        ReadData(int maxBytes);
        result_type operator()(const std::pair<int, QString>& path);

    private:
        const int mMaxBytes{-1};
    };

private:
    QFuture<std::pair<int, QByteArray>> mFuture;
    const qsizetype mCount{0};
    int mIndex{0};
};
