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



#ifndef UBPERSISTENCEWORKER_H
#define UBPERSISTENCEWORKER_H

#include <atomic>

#include <QObject>
#include <QElapsedTimer>
#include <QMutex>
#include <QSemaphore>
#include <QWaitCondition>
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"

typedef enum{
    WriteScene = 0,
    WriteMetadata,
    WriteFile,
    ReplayJournal,
    WriteThumbnail
}ActionType;

struct PersistenceInformation
{
    ActionType action;
    std::shared_ptr<UBDocumentProxy> proxy;
    UBGraphicsScene* scene;
    int sceneIndex;
    QString path;       // WriteFile, WriteThumbnail: target file, ReplayJournal: root directory
    QByteArray data;    // WriteFile, WriteThumbnail: content
    quint64 sequence{0}; // order of the saves, see nextSequence()
};

/**
 * Writes pages, metadata and other files in a separate thread.
 *
 * Every file is first stored in the local journal (see UBPersistenceJournal) and then
 * written to the document storage. If writing fails, the local copy is kept and written
 * later by ReplayJournal.
 */
class UBPersistenceWorker : public QObject
{
    Q_OBJECT
public:
    explicit UBPersistenceWorker(QObject *parent = 0);

    void saveScene(std::shared_ptr<UBDocumentProxy> proxy, UBGraphicsScene* scene, const int pageIndex);
    void saveMetadata(std::shared_ptr<UBDocumentProxy> proxy);
    void saveFile(const QString& fileName, const QByteArray& data);
    void replayJournal(const QString& rootDirectory);
    // written without local copy, failures are ignored
    void saveThumbnail(const QString& fileName, const QByteArray& data);

    // increasing number given to each save, orders the versions of a file in the journal
    static quint64 nextSequence();

    // Block until all pending saves are written or the timeout (ms, -1 for none) expired.
    // Returns false on timeout. Must not be called from the worker thread.
    bool waitForAllSaved(int timeoutMs = -1);

    // milliseconds the current save is already running, 0 if idle
    qint64 busyDuration() const;

    // the save running right now, returns false if none
    bool saveInProgress(PersistenceInformation& info) const;

    // remove and return the saves which are not started yet
    QList<PersistenceInformation> takePendingSaves();

    // put saves taken with takePendingSaves() back in front of the queue
    void requeue(const QList<PersistenceInformation>& pending);

    // writing is retried only while the storage is available
    void setStorageAvailable(bool available);

signals:
   void finished();
   void error(QString string);
   void scenePersisted(UBGraphicsScene* scene);
   void saveFailed(UBGraphicsScene* scene, const QString& path);
   void saveSucceeded();
   void metadataPersisted(std::shared_ptr<UBDocumentProxy> proxy);
   void journalReplayed(bool complete);
   void thumbnailWritten(const QString& fileName);

private:
   bool persist(const QString& fileName, const QByteArray& data, bool createDirectory, quint64 sequence);
   bool write(const QString& fileName, const QByteArray& data, bool createDirectory);
   bool replay(const QString& rootDirectory);
   void enqueue(PersistenceInformation entry);

public slots:
   void process();
   void applicationWillClose();

protected:
   bool mReceivedApplicationClosing;
   bool mBusy{false};
   PersistenceInformation mCurrent{};
   QElapsedTimer mBusyTimer;
   std::atomic<bool> mStorageAvailable{true};
   QSemaphore mSemaphore;
   mutable QMutex mMutex;
   QWaitCondition mAllSaved;
   QList<PersistenceInformation> saves;
};

#endif // UBPERSISTENCEWORKER_H
