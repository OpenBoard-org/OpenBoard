/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBDocumentStorage.h"

#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QSemaphore>
#include <QStorageInfo>
#include <QThread>
#include <QThreadPool>

#include <atomic>
#include <memory>

#include "core/UBPersistenceJournal.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"

// A read is given up when no data arrived during this time: a slow storage which still
// delivers data is waited for.
static const int answerTimeoutMs = 3000;

// shorter while the storage is already known to be unavailable
static const int answerTimeoutUnavailableMs = 1000;

static const qint64 chunkSize = 256 * 1024;

// Threads reading from the storage. Allocated once and never destroyed:
// a read blocked on a storage which does not answer may still be running at exit.
static QThreadPool* readPool()
{
    static QThreadPool* pool = nullptr;

    if (!pool)
    {
        pool = new QThreadPool();
        pool->setMaxThreadCount(4);
    }

    return pool;
}

static bool isReadPoolBusy()
{
    return readPool()->activeThreadCount() >= readPool()->maxThreadCount();
}

struct UBDocumentStorage::FileRead
{
    QSemaphore done;
    std::atomic<qint64> received{0};
    bool exists{true};
    bool complete{false};
    QByteArray data;
};

UBDocumentStorage::UBDocumentStorage(const QString& mountPoint, QObject* parent)
    : QObject(parent)
    , mMountPoint(mountPoint)
{
}

UBDocumentStorage::ReadResult UBDocumentStorage::read(const QString& path, QByteArray& data)
{
    // a newer version of the file may still wait to be written
    const QByteArray pending = UBPersistenceJournal::journal()->data(path);

    if (!pending.isEmpty())
    {
        data = pending;
        return ReadResult::Read;
    }

    auto read = std::make_shared<FileRead>();

    if (QThread::currentThread() != thread())
    {
        // not the GUI thread: waiting does not block the user interface
        readFile(path, *read);
        postResult(path, read);
    }
    else
    {
        if (isReadPoolBusy())
        {
            // previous reads are still blocked
            qWarning() << "The document location does not answer, cannot read" << path;
            mUnreadFiles.insert(path);
            setAvailable(false);
            return ReadResult::Unavailable;
        }

        startRead(path, read);

        // give up only if no data arrives anymore
        const int timeoutMs = mAvailable ? answerTimeoutMs : answerTimeoutUnavailableMs;
        qint64 received = -1;
        QElapsedTimer sinceLastData;
        sinceLastData.start();

        while (!read->done.tryAcquire(1, 100))
        {
            if (read->received != received)
            {
                received = read->received;
                sinceLastData.restart();
            }
            else if (sinceLastData.elapsed() > timeoutMs)
            {
                // the read goes on in the background and is delivered by fileReadLater() if it succeeds
                qWarning() << "The document location does not answer, cannot read" << path;
                mUnreadFiles.insert(path);
                mReadingAgain.insert(path);
                setAvailable(false);
                return ReadResult::Unavailable;
            }
        }
    }

    if (!read->exists)
    {
        return ReadResult::Missing;
    }

    if (!read->complete)
    {
        qWarning() << "Cannot read" << path;
        return ReadResult::Unavailable;
    }

    data = read->data;
    return ReadResult::Read;
}

bool UBDocumentStorage::isLocal() const
{
    return mMountPoint.isEmpty();
}

bool UBDocumentStorage::isAvailable() const
{
    return mAvailable;
}

void UBDocumentStorage::setAvailable(bool available)
{
    if (available != mAvailable)
    {
        mAvailable = available;
        emit availabilityChanged(available);

        if (available)
        {
            readUnreadFiles();
        }
    }
}

