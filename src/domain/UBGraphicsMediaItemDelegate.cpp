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

UBGraphicsMediaItemDelegate::UBGraphicsMediaItemDelegate(UBGraphicsMediaItem* pDelegated, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated, parent, GF_COMMON
                             | GF_RESPECT_RATIO
                             | GF_TOOLBAR_USED)
    , mPlayPauseButton(NULL)
    , mToolBarShowTimer(NULL)
    , m_iToolBarShowingInterval(5000)
{
    QPalette palette;
    palette.setBrush ( QPalette::Light, Qt::darkGray );

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

/**
 * @brief Show the toolbar (play/pause, seek, mute).
 *
 * The toolbar then auto-hides after a set amount of time, if the video is currently
 * playing or is paused.
 */
void UBGraphicsMediaItemDelegate::showToolBar(bool autohide)
{
    mToolBarItem->show();
    if (mToolBarShowTimer) {

        if (delegated()->isPlaying() || delegated()->isPaused())
            mToolBarShowTimer->start();
        else
            mToolBarShowTimer->stop();

        // Don't hide the toolbar if we're at the beginning of the video
        if (delegated()->mediaPosition() == delegated()->initialPos())
            mToolBarShowTimer->stop();

        // Don't hide the toolbar if it was explicitly requested
        if (!autohide)
            mToolBarShowTimer->stop();
    }
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
                delegated(), SLOT(stop()));

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

        if (!mToolBarShowTimer) {
            if (delegated()->hasLinkedImage()) {
                mToolBarShowTimer = new QTimer();
                mToolBarShowTimer->setInterval(m_iToolBarShowingInterval);
                connect(mToolBarShowTimer, SIGNAL(timeout()), this, SLOT(hideToolBar()));
            }
        }

        else {
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
    if (delegated())
        delegated()->stop();

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
    if (delegated())
        delegated()->togglePlayPause();
}

void UBGraphicsMediaItemDelegate::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    // Possible statuses are: UnknownMediaStatus, NoMedia, LoadingMedia, LoadedMedia,
    // StalledMedia, BufferingMedia, BufferedMedia, EndOfMedia, InvalidMedia

    //qDebug() << "Media status changed to " << status << "; state: " << delegated()->playerState();

    if (status == QMediaPlayer::LoadedMedia)
        mMediaControl->totalTimeChanged(delegated()->mediaDuration());

    // At the beginning of the video, play/pause to load and display the first frame (not working on OSX)
#ifndef Q_OS_OSX
    if ((status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia)
            && delegated()->mediaPosition() == delegated()->initialPos()
            && !delegated()->isStopped()
            && delegated()->firstLoad()
            )
    {
        delegated()->play();
        delegated()->pause();
        delegated()->setFirstLoad(false);
    }
#endif

    // At the end of the video, make sure the progress bar doesn't autohide
    if (status == QMediaPlayer::EndOfMedia)
    {
        delegated()->setFirstLoad(true);
        showToolBar(false);
    }


    // in most cases, the only necessary action is to update the play/pause state
    updatePlayPauseState();
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void UBGraphicsMediaItemDelegate::mediaStateChanged(QMediaPlayer::PlaybackState state)
#else
void UBGraphicsMediaItemDelegate::mediaStateChanged(QMediaPlayer::State state)
#endif
{
    Q_UNUSED(state);
    // Possible states are StoppedState, PlayingState and PausedState

    // updatePlayPauseState handles this functionality
    if (state == QMediaPlayer::StoppedState)
    {
        delegated()->setMediaPos(0);
#ifdef Q_OS_OSX //media positionChanged signal is not always called in osx
        mMediaControl->updateTicker(0);
#endif
    }
    updatePlayPauseState();
}


void UBGraphicsMediaItemDelegate::updatePlayPauseState()
{
    if (delegated()->playerState() == QMediaPlayer::PlayingState)
        mPlayPauseButton->setFileName(":/images/pause.svg");
    else
        mPlayPauseButton->setFileName(":/images/play.svg");
}


void UBGraphicsMediaItemDelegate::updateTicker(qint64 time)
{
    if (!delegated()->isStopped())
    {
        mMediaControl->totalTimeChanged(delegated()->mediaDuration());
        mMediaControl->updateTicker(time);
    }
}


void UBGraphicsMediaItemDelegate::totalTimeChanged(qint64 newTotalTime)
{
    mMediaControl->totalTimeChanged(newTotalTime);
}

void UBGraphicsMediaItemDelegate::showHide(bool show)
{
    QVariant showFlag = QVariant(show ? UBItemLayerType::Object : UBItemLayerType::Control);
    showHideRecurs(showFlag, mDelegated);
    mDelegated->update();

    emit showOnDisplayChanged(show);
}
