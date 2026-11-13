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
#include <QHash>
#include <QMutex>
#include <QString>
#include <QStringList>

#include <limits>

class QThread;

/**
 * Local copy of the files written to the document storage.
 *
 * Every page, metadata or folder file is first stored in a local recovery directory
 * and then written to the document storage. The local copy is removed as soon as the
 * file was written successfully. If the document storage is not available (network
 * share disconnected, storage full, ...), the local copy is kept and written later,
 * at the latest at the next start of OpenBoard. Nothing is lost when OpenBoard is closed,
 * killed or crashes before the storage is available again.
 *
 * The recovery directory is local to the computer, see UBSettings::recoveryDirectory().
 * All functions are thread safe.
 */
class UBPersistenceJournal
{
public:
    static UBPersistenceJournal* journal();

    // Keep a local copy of the data for the target file. Returns false if it could not be stored.
    // The sequence number orders the versions of a file (see UBPersistenceWorker::nextSequence()):
    // an older version never replaces a newer one, whichever thread stores it first.
    bool store(const QString& targetFile, const QByteArray& data, quint64 sequence);

    // The target file was written successfully with the version of this sequence number:
    // the local copy is not needed anymore, unless it is newer.
    void written(const QString& targetFile, quint64 sequence = std::numeric_limits<quint64>::max());

    // Remember the state of a target file after reading or writing it. Used at startup
    // to detect if the file was changed elsewhere before the local copy is written.
    void noteFileState(const QString& targetFile);

    // Drop the local copies of all files in this directory and its subdirectories.
    void discard(const QString& directory);

    // Stop accepting changes from other threads. Used when closing without waiting for
    // the persistence thread: the local copies stored by the calling thread are final.
    void freeze();

    bool contains(const QString& targetFile) const;
    QByteArray data(const QString& targetFile, quint64* sequence = nullptr) const;

    // true if local copies of files in this directory are waiting
    bool hasPendingFiles(const QString& directory) const;

    // target files waiting in this directory and its subdirectories
    QStringList pendingFiles(const QString& rootDirectory) const;

    // true if the target file was changed (or created) elsewhere since the local copy was based on it
    bool isChangedElsewhere(const QString& targetFile) const;

    // true if the target file existed in the storage before the local copy was stored
    bool existedBefore(const QString& targetFile) const;

private:
    UBPersistenceJournal();

    struct Entry
    {
        QString localFile;
        QString base; // state of the target file before the first local copy, empty if unknown
        quint64 sequence{0}; // version of the local copy, 0 when loaded from a previous session
    };

    static QString stateOf(const QString& file);
    QString groupDirectory(const QString& targetDirectory) const;
    bool isRejectedThread() const;
    void load();
    bool saveIndex(const QString& targetDirectory);

    mutable QMutex mMutex;
    QString mRoot;
    QHash<QString, Entry> mEntries;
    QHash<QString, QString> mFileStates;
    QThread* mFrozenThread{nullptr};
};
