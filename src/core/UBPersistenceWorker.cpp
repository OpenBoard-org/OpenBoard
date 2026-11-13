/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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



#include "UBPersistenceWorker.h"

#include <QDeadlineTimer>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "adaptors/UBMetadataDcSubsetAdaptor.h"
#include "core/UBPersistenceJournal.h"
#include "core/UBPersistenceManager.h"
#include "frameworks/UBFileSystemUtils.h"

UBPersistenceWorker::UBPersistenceWorker(QObject *parent) :
    QObject(parent)
  , mReceivedApplicationClosing(false)
{
}

void UBPersistenceWorker::saveScene(std::shared_ptr<UBDocumentProxy> proxy, UBGraphicsScene *scene, const int pageIndex)
{
    enqueue({WriteScene, proxy, scene, pageIndex, QString(), QByteArray()});
}

void UBPersistenceWorker::saveMetadata(std::shared_ptr<UBDocumentProxy> proxy)
{
    enqueue({WriteMetadata, proxy, nullptr, 0, QString(), QByteArray()});
}

void UBPersistenceWorker::saveFile(const QString& fileName, const QByteArray& data)
{
    enqueue({WriteFile, nullptr, nullptr, 0, fileName, data});
}

void UBPersistenceWorker::replayJournal(const QString& rootDirectory)
{
    enqueue({ReplayJournal, nullptr, nullptr, 0, rootDirectory, QByteArray()});
}

void UBPersistenceWorker::saveThumbnail(const QString& fileName, const QByteArray& data)
{
    enqueue({WriteThumbnail, nullptr, nullptr, 0, fileName, data});
}

quint64 UBPersistenceWorker::nextSequence()
{
    static std::atomic<quint64> sSequence{0};
    return ++sSequence;
}

void UBPersistenceWorker::enqueue(PersistenceInformation entry)
{
    entry.sequence = nextSequence();
    QMutexLocker locker(&mMutex);
    saves.append(entry);
    mSemaphore.release();
}

bool UBPersistenceWorker::waitForAllSaved(int timeoutMs)
{
    // must not be called from the worker thread
    QDeadlineTimer deadline = timeoutMs < 0 ? QDeadlineTimer(QDeadlineTimer::Forever) : QDeadlineTimer(timeoutMs);
    QMutexLocker locker(&mMutex);

    while (!saves.isEmpty() || mBusy)
    {
        if (!mAllSaved.wait(&mMutex, deadline))
        {
            return false;
        }
    }

    return true;
}

qint64 UBPersistenceWorker::busyDuration() const
{
    QMutexLocker locker(&mMutex);
    return mBusy ? mBusyTimer.elapsed() : 0;
}

bool UBPersistenceWorker::saveInProgress(PersistenceInformation& info) const
{
    QMutexLocker locker(&mMutex);

    if (!mBusy)
    {
        return false;
    }

    info = mCurrent;
    return true;
}

QList<PersistenceInformation> UBPersistenceWorker::takePendingSaves()
{
    QMutexLocker locker(&mMutex);
    QList<PersistenceInformation> pending;
    pending.swap(saves);
    return pending;
}

void UBPersistenceWorker::requeue(const QList<PersistenceInformation>& pending)
{
    // put back in front, in the same order and with the same sequence numbers
    QMutexLocker locker(&mMutex);
    saves = pending + saves;
    mSemaphore.release(pending.size());
}

void UBPersistenceWorker::setStorageAvailable(bool available)
{
    mStorageAvailable = available;
}

bool UBPersistenceWorker::write(const QString& fileName, const QByteArray& data, bool createDirectory)
{
    // Replacing the file may fail temporarily, e.g. on Windows when an antivirus or
    // a synchronization client holds the file open. Retry a few times before failing,
    // but not while the storage is known to be unavailable.
    const int attempts = mStorageAvailable ? 3 : 1;

    for (int attempt = 0; attempt < attempts; ++attempt)
    {
        if (attempt > 0)
        {
            QThread::msleep(300);
        }

        if (createDirectory)
        {
            // Create the document folder only, never its parents: the parents are missing when
            // the storage is not mounted, and the file must not be written to the mount point.
            const QString directory = QFileInfo(fileName).path();

            if (!QFileInfo::exists(directory))
            {
                QDir().mkdir(directory);
            }
        }

        if (UBFileSystemUtils::writeFile(fileName, data))
        {
            return true;
        }
    }

    return false;
}

bool UBPersistenceWorker::persist(const QString& fileName, const QByteArray& data, bool createDirectory, quint64 sequence)
{
    const auto journal = UBPersistenceJournal::journal();

    // keep a local copy first: nothing is lost if writing to the storage fails or hangs
    if (!journal->store(fileName, data, sequence))
    {
        qWarning() << "No local copy could be stored for" << fileName;
    }

    if (!write(fileName, data, createDirectory))
    {
        return false;
    }

    journal->written(fileName, sequence);
    journal->noteFileState(fileName);
    return true;
}

