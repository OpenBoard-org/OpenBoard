/*
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




#include <QtGui>
#include <QtSvg>

#include "UBGraphicsMediaItem.h"
#include "UBGraphicsMediaItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"

#include "UBGraphicsScene.h"

#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "domain/UBGraphicsMediaItem.h"

#include "core/memcheck.h"

UBGraphicsMediaItemDelegate::UBGraphicsMediaItemDelegate(UBGraphicsMediaItem* pDelegated, QMediaPlayer* pMedia, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated, parent, GF_COMMON
                             | GF_RESPECT_RATIO
                             | GF_TOOLBAR_USED
                             | GF_SHOW_CONTENT_SOURCE)
    , mPlayPauseButton(NULL)
    , mMedia(pMedia)
    , mToolBarShowTimer(NULL)
    , m_iToolBarShowingInterval(5000)
{
    QPalette palette;
    palette.setBrush ( QPalette::Light, Qt::darkGray );

    mMedia->setNotifyInterval(50);

    connect(mMedia, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));

    connect(mMedia, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(mediaStateChanged(QMediaPlayer::State)));

    connect(mMedia, static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
        this, &UBGraphicsMediaItemDelegate::mediaError);

    connect(mMedia, SIGNAL(positionChanged(qint64)),
            this, SLOT(updateTicker(qint64)));

    connect(mMedia, SIGNAL(durationChanged(qint64)),
            this, SLOT(totalTimeChanged(qint64)));

    if (delegated()->hasLinkedImage())
    {
        mToolBarShowTimer = new QTimer();
        connect(mToolBarShowTimer, SIGNAL(timeout()), this, SLOT(hideToolBar()));
        mToolBarShowTimer->setInterval(m_iToolBarShowingInterval);
    }

    if (delegated()->isMuted())
        delegated()->setMute(true);

}

bool UBGraphicsMediaItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    mToolBarItem->show();
    positionHandles();

    if (mToolBarShowTimer)
        mToolBarShowTimer->start();

    return UBGraphicsItemDelegate::mousePressEvent(event);
}

void UBGraphicsMediaItemDelegate::hideToolBar()
{
    mToolBarItem->hide();
}

void UBGraphicsMediaItemDelegate::buildButtons()
{
    if(!mPlayPauseButton){
        mPlayPauseButton = new DelegateButton(":/images/play.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
        connect(mPlayPauseButton, SIGNAL(clicked(bool)),
                this, SLOT(togglePlayPause()));

        mStopButton = new DelegateButton(":/images/stop.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
        connect(mStopButton, SIGNAL(clicked(bool)),
                mMedia, SLOT(stop()));

        mMediaControl = new DelegateMediaControl(delegated(), mToolBarItem);
        mMediaControl->setFlag(QGraphicsItem::ItemIsSelectable, true);
        UBGraphicsItem::assignZValue(mMediaControl, delegated()->zValue());

        if (delegated()->isMuted())
            mMuteButton = new DelegateButton(":/images/soundOff.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
        else
            mMuteButton = new DelegateButton(":/images/soundOn.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

        connect(mMuteButton, SIGNAL(clicked(bool)),
                delegated(), SLOT(toggleMute()));
        connect(mMuteButton, SIGNAL(clicked(bool)),
                this, SLOT(toggleMute())); // for changing button image

        mToolBarButtons << mPlayPauseButton << mStopButton << mMuteButton;

        mToolBarItem->setItemsOnToolBar(QList<QGraphicsItem*>() << mPlayPauseButton << mStopButton << mMediaControl  << mMuteButton );
        mToolBarItem->setVisibleOnBoard(true);
        mToolBarItem->setShifting(false);

        if (mToolBarShowTimer)
        {
            connect(mPlayPauseButton, SIGNAL(clicked(bool)),
                    mToolBarShowTimer, SLOT(start()));

            connect(mStopButton, SIGNAL(clicked(bool)),
                    mToolBarShowTimer, SLOT(start()));

            connect(mMediaControl, SIGNAL(used()),
                    mToolBarShowTimer, SLOT(start()));

            connect(mMuteButton, SIGNAL(clicked(bool)),
                    mToolBarShowTimer, SLOT(start()));
        }


        positionHandles();
    }
}

UBGraphicsMediaItemDelegate::~UBGraphicsMediaItemDelegate()
{
    if (mToolBarShowTimer){
        delete mToolBarShowTimer;
        mToolBarShowTimer = NULL;
    }
}

void UBGraphicsMediaItemDelegate::positionHandles()
{
    UBGraphicsItemDelegate::positionHandles();

    UBGraphicsMediaItem *mediaItem = dynamic_cast<UBGraphicsMediaItem*>(mDelegated);
    if (mediaItem)
    {
        QRectF toolBarRect = mToolBarItem->rect();

        mToolBarItem->setPos(0, mediaItem->boundingRect().height()-mToolBarItem->rect().height());

        toolBarRect.setWidth(mediaItem->boundingRect().width());
        mToolBarItem->show();

        mToolBarItem->setRect(toolBarRect);
    }

    int toolBarButtonsWidth = 0;
    foreach (DelegateButton* button, mToolBarButtons)
        toolBarButtonsWidth += button->boundingRect().width() + mToolBarItem->getElementsPadding();

    QRectF mediaItemRect = mMediaControl->rect();
    mediaItemRect.setWidth(mediaItem->boundingRect().width() - toolBarButtonsWidth);
    mediaItemRect.setHeight(mToolBarItem->boundingRect().height());
    mMediaControl->setRect(mediaItemRect);

    mToolBarItem->positionHandles();
    mMediaControl->positionHandles();

    if (mediaItem)
        mToolBarItem->show();
}

void UBGraphicsMediaItemDelegate::remove(bool canUndo)
{
    if (delegated() && delegated()->mediaObject())
        delegated()->mediaObject()->stop();

    UBGraphicsItemDelegate::remove(canUndo);
}


void UBGraphicsMediaItemDelegate::toggleMute()
{
    if (delegated()->isMuted())
        mMuteButton->setFileName(":/images/soundOff.svg");
    else
        mMuteButton->setFileName(":/images/soundOn.svg");
}


UBGraphicsMediaItem* UBGraphicsMediaItemDelegate::delegated()
{
    return dynamic_cast<UBGraphicsMediaItem*>(mDelegated);
}

void UBGraphicsMediaItemDelegate::togglePlayPause()
{
    if (delegated() && delegated()->mediaObject()) {

        QMediaPlayer * media = delegated()->mediaObject();

        if (media->state() == QMediaPlayer::StoppedState)
            media->play();

        else if (media->state() == QMediaPlayer::PlayingState) {

            if ((media->duration() - media->position()) <= 0) {
                media->stop();
                media->play();
            }

            else {
                media->pause();
                if(delegated()->scene())
                        delegated()->scene()->setModified(true);
            }
        }

        else if (media->state() == QMediaPlayer::PausedState) {
            if ((media->duration() - media->position()) <= 0)
                media->stop();

            media->play();
        }

        else  if ( media->mediaStatus() == QMediaPlayer::LoadingMedia) {
            delegated()->mediaObject()->setMedia(delegated()->mediaFileUrl());
            media->play();
        }

        else if (media->error())
            qDebug() << "Error appeared." << media->errorString();

    }
}

void UBGraphicsMediaItemDelegate::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    // Possible statuses are: UnknownMediaStatus, NoMedia, LoadingMedia, LoadedMedia,
    // StalledMedia, BufferingMedia, BufferedMedia, EndOfMedia, InvalidMedia

    if (status == QMediaPlayer::LoadedMedia)
        mMediaControl->totalTimeChanged(delegated()->mediaObject()->duration());

    // in most cases, the only necessary action is to update the play/pause state
    updatePlayPauseState();
}

void UBGraphicsMediaItemDelegate::mediaStateChanged(QMediaPlayer::State state)
{
    // Possible states are StoppedState, PlayingState and PausedState

    // updatePlayPauseState handles this functionality
    updatePlayPauseState();
}

void UBGraphicsMediaItemDelegate::mediaError(QMediaPlayer::Error error)
{
    // Possible errors are NoError, ResourceError, FormatError, NetworkError, AccessDeniedError,
    // ServiceMissingError
    Q_UNUSED(error);

    qDebug() << "Error appeared." << mMedia->errorString();
}


void UBGraphicsMediaItemDelegate::updatePlayPauseState()
{
    QMediaPlayer * media = delegated()->mediaObject();

    if (media->state() == QMediaPlayer::PlayingState)
        mPlayPauseButton->setFileName(":/images/pause.svg");
    else
        mPlayPauseButton->setFileName(":/images/play.svg");
}


void UBGraphicsMediaItemDelegate::updateTicker(qint64 time)
{
    QMediaPlayer* media = delegated()->mediaObject();
    mMediaControl->totalTimeChanged(media->duration());
    mMediaControl->updateTicker(time);
}


void UBGraphicsMediaItemDelegate::totalTimeChanged(qint64 newTotalTime)
{
    mMediaControl->totalTimeChanged(newTotalTime);
}
