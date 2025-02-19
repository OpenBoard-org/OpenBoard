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

#include <QMutex>
#include <QObject>
#include <QSemaphore>
#include <QThread>
#include <deque>

class UBBackgroundLoader : public QThread
{
    Q_OBJECT

public:
    explicit UBBackgroundLoader(QObject* parent = nullptr);
    UBBackgroundLoader(QList<std::pair<int, QString>> paths, QObject* parent = nullptr);
    virtual ~UBBackgroundLoader();

    bool isIdle();
    bool isResultAvailable();
    std::pair<int, QByteArray> takeResult();

public slots:
    void start();
    void addPaths(QList<std::pair<int, QString>> paths);
    void abort();

signals:
    void resultAvailable(int index, QByteArray data);

protected:
    void run() override;

private:
    std::deque<std::pair<int, QString>> mPaths{};
    std::deque<std::pair<int, QByteArray>> mResults{};
    QMutex mMutex{};
    QSemaphore mPathCounter{};
    bool mRunning{false};
};
