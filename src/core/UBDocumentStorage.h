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


#pragma once

#include <QByteArray>
#include <QObject>
#include <QSet>
#include <QString>

#include <memory>

class QThreadPool;

/**
 * The storage where the documents are kept: the data directory, on a local disk or on mounted
 * storage (e.g. a network share).
 *
 * It is the one place that knows whether the storage answers. Reading never blocks the user
 * interface for long: a file which does not answer is reported as unavailable, and the storage
 * is then considered unavailable until writing to it succeeds again.
 *
 * Reading returns the newest content of a file: the local copy kept by the journal while it is
 * not written yet, otherwise the file on the storage.
 *
 * Writing goes through UBPersistenceManager and UBPersistenceWorker, which report failures
 * with setAvailable().
 */
class UBDocumentStorage : public QObject
{
    Q_OBJECT

public:
    enum class ReadResult
    {
        Read,        // the content was read
        Missing,     // the file does not exist
        Unavailable  // the storage does not answer now, or the file cannot be read
    };

    // An empty mount point means local storage. Otherwise the documents are on the storage mounted
    // at this path, which is checked to be currently mounted.
    explicit UBDocumentStorage(const QString& mountPoint, QObject* parent = nullptr);

    // The newest content of a file (absolute path): the local copy if one waits, else the storage.
    // A file reported as unavailable is read again in the background: fileReadLater() delivers
    // its content as soon as it can be read.
    ReadResult read(const QString& path, QByteArray& data);

    bool isLocal() const;
    bool isAvailable() const;
    void setAvailable(bool available);

    // True if the data directory can be used: part of its path exists, the directory can be read
    // if it exists and, if a mount point is given, this mount point is currently the root of a
    // mounted volume containing the directory. A share which is not mounted may leave an empty
    // local directory at its mount point; writing there would hide the documents once the share
    // is mounted again.
    static bool isReachable(const QString& dataDirectory, const QString& mountPoint);

signals:
    void availabilityChanged(bool available);

    // A file which read() reported as unavailable could be read since: its read finished late,
    // or it was read again once the storage became available.
    void fileReadLater(const QString& path, const QByteArray& data);

private:
    // a read on the storage, shared with the reading thread, which may finish after read() gave up
    struct FileRead;

    // Threads reading from the storage. Allocated once and never destroyed:
    // a read blocked on a storage which does not answer may still be running at exit.
    static QThreadPool* readPool();
    static bool isReadPoolBusy();
    static void readFile(const QString& path, FileRead& read);
    void startRead(const QString& path, const std::shared_ptr<FileRead>& read);
    // passes the result to onFileRead() in the thread of the storage, from any thread
    void postResult(const QString& path, const std::shared_ptr<FileRead>& read);
    // result of every read, also of the reads given up by read()
    void onFileRead(const QString& path, const FileRead& read);
    void readUnreadFiles();

    static bool isMountedAt(const QString& existingPath, const QString& mountPoint);

    QString mMountPoint;
    bool mAvailable{true};
    // files reported as unavailable by read() and not read since
    QSet<QString> mUnreadFiles;
    // files of mUnreadFiles currently read again
    QSet<QString> mReadingAgain;
};
