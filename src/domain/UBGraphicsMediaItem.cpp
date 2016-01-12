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

UBGraphicsMediaItem::UBGraphicsMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent)
        : UBGraphicsProxyWidget(parent)
        , mDummyVideoWidget(NULL)
        , mVideoItem(NULL)
        , mAudioWidget(NULL)
        , mMuted(sIsMutedByDefault)
        , mMutedByUserAction(sIsMutedByDefault)
        , mMediaFileUrl(pMediaFileUrl)
        , mLinkedImage(NULL)
        , mInitialPos(0)
{
    update();


    mMediaObject = new QMediaPlayer(this);

    QString mediaPath = pMediaFileUrl.toString();
    if ("" == mediaPath)
        mediaPath = pMediaFileUrl.toLocalFile();

    if (mediaPath.toLower().contains("videos"))
    {
        mMediaType = mediaType_Video;

        mMediaObject->setNotifyInterval(50);

        mDummyVideoWidget = new QWidget(); // owned and destructed by the scene ...
        mDummyVideoWidget->resize(320,240);
        mDummyVideoWidget->setMinimumSize(320, 240);
        mDummyVideoWidget->setWindowOpacity(0.0);
        
        mVideoItem = new QGraphicsVideoItem();

        // Necessary to allow the video to be displayed on secondary screen
        mVideoItem->setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

        mMediaObject->setVideoOutput(mVideoItem);

        mVideoItem->setSize(QSize(320,240));

        haveLinkedImage = true;
    }
    else if (mediaPath.toLower().contains("audios"))
    {
        mMediaType = mediaType_Audio;

        mMediaObject->setNotifyInterval(1000);
        mAudioWidget = new QWidget();
        mAudioWidget->resize(320,26);
        mAudioWidget->setMinimumSize(150,26);

        haveLinkedImage = false;
    }

     mMediaObject->setMedia(pMediaFileUrl);


    // we should create delegate after media objects because delegate uses his properties at creation.
    setDelegate(new UBGraphicsMediaItemDelegate(this, mMediaObject));


    // delegate should be created earler because we setWidget calls resize event for graphics proxy widgt.
    // resize uses delegate.
    if (mediaType_Video == mMediaType)
        setWidget(mDummyVideoWidget);
    else
        setWidget(mAudioWidget);

    // media widget should be created and placed on proxy widget here.
    // TODO claudio remove this because in contrast with the fact the frame should be created on demand.
    // but without forcing the control creation we do not have the frame and all the calculation
    // for the different element of the interface will fail
    Delegate()->createControls();
    if (mediaType_Audio == mMediaType)
        Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::ResizingHorizontally);
    else
        Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly

    connect(Delegate(), SIGNAL(showOnDisplayChanged(bool)), this, SLOT(showOnDisplayChanged(bool)));
    connect(mMediaObject, SIGNAL(videoAvailableChanged(bool)), this, SLOT(hasMediaChanged(bool)));

    // Resize the widget as soon as the video's native size is known (it isn't at this stage)
    connect(mVideoItem, SIGNAL(nativeSizeChanged(QSizeF)), this, SLOT(resize(QSizeF)));
}


UBGraphicsMediaItem::~UBGraphicsMediaItem()
{
    if (mMediaObject)
        mMediaObject->stop();
}


void UBGraphicsMediaItem::setSelected(bool selected)
{
    if(selected){
        Delegate()->createControls();
        if (mediaType_Audio == mMediaType)
            Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::ResizingHorizontally);
        else
            Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);
    }
    UBGraphicsProxyWidget::setSelected(selected);
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

    // Pass on geometry and position changes to the videoItem
    else if (mVideoItem && change == QGraphicsItem::ItemTransformChange)
        mVideoItem->setTransform(qvariant_cast<QTransform>(value));

    else if (mVideoItem && change == QGraphicsItem::ItemPositionChange)
        mVideoItem->setPos(qvariant_cast<QPointF>(value));

    return UBGraphicsProxyWidget::itemChange(change, value);
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
    if (!shown)
    {
        mMuted = true;
        mMediaObject->setMuted(mMuted);
    }
    else if (!mMutedByUserAction)
    {
        mMuted = false;
        mMediaObject->setMuted(mMuted);
    }
}

UBItem* UBGraphicsMediaItem::deepCopy() const
{
    QUrl url = this->mediaFileUrl();
    UBGraphicsMediaItem *copy = new UBGraphicsMediaItem(url, parentItem());

    copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable

    copyItemParameters(copy);

    return copy;
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
        cp->resize(this->size());

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

}

void UBGraphicsMediaItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(mShouldMove && (event->buttons() & Qt::LeftButton))
    {
        QPointF offset = event->scenePos() - mMousePressPos;

        if (offset.toPoint().manhattanLength() > QApplication::startDragDistance())
        {
            QPointF mouseMovePos = mapFromScene(mMouseMovePos);
            QPointF eventPos = mapFromScene( event->scenePos());

            QPointF translation = eventPos - mouseMovePos;
            setPos(translation.x(), translation.y());
        }

        mMouseMovePos = event->scenePos();
    }

    event->accept();

}

void UBGraphicsMediaItem::setPos(const QPointF &pos)
{
   QGraphicsItem::setPos(pos);
   if (mVideoItem)
       mVideoItem->setPos(pos);
}

void UBGraphicsMediaItem::setPos(qreal x, qreal y)
{
    setPos(QPointF(x, y));
}

void UBGraphicsMediaItem::setMatrix(const QMatrix &matrix, bool combine)
{
    QGraphicsItem::setMatrix(matrix, combine);

    if (mVideoItem)
        mVideoItem->setMatrix(matrix, combine);
}

void UBGraphicsMediaItem::setTransform(const QTransform &matrix, bool combine)
{
    QGraphicsItem::setTransform(matrix, combine);

    if (mVideoItem)
        mVideoItem->setTransform(matrix, combine);
}

void UBGraphicsMediaItem::resize(const QSizeF & pSize)
{
    // Resize the video, then the rest of the Item

    if (mVideoItem) {
        qreal sizeX = 0;
        qreal sizeY = 0;

        QSizeF minimumItemSize(mDummyVideoWidget->minimumSize());
        if (minimumItemSize.width() > pSize.width())
            sizeX = minimumItemSize.width();
        else
            sizeX = pSize.width();

        if (minimumItemSize.height() > pSize.height())
            sizeY = minimumItemSize.height();
        else
            sizeY = pSize.height();

        mVideoItem->setSize(QSizeF (sizeX, sizeY));
    }

    UBGraphicsProxyWidget::resize(pSize);
}

void UBGraphicsMediaItem::resize(qreal w, qreal h)
{
    UBGraphicsMediaItem::resize(QSizeF(w, h));
}


