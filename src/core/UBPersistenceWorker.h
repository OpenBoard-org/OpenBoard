/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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

#include <QObject>
#include <QSemaphore>
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"

typedef enum{
    WriteScene = 0,
    ReadScene,
    WriteMetadata
}ActionType;

typedef struct{
    ActionType action;
    UBDocumentProxy* proxy;
    UBGraphicsScene* scene;
    int sceneIndex;
}PersistenceInformation;

class UBPersistenceWorker : public QObject
{
    Q_OBJECT
public:
    explicit UBPersistenceWorker(QObject *parent = 0);

    void saveScene(UBDocumentProxy* proxy, UBGraphicsScene* scene, const int pageIndex);
    void readScene(UBDocumentProxy* proxy, const int pageIndex);
    void saveMetadata(UBDocumentProxy* proxy);

signals:
   void finished();
   void error(QString string);
   void sceneLoaded(QByteArray text,UBDocumentProxy* proxy, const int pageIndex);
   void scenePersisted(UBGraphicsScene* scene);
   void metadataPersisted(UBDocumentProxy* proxy);

public slots:
   void process();
   void applicationWillClose();

protected:
   bool mReceivedApplicationClosing;
   QSemaphore mSemaphore;
   QList<PersistenceInformation> saves;
};

#endif // UBPERSISTENCEWORKER_H
