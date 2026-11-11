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


#include "UBPersistenceJournal.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QSet>
#include <QThread>

#include "core/UBSettings.h"

namespace
{
    const QString indexFileName{"journal.txt"};
    const QByteArray indexHeader{"OpenBoard journal 1"};
    const QString absent{"absent"};

    QString fileNameOf(const QString& file)
    {
        return file.mid(file.lastIndexOf('/') + 1);
    }
}

UBPersistenceJournal* UBPersistenceJournal::journal()
{
    // never destroyed: a persistence thread left behind when closing may still use it
    static UBPersistenceJournal* sJournal = new UBPersistenceJournal;
    return sJournal;
}

UBPersistenceJournal::UBPersistenceJournal()
    : mRoot(UBSettings::recoveryDirectory())
{
    load();
}

bool UBPersistenceJournal::writeFile(const QString& fileName, const QByteArray& data)
{
    QSaveFile file(fileName);

    // some storages do not allow to create the temporary file next to the target file:
    // then write directly to the target file, like before
    file.setDirectWriteFallback(true);

    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << "cannot open" << fileName << "for writing. Error:" << file.errorString();
        return false;
    }

    if (file.write(data) != data.size())
    {
        qCritical() << "cannot write" << fileName << ". Error:" << file.errorString();
        file.cancelWriting();
        return false;
    }

    if (!file.commit())
    {
        qCritical() << "cannot save" << fileName << ". Error:" << file.errorString();
        return false;
    }

    return true;
}

bool UBPersistenceJournal::store(const QString& targetFile, const QByteArray& data, quint64 sequence)
{
    QMutexLocker locker(&mMutex);

    if (isRejectedThread())
    {
        return true;
    }

    const QString targetDirectory = parentDirectory(targetFile);
    const QString group = groupDirectory(targetDirectory);

    if (!QDir().mkpath(group))
    {
        qCritical() << "cannot create the recovery directory" << group;
        return false;
    }

    const bool isNew = !mEntries.contains(targetFile);

    if (!isNew && sequence < mEntries.value(targetFile).sequence)
    {
        // a newer version is already stored
        return true;
    }

    if (isNew)
    {
        // register the file before writing it: at startup, an entry without data is ignored
        mEntries.insert(targetFile, {group + "/" + fileNameOf(targetFile), mFileStates.value(targetFile)});

        if (!saveIndex(targetDirectory))
        {
            mEntries.remove(targetFile);
            return false;
        }
    }

    if (!writeFile(mEntries.value(targetFile).localFile, data))
    {
        if (isNew)
        {
            mEntries.remove(targetFile);
            saveIndex(targetDirectory);
        }

        // an existing local copy is kept: it is older, but still newer than the target file
        return false;
    }

    mEntries[targetFile].sequence = sequence;
    return true;
}

void UBPersistenceJournal::written(const QString& targetFile, quint64 sequence)
{
    QMutexLocker locker(&mMutex);

    // keep a newer local copy, stored while this version was being written
    if (isRejectedThread() || !mEntries.contains(targetFile) || mEntries.value(targetFile).sequence > sequence)
    {
        return;
    }

    const Entry entry = mEntries.take(targetFile);
    saveIndex(parentDirectory(targetFile));
    QFile::remove(entry.localFile);
}

void UBPersistenceJournal::noteFileState(const QString& targetFile)
{
    // access the file outside the lock, it may be slow
    const QString state = stateOf(targetFile);

    QMutexLocker locker(&mMutex);
    mFileStates.insert(targetFile, state);
}

