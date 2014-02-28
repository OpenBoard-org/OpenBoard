/*
 * Copyright (C) 2013-2014 Open Education Foundation
 *
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
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"

UBPersistenceWorker::UBPersistenceWorker(QObject *parent) :
    QObject(parent)
  , mReceivedApplicationClosing(false)
{
}

void UBPersistenceWorker::saveScene(UBDocumentProxy* proxy, UBGraphicsScene* scene, const int pageIndex)
{
    saves.append({WriteScene,proxy,scene,pageIndex});
    mSemaphore.release();
}

void UBPersistenceWorker::readScene(UBDocumentProxy* proxy, const int pageIndex)
{
    saves.append({ReadScene,proxy,0,pageIndex});
    mSemaphore.release();
}

void UBPersistenceWorker::applicationWillClose()
{
    qDebug() << "applicaiton Will close signal received";
    mReceivedApplicationClosing = true;
    mSemaphore.release();
}

void UBPersistenceWorker::process()
{
    qDebug() << "process starts";
    mSemaphore.acquire();
    do{
        PersistenceInformation info = saves.takeFirst();
        if(info.action == WriteScene){
            UBSvgSubsetAdaptor::persistScene(info.proxy, info.scene, info.sceneIndex);
            delete info.scene;
            info.scene = NULL;
        }
        else{
            emit sceneLoaded(UBSvgSubsetAdaptor::loadSceneAsText(info.proxy,info.sceneIndex), info.proxy, info.sceneIndex);
        }
        mSemaphore.acquire();
    }while(!mReceivedApplicationClosing);
    qDebug() << "process will stop";
    emit finished();
}
