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




#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsMediaItem.h"
#include "UBGraphicsMediaItemDelegate.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsDelegateFrame.h"
#include "document/UBDocumentProxy.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "core/memcheck.h"

#include <QGraphicsVideoItem>

bool UBGraphicsMediaItem::sIsMutedByDefault = false;

/**
 * @brief Create and return a UBGraphicsMediaItem instance. The type (audio or video) is determined from the URL.
 * @param pMediaFileUrl The URL of the audio or video file
 * @param parent (Optional) the parent item
 * @return A pointer to the newly created instance.
 */
UBGraphicsMediaItem* UBGraphicsMediaItem::createMediaItem(const QUrl &pMediaFileUrl, QGraphicsItem* parent)
{
    UBGraphicsMediaItem * mediaItem;

    QString mediaPath = pMediaFileUrl.toString();
    if ("" == mediaPath)
        mediaPath = pMediaFileUrl.toLocalFile();

    if (mediaPath.toLower().contains("videos"))
        mediaItem = new UBGraphicsVideoItem(pMediaFileUrl, parent);
    else if (mediaPath.toLower().contains("audios"))
        mediaItem = new UBGraphicsAudioItem(pMediaFileUrl, parent);

    return mediaItem;
}

UBGraphicsMediaItem::UBGraphicsMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent)
        : QGraphicsRectItem(parent)
        , mMuted(sIsMutedByDefault)
        , mMutedByUserAction(sIsMutedByDefault)
        , mMediaFileUrl(pMediaFileUrl)
        , mLinkedImage(NULL)
        , mInitialPos(0)
{

    mMediaObject = new QMediaPlayer(this);
    mMediaObject->setMedia(pMediaFileUrl);

    setDelegate(new UBGraphicsMediaItemDelegate(this));
    Delegate()->createControls();

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem));
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);

    setAcceptHoverEvents(true);

    connect(mMediaObject, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            Delegate(), SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));

    connect(mMediaObject, SIGNAL(stateChanged(QMediaPlayer::State)),
            Delegate(), SLOT(mediaStateChanged(QMediaPlayer::State)));

    /*
    connect(mMediaObject, static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
            Delegate(), &UBGraphicsMediaItemDelegate::mediaError);
            */

    connect(mMediaObject, SIGNAL(positionChanged(qint64)),
            Delegate(), SLOT(updateTicker(qint64)));

    connect(mMediaObject, SIGNAL(durationChanged(qint64)),
            Delegate(), SLOT(totalTimeChanged(qint64)));

    connect(Delegate(), SIGNAL(showOnDisplayChanged(bool)),
            this, SLOT(showOnDisplayChanged(bool)));

    connect(mMediaObject, SIGNAL(videoAvailableChanged(bool)),
            this, SLOT(hasMediaChanged(bool)));
}

UBGraphicsAudioItem::UBGraphicsAudioItem(const QUrl &pMediaFileUrl, QGraphicsItem *parent)
    :UBGraphicsMediaItem(pMediaFileUrl, parent)
{
    this->setSize(320, 26);
    this->setMinimumSize(QSize(150, 26));

    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::ResizingHorizontally);

    mMediaObject->setNotifyInterval(1000);
    haveLinkedImage = false;

}

UBGraphicsVideoItem::UBGraphicsVideoItem(const QUrl &pMediaFileUrl, QGraphicsItem *parent)
    :UBGraphicsMediaItem(pMediaFileUrl, parent)
{
    mVideoItem = new QGraphicsVideoItem(this);

    mVideoItem->setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    mVideoItem->setFlag(ItemStacksBehindParent, true);

    mMediaObject->setVideoOutput(mVideoItem);
    mMediaObject->setNotifyInterval(50);

    setMinimumSize(QSize(320, 240));
    setSize(320, 240);


    connect(mVideoItem, SIGNAL(nativeSizeChanged(QSizeF)),
            this, SLOT(videoSizeChanged(QSizeF)));

    haveLinkedImage = true;
    update();
}

UBGraphicsMediaItem::~UBGraphicsMediaItem()
{
    if (mMediaObject)
        mMediaObject->stop();
}