void UBPersistenceJournal::discard(const QString& directory)
{
    QMutexLocker locker(&mMutex);

    if (isRejectedThread())
    {
        return;
    }

    const QString prefix = directory + "/";
    QSet<QString> directories;

    for (auto it = mEntries.begin(); it != mEntries.end();)
    {
        if (it.key().startsWith(prefix))
        {
            directories << parentDirectory(it.key());
            QFile::remove(it.value().localFile);
            it = mEntries.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (const QString& targetDirectory : directories)
    {
        saveIndex(targetDirectory);
    }
}

void UBPersistenceJournal::freeze()
{
    QMutexLocker locker(&mMutex);
    mFrozenThread = QThread::currentThread();
}

bool UBPersistenceJournal::contains(const QString& targetFile) const
{
    QMutexLocker locker(&mMutex);
    return mEntries.contains(targetFile);
}

QByteArray UBPersistenceJournal::data(const QString& targetFile, quint64* sequence) const
{
    QMutexLocker locker(&mMutex);

    if (!mEntries.contains(targetFile))
    {
        return QByteArray();
    }

    if (sequence)
    {
        *sequence = mEntries.value(targetFile).sequence;
    }

    QFile file(mEntries.value(targetFile).localFile);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "cannot read the local copy of" << targetFile;
        return QByteArray();
    }

    return file.readAll();
}

bool UBPersistenceJournal::hasPendingFiles(const QString& directory) const
{
    QMutexLocker locker(&mMutex);
    const QString prefix = directory + "/";

    for (auto it = mEntries.cbegin(); it != mEntries.cend(); ++it)
    {
        if (it.key().startsWith(prefix))
        {
            return true;
        }
    }

    return false;
}

QStringList UBPersistenceJournal::pendingFiles(const QString& rootDirectory) const
{
    QMutexLocker locker(&mMutex);
    const QString prefix = rootDirectory + "/";
    QStringList files;

    for (auto it = mEntries.cbegin(); it != mEntries.cend(); ++it)
    {
        if (it.key().startsWith(prefix))
        {
            files << it.key();
        }
    }

    files.sort();
    return files;
}

bool UBPersistenceJournal::isChangedElsewhere(const QString& targetFile) const
{
    QString base;

    {
        QMutexLocker locker(&mMutex);
        base = mEntries.value(targetFile).base;
    }

    const QString current = stateOf(targetFile);

    if (base.isEmpty())
    {
        // state unknown: the file was created by OpenBoard and never written successfully
        return current != absent;
    }

    return current != base;
}

bool UBPersistenceJournal::existedBefore(const QString& targetFile) const
{
    QMutexLocker locker(&mMutex);
    const QString base = mEntries.value(targetFile).base;
    return !base.isEmpty() && base != absent;
}

QString UBPersistenceJournal::parentDirectory(const QString& file)
{
    const int index = file.lastIndexOf('/');
    return index < 0 ? QString() : file.left(index);
}

QString UBPersistenceJournal::stateOf(const QString& file)
{
    const QFileInfo info(file);

    if (!info.exists())
    {
        return absent;
    }

    return QString("%1:%2").arg(info.lastModified().toMSecsSinceEpoch()).arg(info.size());
}

QString UBPersistenceJournal::groupDirectory(const QString& targetDirectory) const
{
    const QByteArray hash = QCryptographicHash::hash(targetDirectory.toUtf8(), QCryptographicHash::Sha1).toHex();
    return mRoot + "/" + QString::fromLatin1(hash.left(16));
}

bool UBPersistenceJournal::isRejectedThread() const
{
    return mFrozenThread && mFrozenThread != QThread::currentThread();
}

void UBPersistenceJournal::load()
{
    const QDir root(mRoot);
    const QFileInfoList groups = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& groupInfo : groups)
    {
        const QString group = groupInfo.absoluteFilePath();
        QFile index(group + "/" + indexFileName);
        bool hasEntries = false;

        if (index.open(QIODevice::ReadOnly))
        {
            const QList<QByteArray> lines = index.readAll().split('\n');
            index.close();

            if (lines.size() >= 2 && lines.at(0).trimmed() == indexHeader)
            {
                const QString targetDirectory = QString::fromUtf8(lines.at(1)).trimmed();

                for (int i = 2; i < lines.size(); ++i)
                {
                    const QList<QByteArray> fields = lines.at(i).trimmed().split('\t');
                    const QString fileName = QString::fromUtf8(fields.value(0));

                    if (fileName.isEmpty() || !QFile::exists(group + "/" + fileName))
                    {
                        continue;
                    }

                    mEntries.insert(targetDirectory + "/" + fileName, {group + "/" + fileName, QString::fromUtf8(fields.value(1))});
                    hasEntries = true;
                }
            }
        }

        if (!hasEntries)
        {
            QDir(group).removeRecursively();
        }
    }

    if (!mEntries.isEmpty())
    {
        qWarning() << mEntries.size() << "file(s) waiting in the recovery directory" << mRoot;
    }
}

bool UBPersistenceJournal::saveIndex(const QString& targetDirectory)
{
    // must be called with the mutex locked
    const QString group = groupDirectory(targetDirectory);
    QByteArray content = indexHeader + "\n" + targetDirectory.toUtf8() + "\n";
    bool hasEntries = false;

    for (auto it = mEntries.cbegin(); it != mEntries.cend(); ++it)
    {
        if (parentDirectory(it.key()) == targetDirectory)
        {
            content += fileNameOf(it.key()).toUtf8() + "\t" + it.value().base.toUtf8() + "\n";
            hasEntries = true;
        }
    }

    if (!hasEntries)
    {
        return QDir(group).removeRecursively();
    }

    return writeFile(group + "/" + indexFileName, content);
}
