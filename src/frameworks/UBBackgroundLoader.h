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
#include <QVariant>

// forward
class UBBlockingBuffer;


class UBBackgroundLoader : public QObject
{
    Q_OBJECT

public:
    enum ResultType {
        ByteArray,
        Pixmap
    };

    UBBackgroundLoader(ResultType resultType, QObject* parent = nullptr);
    virtual ~UBBackgroundLoader();

    void load(const QList<std::pair<int, QString>>& paths, int maxBytes = -1, std::function<void(int,QString)> preCheck = nullptr);
    void abort();
    void setKeepAlive(std::shared_ptr<void> keepAlive);

public slots:
    void resultProcessed();

signals:
    void resultAvailable(int index, const QVariant& data);
    void finished();

private:
    class ReadData
    {
    public:
        typedef std::pair<int, QVariant> result_type;

        ReadData(ResultType resultType, int maxBytes, std::function<void(int,QString)> preCheck);
        result_type operator()(const std::pair<int, QString>& path);

    private:
        const ResultType mResultType{ByteArray};
        const int mMaxBytes{-1};
        std::function<void(int,QString)> mPreCheck{};
    };

private:
    const ResultType mResultType{ByteArray};
    QFuture<std::pair<int, QVariant>> mFuture;
    int mIndex{0};
    QFutureWatcher<std::pair<int, QVariant>>* mWatcher{nullptr};
    std::shared_ptr<void> mKeepAlive;
    UBBlockingBuffer* mBlockingBuffer{nullptr};
    QThread* mWatcherThread{nullptr};
};