QVariant UBGraphicsMediaItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == QGraphicsItem::ItemEnabledChange)
            || (change == QGraphicsItem::ItemSceneChange)
            || (change == QGraphicsItem::ItemVisibleChange))
    {
        if (mMediaObject && (!isEnabled() || !isVisible() || !scene()))
            mMediaObject->pause();
    }
    else if (change == QGraphicsItem::ItemSceneHasChanged)
    {
        if (!scene())
            mMediaObject->stop();
        else {
            QString absoluteMediaFilename;

            if(mMediaFileUrl.toLocalFile().startsWith("audios/") || mMediaFileUrl.toLocalFile().startsWith("videos/"))
                absoluteMediaFilename = scene()->document()->persistencePath() + "/"  + mMediaFileUrl.toLocalFile();
            else
                absoluteMediaFilename = mMediaFileUrl.toLocalFile();

            if (absoluteMediaFilename.length() > 0)
                  mMediaObject->setMedia(QUrl::fromLocalFile(absoluteMediaFilename));

        }
    }

    if (Delegate()) {
        QVariant newValue = Delegate()->itemChange(change, value);
        return QGraphicsRectItem::itemChange(change, newValue);
    }

    else
        return QGraphicsRectItem::itemChange(change, value);
}

void UBGraphicsMediaItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    Delegate()->postpaint(painter, option, widget);
    painter->restore();
}


QMediaPlayer::State UBGraphicsMediaItem::playerState() const
{
    return mMediaObject->state();
}

qint64 UBGraphicsMediaItem::mediaDuration() const
{
    return mMediaObject->duration();
}

qint64 UBGraphicsMediaItem::mediaPosition() const
{
    return mMediaObject->position();
}

bool UBGraphicsMediaItem::isMediaSeekable() const
{
    return mMediaObject->isSeekable();
}

/**
 * @brief Set the item's minimum size. If the current size is smaller, it will be resized.
 * @param size The new minimum size
 */
void UBGraphicsMediaItem::setMinimumSize(const QSize& size)
{
    mMinimumSize = size;

    QSizeF newSize = rect().size();
    int width = newSize.width();
    int height = newSize.height();

    if (rect().width() < mMinimumSize.width())
        width = mMinimumSize.width();

    if (rect().height() < mMinimumSize.height())
        height = mMinimumSize.height();

    this->setSize(width, height);
}

void UBGraphicsMediaItem::setMediaFileUrl(QUrl url)
{
    mMediaFileUrl = url;
}

void UBGraphicsMediaItem::setInitialPos(qint64 p)
{
    mInitialPos = p;
}

void UBGraphicsMediaItem::setMediaPos(qint64 p)
{
    mMediaObject->setPosition(p);
}

void UBGraphicsMediaItem::setSelected(bool selected)
{
    if(selected){
        Delegate()->createControls();
        if (this->getMediaType() == mediaType_Audio)
            Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::ResizingHorizontally);
        else
            Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);
    }
    QGraphicsRectItem::setSelected(selected);
}

void UBGraphicsMediaItem::setSourceUrl(const QUrl &pSourceUrl)
{
    UBItem::setSourceUrl(pSourceUrl);
}

void UBGraphicsMediaItem::clearSource()
{
    QString path = mediaFileUrl().toLocalFile();
    //if path is absolute clean duplicated path string
    if (!path.contains(UBApplication::boardController->selectedDocument()->persistencePath()))
        path = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + path;

    if (!UBFileSystemUtils::deleteFile(path))
        qDebug() << "cannot delete file: " << path;
}

void UBGraphicsMediaItem::toggleMute()
{
    mMuted = !mMuted;
    setMute(mMuted);
}

void UBGraphicsMediaItem::setMute(bool bMute)
{
    mMuted = bMute;
    mMediaObject->setMuted(mMuted);
    mMutedByUserAction = mMuted;
    sIsMutedByDefault = mMuted;
}


void UBGraphicsMediaItem::hasMediaChanged(bool hasMedia)
{
    if(hasMedia && mMediaObject->isSeekable())
    {
        mMediaObject->setPosition(mInitialPos);

        UBGraphicsMediaItemDelegate *med = dynamic_cast<UBGraphicsMediaItemDelegate *>(Delegate());
        if (med)
            med->updateTicker(initialPos());
    }
}


UBGraphicsScene* UBGraphicsMediaItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsMediaItem::activeSceneChanged()
{
    if (UBApplication::boardController->activeScene() != scene())
        mMediaObject->pause();
}


void UBGraphicsMediaItem::showOnDisplayChanged(bool shown)
{
    if (!shown) {
        mMuted = true;
        mMediaObject->setMuted(mMuted);
    }
    else if (!mMutedByUserAction) {
        mMuted = false;
        mMediaObject->setMuted(mMuted);
    }
}
void UBGraphicsMediaItem::play()
{
    mMediaObject->play();
}

