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

// forward
class UBBlockingBuffer;


class UBBackgroundLoader : public QObject
{
    Q_OBJECT

public:
    UBBackgroundLoader(QObject* parent = nullptr);
    virtual ~UBBackgroundLoader();

    void load(const QList<std::pair<int, QString>>& paths, int maxBytes = -1);
    void abort();
    void waitForFinished();
    void setKeepAlive(std::shared_ptr<void> keepAlive);

public slots:
    void resultProcessed(int index);

signals:
    void resultAvailable(int index, const QByteArray& data);
    void finished();

private:
    class ReadData
    {
    public:
        typedef std::pair<int, QByteArray> result_type;

        ReadData(int maxBytes);
        result_type operator()(const std::pair<int, QString>& path);

    private:
        const int mMaxBytes{-1};
    };

private:
    QFuture<std::pair<int, QByteArray>> mFuture;
    int mIndex{0};
    QFutureWatcher<std::pair<int, QByteArray>>* mWatcher{nullptr};
    std::shared_ptr<void> mKeepAlive;
    UBBlockingBuffer* mBlockingBuffer{nullptr};
    QThread* mWatcherThread{nullptr};
};
