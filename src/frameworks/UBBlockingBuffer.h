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

#include <QFutureWatcher>
#include <QSemaphore>
#include <QThread>

#include <deque>

class UBBlockingBuffer : public QThread
{
    Q_OBJECT

public:
    UBBlockingBuffer(QObject* parent = nullptr);
    void setWatcher(QFutureWatcher<std::pair<int, QVariant>>* watcher);

public slots:
    void addResult(int index);
    void resultProcessed();
    void watcherFinished();
    void halt();

signals:
    void resultAvailable(int index, const QVariant& data);
    void finished();

protected:
    void run() override;

private:
    QFutureWatcher<std::pair<int, QVariant>>* mWatcher;
    QSemaphore mAvailableResults;
    QSemaphore mAvailableSpace{1};
    QMutex mMutex;
    std::deque<std::pair<int,QVariant>> mBuffer;
};