void UBGraphicsMediaItem::pause()
{
    mMediaObject->pause();
}

void UBGraphicsMediaItem::stop()
{
    mMediaObject->stop();
}

void UBGraphicsMediaItem::togglePlayPause()
{

    if (mMediaObject->state() == QMediaPlayer::StoppedState)
        mMediaObject->play();

    else if (mMediaObject->state() == QMediaPlayer::PlayingState) {

        if ((mMediaObject->duration() - mMediaObject->position()) <= 0) {
            mMediaObject->stop();
            mMediaObject->play();
        }

        else {
            mMediaObject->pause();
            if(scene())
                scene()->setModified(true);
        }
    }

    else if (mMediaObject->state() == QMediaPlayer::PausedState) {
        if ((mMediaObject->duration() - mMediaObject->position()) <= 0)
            mMediaObject->stop();

        mMediaObject->play();
    }

    else  if ( mMediaObject->mediaStatus() == QMediaPlayer::LoadingMedia) {
        mMediaObject->setMedia(mediaFileUrl());
        mMediaObject->play();
    }

    if (mMediaObject->error())
        qDebug() << "Error appeared." << mMediaObject->errorString();

}

void UBGraphicsMediaItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsMediaItem *cp = dynamic_cast<UBGraphicsMediaItem*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setSourceUrl(this->sourceUrl());
        cp->setSize(rect().width(), rect().height());

        cp->setZValue(this->zValue());

        connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), cp, SLOT(activeSceneChanged()));
        // TODO UB 4.7 complete all members
    }
}

void UBGraphicsMediaItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate())
    {
        Delegate()->mousePressEvent(event);
        if (parentItem() && UBGraphicsGroupContainerItem::Type == parentItem()->type())
        {
            UBGraphicsGroupContainerItem *group = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(parentItem());
            if (group)
            {
                QGraphicsItem *curItem = group->getCurrentItem();
                if (curItem && this != curItem)
                    group->deselectCurrentItem();
                group->setCurrentItem(this);
                this->setSelected(true);
                Delegate()->positionHandles();
            }

        }
    }

    if (parentItem() && parentItem()->type() == UBGraphicsGroupContainerItem::Type) {
        mShouldMove = false;
        if (!Delegate()->mousePressEvent(event))
            event->accept();
    }
    else {
        mShouldMove = (event->buttons() & Qt::LeftButton);
        mMousePressPos = event->scenePos();
        mMouseMovePos = mMousePressPos;

        event->accept();
        setSelected(true);
    }
    QGraphicsRectItem::mousePressEvent(event);
}

QRectF UBGraphicsMediaItem::boundingRect() const
{
    return rect();
}

void UBGraphicsMediaItem::setSize(int width, int height)
{
    QRectF r = rect();
    r.setWidth(width);
    r.setHeight(height);
    setRect(r);

    if (Delegate())
        Delegate()->positionHandles();
    if (scene())
        scene()->setModified(true);
}

UBItem* UBGraphicsAudioItem::deepCopy() const
{
    QUrl url = this->mediaFileUrl();
    UBGraphicsMediaItem *copy = new UBGraphicsAudioItem(url, parentItem());

    copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable

    copyItemParameters(copy);

    return copy;
}

UBItem* UBGraphicsVideoItem::deepCopy() const
{
    QUrl url = this->mediaFileUrl();
    UBGraphicsMediaItem *copy = new UBGraphicsVideoItem(url, parentItem());

    copy->setUuid(this->uuid());
    copyItemParameters(copy);

    return copy;
}

void UBGraphicsVideoItem::setSize(int width, int height)
{
    // Resize the video, then the rest of the Item

    int sizeX = 0;
    int sizeY = 0;

    if (mMinimumSize.width() > width)
        sizeX = mMinimumSize.width();
    else
        sizeX = width;

    if (mMinimumSize.height() > height)
        sizeY = mMinimumSize.height();
    else
        sizeY = height;

    mVideoItem->setSize(QSize(sizeX, sizeY));


    UBGraphicsMediaItem::setSize(sizeX, sizeY);
}

void UBGraphicsVideoItem::videoSizeChanged(QSizeF newSize)
{
    this->setSize(newSize.width(), newSize.height());
}