bool UBDocumentStorage::isReachable(const QString& dataDirectory, const QString& mountPoint)
{
    if (dataDirectory.isEmpty())
    {
        return false;
    }

    // Find the nearest existing directory: the data directory itself or a parent.
    // Drive letters or shares which are not connected have no existing directory.
    QString existingPath = dataDirectory;
    const bool dataDirectoryExists = QDir(dataDirectory).exists();

    while (!QDir(existingPath).exists())
    {
        const QString next = QFileInfo(existingPath).absolutePath();

        if (next.isEmpty() || next == existingPath || QDir(existingPath).isRoot())
        {
            return false;
        }

        existingPath = next;
    }

    if (!mountPoint.isEmpty() && !isMountedAt(existingPath, mountPoint))
    {
        return false;
    }

    // A data directory which exists but cannot be read (e.g. access denied once the user's
    // credentials to a share expired) would show an empty list of documents. If it does not
    // exist yet, it is created: its parent only needs to exist.
    return !dataDirectoryExists || UBPlatformUtils::canListDirectory(dataDirectory);
}

void UBDocumentStorage::readFile(const QString& path, FileRead& read)
{
    QFile file(path);

    if (!file.exists())
    {
        read.exists = false;
    }
    else if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data;

        while (!file.atEnd())
        {
            const QByteArray chunk = file.read(chunkSize);

            if (chunk.isEmpty())
            {
                break;
            }

            data += chunk;
            read.received += chunk.size();
        }

        read.complete = file.atEnd() && file.error() == QFileDevice::NoError;
        read.data = data;
    }

    // remember the state of the file, to detect changes made elsewhere before a local copy
    // of it is written
    UBPersistenceJournal::journal()->noteFileState(path);
}

void UBDocumentStorage::startRead(const QString& path, const std::shared_ptr<FileRead>& read)
{
    readPool()->start([this, path, read]() {
        readFile(path, *read);
        read->done.release();
        postResult(path, read);
    });
}

void UBDocumentStorage::postResult(const QString& path, const std::shared_ptr<FileRead>& read)
{
    QMetaObject::invokeMethod(this, [this, path, read]() {
        onFileRead(path, *read);
    }, Qt::QueuedConnection);
}

void UBDocumentStorage::onFileRead(const QString& path, const FileRead& read)
{
    mReadingAgain.remove(path);

    if (!read.complete)
    {
        if (read.exists)
        {
            // reported as unavailable, read again when the storage is available
            mUnreadFiles.insert(path);
        }

        return;
    }

    if (!mUnreadFiles.contains(path))
    {
        return;
    }

    mUnreadFiles.remove(path);

    // a newer version of the file may wait to be written meanwhile
    const QByteArray pending = UBPersistenceJournal::journal()->data(path);
    emit fileReadLater(path, pending.isEmpty() ? read.data : pending);

    if (mAvailable)
    {
        // continue with the next files, the storage answers
        readUnreadFiles();
    }
}

void UBDocumentStorage::readUnreadFiles()
{
    for (const QString& path : std::as_const(mUnreadFiles))
    {
        if (mReadingAgain.contains(path))
        {
            continue;
        }

        if (isReadPoolBusy())
        {
            // reads of the user interface go first; the remaining files are read when a read finished
            return;
        }

        mReadingAgain.insert(path);
        startRead(path, std::make_shared<FileRead>());
    }
}

bool UBDocumentStorage::isMountedAt(const QString& existingPath, const QString& mountPoint)
{
    // the mount point must currently be the root of a mounted volume, and the path inside it
    const QFileInfo mountInfo(mountPoint);

    if (!mountInfo.isDir())
    {
        return false;
    }

    const QStorageInfo volume(mountInfo.absoluteFilePath());

    if (volume.isValid())
    {
        if (!volume.isReady() || !UBFileSystemUtils::isSamePath(volume.rootPath(), mountPoint))
        {
            return false;
        }
    }
    else if (!QDir(mountInfo.absoluteFilePath()).isRoot())
    {
        // Qt cannot read the volume information of every drive (e.g. one created with subst):
        // its root is then unknown, only the root of a drive can be such a mount point
        return false;
    }

    return UBFileSystemUtils::isSamePath(existingPath, mountPoint) || UBFileSystemUtils::isInside(existingPath, mountPoint);
}
