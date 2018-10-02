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



#include "UBGraphicsItemAction.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "board/UBBoardController.h"
#include "web/UBWebController.h"
#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"
#include "document/UBDocumentContainer.h"

#include "board/UBBoardController.h"


UBGraphicsItemAction::UBGraphicsItemAction(eUBGraphicsItemLinkType linkType, QObject *parent) :
    QObject(parent)
{
    mLinkType = linkType;
}

 void UBGraphicsItemAction::actionRemoved()
 {
    //NOOP
 }


UBGraphicsItemPlayAudioAction::UBGraphicsItemPlayAudioAction(QString audioFile, bool onImport, QObject *parent) :
    UBGraphicsItemAction(eLinkToAudio,parent)
  , mMediaObject(0)
  , mIsLoading(true)
{
    Q_ASSERT(audioFile.length() > 0);

    if(onImport){
        QString extension = QFileInfo(audioFile).completeSuffix();
        QString destDir = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + UBPersistenceManager::audioDirectory;
        QString destFile = destDir + "/" + QUuid::createUuid().toString() + "." + extension;
        if(!QDir(destDir).exists())
            QDir(UBApplication::boardController->selectedDocument()->persistencePath()).mkdir(destDir);
        //explanation : the audioFile could be relative. The method copy will return false so a second try is done adding
        // the document file path
        if(!QFile(audioFile).copy(destFile))
            QFile(UBApplication::boardController->selectedDocument()->persistencePath() + "/" + audioFile).copy(destFile);
        mAudioPath = destFile;
        mFullPath = destFile;
    }
    else
    {
        //On import don't recreate the file
        mAudioPath = audioFile;
        mFullPath = mAudioPath;
    }

    mAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    mMediaObject = new Phonon::MediaObject(this);
    Phonon::createPath(mMediaObject, mAudioOutput);
    mMediaObject->setCurrentSource(Phonon::MediaSource(mAudioPath));
}


UBGraphicsItemPlayAudioAction::UBGraphicsItemPlayAudioAction() :
    UBGraphicsItemAction(eLinkToAudio,NULL)
  , mMediaObject(0)
  , mIsLoading(true)
{
}


void UBGraphicsItemPlayAudioAction::setPath(QString audioPath)
{
    Q_ASSERT(audioPath.length() > 0);
    mAudioPath = audioPath;
    mFullPath = mAudioPath;
    mAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    mMediaObject = new Phonon::MediaObject(this);
    Phonon::createPath(mMediaObject, mAudioOutput);
    mMediaObject->setCurrentSource(Phonon::MediaSource(mAudioPath));
}

QString UBGraphicsItemPlayAudioAction::fullPath()
{
    return mFullPath;
}

UBGraphicsItemPlayAudioAction::~UBGraphicsItemPlayAudioAction()
{
    if(!mMediaObject && mMediaObject->state() == Phonon::PlayingState)
        mMediaObject->stop();
}

void UBGraphicsItemPlayAudioAction::onSourceHide()
{
    if(mMediaObject && mMediaObject->state() == Phonon::PlayingState){
        mMediaObject->stop();
    }
}

void UBGraphicsItemPlayAudioAction::play()
{
    if(mMediaObject->state() == Phonon::PlayingState){
        mMediaObject->stop();
    }
    mMediaObject->seek(0);
    mMediaObject->play();
}


QStringList UBGraphicsItemPlayAudioAction::save()
{
    //Another hack
    if(UBApplication::documentController && UBApplication::documentController->selectedDocument()){
        QString documentPath = UBApplication::documentController->selectedDocument()->persistencePath() + "/";
        return QStringList() << QString("%1").arg(eLinkToAudio) <<  mAudioPath.replace(documentPath,"");
    }
    else{
        int index = mAudioPath.indexOf("/audios/");
        QString relativePath = mAudioPath.remove(0,index + 1);
        return QStringList() << QString("%1").arg(eLinkToAudio) <<  relativePath;
    }
}

void UBGraphicsItemPlayAudioAction::actionRemoved()
{
    QFile(mAudioPath).remove();
}


UBGraphicsItemMoveToPageAction::UBGraphicsItemMoveToPageAction(eUBGraphicsItemMovePageAction actionType, int page, QObject* parent) :
    UBGraphicsItemAction(eLinkToPage,parent)
{
    mActionType = actionType;
    mPage = page;
}

void UBGraphicsItemMoveToPageAction::play()
{
    UBBoardController* boardController = UBApplication::boardController;

    switch (mActionType) {
    case eMoveToFirstPage:
        boardController->firstScene();
        break;
    case eMoveToLastPage:
        boardController->lastScene();
        break;
    case eMoveToPreviousPage:
        boardController->previousScene();
        break;
    case eMoveToNextPage:
        boardController->nextScene();
        break;
    case eMoveToPage:
        if(mPage >= 0 && mPage < boardController->pageCount())
            boardController->setActiveDocumentScene(mPage);
        else
            qWarning() << "scene number " << mPage << "ins't accessible anymore";
        break;
    default:
        break;
    }
}

QStringList UBGraphicsItemMoveToPageAction::save()
{
    return QStringList() << QString("%1").arg(eLinkToPage) << QString("%1").arg(mActionType) << QString("%1").arg(mPage);
}


UBGraphicsItemLinkToWebPageAction::UBGraphicsItemLinkToWebPageAction(QString url, QObject *parent) :
    UBGraphicsItemAction(eLinkToWebUrl,parent)
{
    if (url.length() > 0)
    {
        if(!url.startsWith("http://"))
            url = "http://" + url;
        mUrl = url;
    }
    else
        mUrl = QString();
}

void UBGraphicsItemLinkToWebPageAction::play()
{
    if (mUrl.length() > 0)
        UBApplication::webController->loadUrl(QUrl(mUrl));
}

QStringList UBGraphicsItemLinkToWebPageAction::save()
{
    return QStringList() << QString("%1").arg(eLinkToWebUrl) << mUrl;
}