bool UBPersistenceWorker::replay(const QString& rootDirectory)
{
    const auto journal = UBPersistenceJournal::journal();
    QStringList files = journal->pendingFiles(rootDirectory);

    if (files.isEmpty())
    {
        // nothing to write (e.g. only a read did not answer): check that the location answers again
        return QFileInfo::exists(rootDirectory);
    }

    // write the pages first, they create the document folder
    std::stable_sort(files.begin(), files.end(), [](const QString& a, const QString& b){
        return UBPersistenceManager::isPageFile(a) && !UBPersistenceManager::isPageFile(b);
    });

    bool written = false;

    for (const QString& fileName : files)
    {
        const QString directory = QFileInfo(fileName).path();
        const bool isPage = UBPersistenceManager::isPageFile(fileName);

        if (directory != rootDirectory && UBPersistenceManager::isWrittenByNewerVersion(directory))
        {
            // never written into such a document: a recovered copy is created at the next start
            continue;
        }

        if (!isPage && directory != rootDirectory && !QFileInfo::exists(directory))
        {
            if (QFileInfo::exists(QFileInfo(directory).path()))
            {
                // the document does not exist anymore
                qWarning() << "Dropping the local copy of" << fileName << ": the document does not exist";
                journal->discard(directory);
                continue;
            }

            return false;
        }

        // the local copy is always the newest version of the file
        quint64 sequence = 0;
        const QByteArray data = journal->data(fileName, &sequence);

        if (data.isEmpty())
        {
            continue;
        }

        if (!write(fileName, data, isPage))
        {
            return false;
        }

        qInfo() << "Written from the local copy:" << fileName;
        journal->written(fileName, sequence);
        journal->noteFileState(fileName);
        written = true;
    }

    // nothing written (e.g. only files kept for a recovered copy): check that the location answers
    return written || QFileInfo::exists(rootDirectory);
}

void UBPersistenceWorker::applicationWillClose()
{
    qDebug() << "application Will close signal received";
    QMutexLocker locker(&mMutex);
    mReceivedApplicationClosing = true;
    mSemaphore.release();
}

void UBPersistenceWorker::process()
{
    qDebug() << "process starts";

    // Each queued entry and the closing request release the semaphore once.
    // The loop only terminates when closing was requested AND the queue is
    // empty, so pending saves are never dropped when the application quits.
    while (true)
    {
        mSemaphore.acquire();

        PersistenceInformation info;

        {
            QMutexLocker locker(&mMutex);

            if (saves.isEmpty())
            {
                if (mReceivedApplicationClosing)
                {
                    break;
                }

                continue;
            }

            info = saves.takeFirst();
            mBusy = true;
            mCurrent = info;
            mBusyTimer.start();
        }

        bool saved = true;

        if (info.action == WriteScene)
        {
            const QString fileName = UBSvgSubsetAdaptor::pageFileName(info.proxy, info.sceneIndex);
            const QByteArray data = UBSvgSubsetAdaptor::serializeScene(info.proxy, info.scene->shared_from_this(), info.sceneIndex);
            saved = persist(fileName, data, true, info.sequence);

            if (!saved)
            {
                // emitted before scenePersisted, so the scene copy is still valid when handled
                emit saveFailed(info.scene, info.proxy->persistencePath());
            }

            emit scenePersisted(info.scene);
        }
        else if (info.action == WriteMetadata)
        {
            const QString directory = info.proxy->persistencePath();

            // A document without pages has no folder and its metadata are not saved, like before.
            // But keep them if pages are waiting in the journal: the folder could not be created.
            const bool isEmptyDocument = !QFileInfo::exists(directory)
                    && QFileInfo::exists(QFileInfo(directory).path())
                    && !UBPersistenceJournal::journal()->hasPendingFiles(directory);

            if (!isEmptyDocument)
            {
                saved = persist(UBMetadataDcSubsetAdaptor::fileName(info.proxy), UBMetadataDcSubsetAdaptor::serialize(info.proxy), false, info.sequence);

                if (!saved)
                {
                    emit saveFailed(nullptr, directory);
                }
            }

            emit metadataPersisted(info.proxy);
        }
        else if (info.action == WriteFile)
        {
            saved = persist(info.path, info.data, false, info.sequence);

            if (!saved)
            {
                emit saveFailed(nullptr, QFileInfo(info.path).path());
            }
        }
        else if (info.action == WriteThumbnail)
        {
            // not journaled: thumbnails are created again from the pages when missing
            if (write(info.path, info.data, false))
            {
                emit thumbnailWritten(info.path);
            }

            saved = false; // not a document change
        }
        else if (info.action == ReplayJournal)
        {
            emit journalReplayed(replay(info.path));
            saved = false; // not a new save
        }

        if (saved)
        {
            emit saveSucceeded();
        }

        {
            QMutexLocker locker(&mMutex);
            mBusy = false;
            mCurrent = PersistenceInformation{};

            if (saves.isEmpty())
            {
                mAllSaved.wakeAll();
            }
        }
    }

    qDebug() << "process will stop";
    emit finished();
}
