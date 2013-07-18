/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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

UBGraphicsMediaItemDelegate::UBGraphicsMediaItemDelegate(UBGraphicsMediaItem* pDelegated, Phonon::MediaObject* pMedia, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated, parent, GF_COMMON
                             | GF_RESPECT_RATIO
                             | GF_TOOLBAR_USED
                             | GF_SHOW_CONTENT_SOURCE)
    , mMedia(pMedia)
    , mToolBarShowTimer(NULL)
    , m_iToolBarShowingInterval(5000)
{
    QPalette palette;
    palette.setBrush ( QPalette::Light, Qt::darkGray );

    mMedia->setTickInterval(50);
    connect(mMedia, SIGNAL(stateChanged (Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged (Phonon::State, Phonon::State)));
    connect(mMedia, SIGNAL(finished()), this, SLOT(updatePlayPauseState()));
    connect(mMedia, SIGNAL(tick(qint64)), this, SLOT(updateTicker(qint64)));
    connect(mMedia, SIGNAL(totalTimeChanged(qint64)), this, SLOT(totalTimeChanged(qint64)));

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
    mPlayPauseButton = new DelegateButton(":/images/play.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    connect(mPlayPauseButton, SIGNAL(clicked(bool)), this, SLOT(togglePlayPause()));

    mStopButton = new DelegateButton(":/images/stop.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    connect(mStopButton, SIGNAL(clicked(bool)), mMedia, SLOT(stop()));

    mMediaControl = new DelegateMediaControl(delegated(), mToolBarItem);
    mMediaControl->setFlag(QGraphicsItem::ItemIsSelectable, true);
    UBGraphicsItem::assignZValue(mMediaControl, delegated()->zValue());

    if (delegated()->isMuted())
        mMuteButton = new DelegateButton(":/images/soundOff.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    else
        mMuteButton = new DelegateButton(":/images/soundOn.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

    connect(mMuteButton, SIGNAL(clicked(bool)), delegated(), SLOT(toggleMute()));
    connect(mMuteButton, SIGNAL(clicked(bool)), this, SLOT(toggleMute())); // for changing button image

    mToolBarButtons << mPlayPauseButton << mStopButton << mMuteButton;

    mToolBarItem->setItemsOnToolBar(QList<QGraphicsItem*>() << mPlayPauseButton << mStopButton << mMediaControl << mMuteButton);
    mToolBarItem->setVisibleOnBoard(true);
    mToolBarItem->setShifting(false);

    if (mToolBarShowTimer)
    {
        connect(mPlayPauseButton, SIGNAL(clicked(bool)), mToolBarShowTimer, SLOT(start()));
        connect(mStopButton, SIGNAL(clicked(bool)), mToolBarShowTimer, SLOT(start()));
        connect(mMediaControl, SIGNAL(used()), mToolBarShowTimer, SLOT(start()));
        connect(mMuteButton, SIGNAL(clicked(bool)), mToolBarShowTimer, SLOT(start()));
    }


    positionHandles();
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

    int toolBarButtons= 0;
    foreach (DelegateButton* button, mToolBarButtons)
        toolBarButtons += button->boundingRect().width() + mToolBarItem->getElementsPadding();

    QRectF mediaItemRect = mMediaControl->rect();
    mediaItemRect.setWidth(mediaItem->boundingRect().width() - toolBarButtons);
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

        Phonon::MediaObject* media = delegated()->mediaObject();
        if (media->state() == Phonon::StoppedState) {
            media->play();
        } else if (media->state() == Phonon::PlayingState) {
            if (media->remainingTime() <= 0) {
                media->stop();
                media->play();
            } else {
                media->pause();
                if(delegated()->scene())
                        delegated()->scene()->setModified(true);
            }
        } else if (media->state() == Phonon::PausedState) {
            if (media->remainingTime() <= 0) {
                media->stop();
            }
            media->play();
        } else  if ( media->state() == Phonon::LoadingState ) {
            delegated()->mediaObject()->setCurrentSource(delegated()->mediaFileUrl());
            media->play();
        } else if (media->state() == Phonon::ErrorState){
            qDebug() << "Error appeared." << media->errorString();
        }
    }
}

void UBGraphicsMediaItemDelegate::mediaStateChanged ( Phonon::State newstate, Phonon::State oldstate )
{
    Q_UNUSED(newstate);
    Q_UNUSED(oldstate);

    if (oldstate == Phonon::LoadingState)
    {
        mMediaControl->totalTimeChanged(delegated()->mediaObject()->totalTime());
    }
    updatePlayPauseState();
}


void UBGraphicsMediaItemDelegate::updatePlayPauseState()
{
    Phonon::MediaObject* media = delegated()->mediaObject();

    if (media->state() == Phonon::PlayingState)
        mPlayPauseButton->setFileName(":/images/pause.svg");
    else
        mPlayPauseButton->setFileName(":/images/play.svg");
}


void UBGraphicsMediaItemDelegate::updateTicker(qint64 time)
{
    Phonon::MediaObject* media = delegated()->mediaObject();
    mMediaControl->totalTimeChanged(media->totalTime());
    mMediaControl->updateTicker(time);
}


void UBGraphicsMediaItemDelegate::totalTimeChanged(qint64 newTotalTime)
{
    mMediaControl->totalTimeChanged(newTotalTime);
}
