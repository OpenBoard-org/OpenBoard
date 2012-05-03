/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGui>
#include <QtSvg>

#include "UBGraphicsVideoItemDelegate.h"

#include "UBGraphicsScene.h"

#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "domain/UBGraphicsVideoItem.h"
#include "domain/UBGraphicsDelegateFrame.h"

#include "core/memcheck.h"

UBGraphicsVideoItemDelegate::UBGraphicsVideoItemDelegate(UBGraphicsVideoItem* pDelegated, Phonon::MediaObject* pMedia, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated, parent, true, false)
    , mMedia(pMedia)
{
    // NOOP
}

void UBGraphicsVideoItemDelegate::buildButtons()
{
    mPlayPauseButton = new DelegateButton(":/images/play.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

    mStopButton = new DelegateButton(":/images/stop.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

    mVideoControl = new DelegateVideoControl(delegated(), mToolBarItem);
    UBGraphicsItem::assignZValue(mVideoControl, delegated()->zValue());
    mVideoControl->setFlag(QGraphicsItem::ItemIsSelectable, true);

    if (delegated()->isMuted())
        mMuteButton = new DelegateButton(":/images/soundOff.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    else
        mMuteButton = new DelegateButton(":/images/soundOn.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

    connect(mPlayPauseButton, SIGNAL(clicked(bool)), this, SLOT(togglePlayPause()));
    connect(mStopButton, SIGNAL(clicked(bool)), mMedia, SLOT(stop()));
    connect(mMuteButton, SIGNAL(clicked(bool)), delegated(), SLOT(toggleMute()));
    connect(mMuteButton, SIGNAL(clicked(bool)), this, SLOT(toggleMute()));

    mButtons << mPlayPauseButton << mStopButton << mMuteButton;

    QList<QGraphicsItem*> itemsOnToolBar;
    itemsOnToolBar << mPlayPauseButton << mStopButton << mVideoControl << mMuteButton;
    mToolBarItem->setItemsOnToolBar(itemsOnToolBar);

    mMedia->setTickInterval(50);

    connect(mMedia, SIGNAL(stateChanged (Phonon::State, Phonon::State)), this, SLOT(mediaStateChanged (Phonon::State, Phonon::State)));
    connect(mMedia, SIGNAL(finished()), this, SLOT(updatePlayPauseState()));
    connect(mMedia, SIGNAL(tick(qint64)), this, SLOT(updateTicker(qint64)));
    connect(mMedia, SIGNAL(totalTimeChanged(qint64)), this, SLOT(totalTimeChanged(qint64)));

    mToolBarItem->setVisibleOnBoard(true);
    mToolBarItem->setShifting(false);
}


UBGraphicsVideoItemDelegate::~UBGraphicsVideoItemDelegate()
{
    //NOOP
}


void UBGraphicsVideoItemDelegate::positionHandles()
{
    UBGraphicsItemDelegate::positionHandles();

    if (mDelegated->isSelected())
    {
        qreal scaledFrameWidth = mFrameWidth * mAntiScaleRatio;

        int offset = 0;
        foreach (DelegateButton* button, mButtons)
        {
            if (button->getSection() == Qt::TitleBarArea)
                offset += button->boundingRect().width() * mAntiScaleRatio;
        }

        mVideoControl->setRect(mVideoControl->rect().x()
                , scaledFrameWidth/6 - 0.5
                , (mToolBarItem->rect().width() - 35 - offset) / mAntiScaleRatio 
                , (2 * scaledFrameWidth) / mAntiScaleRatio);

        offset += (mVideoControl->rect().width() + 5) * mAntiScaleRatio;
        mMuteButton->setPos(offset, 0);

        if (!mVideoControl->scene())
        {
            mVideoControl->setParentItem(mToolBarItem);//update parent for the case the item has been previously removed from scene
            mDelegated->scene()->addItem(mVideoControl);
        }

        mVideoControl->setAntiScale(mAntiScaleRatio);
        mVideoControl->setZValue(delegated()->zValue());
        mVideoControl->show();
    }
    else
    {
        mVideoControl->hide();
    }
}


void UBGraphicsVideoItemDelegate::remove(bool canUndo)
{
    if (delegated() && delegated()->mediaObject())
        delegated()->mediaObject()->stop();

    QGraphicsScene* scene = mDelegated->scene();

    scene->removeItem(mVideoControl);

    UBGraphicsItemDelegate::remove(canUndo);
}


void UBGraphicsVideoItemDelegate::toggleMute()
{
    if (delegated()->isMuted())
        mMuteButton->setFileName(":/images/soundOff.svg");
    else
        mMuteButton->setFileName(":/images/soundOn.svg");

}


UBGraphicsVideoItem* UBGraphicsVideoItemDelegate::delegated()
{
    return static_cast<UBGraphicsVideoItem*>(mDelegated);
}


void UBGraphicsVideoItemDelegate::togglePlayPause()
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

void UBGraphicsVideoItemDelegate::mediaStateChanged ( Phonon::State newstate, Phonon::State oldstate )
{
    Q_UNUSED(newstate);
    Q_UNUSED(oldstate);
    updatePlayPauseState();
}


void UBGraphicsVideoItemDelegate::updatePlayPauseState()
{
    Phonon::MediaObject* media = delegated()->mediaObject();

    if (media->state() == Phonon::PlayingState)
        mPlayPauseButton->setFileName(":/images/pause.svg");
    else
        mPlayPauseButton->setFileName(":/images/play.svg");
}


void UBGraphicsVideoItemDelegate::updateTicker(qint64 time)
{
    Phonon::MediaObject* media = delegated()->mediaObject();
    mVideoControl->totalTimeChanged(media->totalTime());

    mVideoControl->updateTicker(time);
}


void UBGraphicsVideoItemDelegate::totalTimeChanged(qint64 newTotalTime)
{
    mVideoControl->totalTimeChanged(newTotalTime);
}


DelegateVideoControl::DelegateVideoControl(UBGraphicsVideoItem* pDelegated, QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , mDelegate(pDelegated)
    , mDisplayCurrentTime(false)
    , mAntiScale(1.0)
    , mCurrentTimeInMs(0)
    , mTotalTimeInMs(0)
    , mStartWidth(200)
{
    setAcceptedMouseButtons(Qt::LeftButton);

    setBrush(QBrush(Qt::white));
    setPen(Qt::NoPen);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    QRectF rect = this->rect();
    rect.setWidth(mStartWidth);
    this->setRect(rect);
}


void DelegateVideoControl::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->fillPath(shape(), brush());

    qreal frameWidth = rect().height() / 2;
    int position = frameWidth;

    if (mTotalTimeInMs > 0)
    {
        position = frameWidth + (rect().width() - (2 * frameWidth)) / mTotalTimeInMs * mCurrentTimeInMs;
    }

    int radius = rect().height() / 6;
    QRectF r(rect().x() + position - radius, rect().y() + (rect().height() / 4) - radius, radius * 2, radius * 2);

    painter->setBrush(UBSettings::documentViewLightColor);
    painter->drawEllipse(r);

    if(mDisplayCurrentTime)
    {
        painter->setBrush(UBSettings::paletteColor);
        painter->setPen(QPen(Qt::NoPen));
        mBalloon.setRect(rect().x() + position - frameWidth, rect().y() - (frameWidth * 1.2), 2 * frameWidth, frameWidth);
        painter->drawRoundedRect(mBalloon, frameWidth/2, frameWidth/2);

        QTime t;
        t = t.addMSecs(mCurrentTimeInMs < 0 ? 0 : mCurrentTimeInMs);
        QFont f = painter->font();
            f.setPointSizeF(f.pointSizeF() * mAntiScale);
        painter->setFont(f);
        painter->setPen(Qt::white);
        painter->drawText(mBalloon, Qt::AlignCenter, t.toString("m:ss"));
    }
}


QPainterPath DelegateVideoControl::shape() const
{
    QPainterPath path;
    QRectF r = rect().adjusted(0,0,0,- rect().height() / 2);
    path.addRoundedRect(r, rect().height() / 4, rect().height() / 4);
    return path;
}


void DelegateVideoControl::updateTicker(qint64 time )
{
    mCurrentTimeInMs = time;
    update();
}


void DelegateVideoControl::totalTimeChanged(qint64 newTotalTime)
{
    mTotalTimeInMs = newTotalTime;
    update();
}


void DelegateVideoControl::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mDisplayCurrentTime = true;
    seekToMousePos(event->pos());
    update();
    event->accept();
}


void DelegateVideoControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (shape().contains(event->pos() - QPointF(mBalloon.width()/2,0)) 
        && shape().contains(event->pos() + QPointF(mBalloon.width()/2,0)))
    {   
        seekToMousePos(event->pos());
        update();
        event->accept();
    }
}


void DelegateVideoControl::seekToMousePos(QPointF mousePos)
{
    qreal minX, length;
    qreal frameWidth = rect().height() / 2;

    minX = rect().x() + frameWidth;
    length = rect().width() - (2 * frameWidth);

    qreal mouseX = mousePos.x();

    if (mTotalTimeInMs > 0 && length > 0 && mDelegate
        && mDelegate->mediaObject() && mDelegate->mediaObject()->isSeekable())
    {
        qint64 tickPos = mTotalTimeInMs / length * (mouseX - minX);
        mDelegate->mediaObject()->seek(tickPos);

        //OSX is a bit lazy
        updateTicker(tickPos);
    }
}

void DelegateVideoControl::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mDisplayCurrentTime = false;
    update();
    event->accept();
}



