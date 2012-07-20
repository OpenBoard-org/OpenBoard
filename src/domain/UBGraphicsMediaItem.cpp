/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsMediaItem.h"
#include "UBGraphicsMediaItemDelegate.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsDelegateFrame.h"
#include "document/UBDocumentProxy.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/memcheck.h"

bool UBGraphicsMediaItem::sIsMutedByDefault = false;

UBGraphicsMediaItem::UBGraphicsMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent)
        : UBGraphicsProxyWidget(parent)
        , mMuted(sIsMutedByDefault)
        , mMutedByUserAction(sIsMutedByDefault)
        , mMediaFileUrl(pMediaFileUrl)
        , mInitialPos(0)
        , mVideoWidget(NULL)
        , mAudioWidget(NULL)
        , mLinkedImage(NULL)
{
    update();

    QString s = pMediaFileUrl.toLocalFile();

    mMediaObject = new Phonon::MediaObject(this);
    if (pMediaFileUrl.toLocalFile().contains("videos")) 
    {
        mMediaType = mediaType_Video;

        mAudioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
        mMediaObject->setTickInterval(50);
        mVideoWidget = new Phonon::VideoWidget(); // owned and destructed by the scene ...
        Phonon::createPath(mMediaObject, mVideoWidget);

        /*
         * The VideoVidget should recover the size from the original movie, but this is not always true expecially on
         * windows and linux os. I don't know why?
         * In this case the wiget size is equal to QSize(1,1).
         */

        if(mVideoWidget->sizeHint() == QSize(1,1)){
            mVideoWidget->resize(320,240);
        }
        setWidget(mVideoWidget);
        haveLinkedImage = true;
    }
    else    
    if (pMediaFileUrl.toLocalFile().contains("audios"))
    {
        mMediaType = mediaType_Audio;
        mAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);

        mMediaObject->setTickInterval(1000);
        mAudioWidget = new QWidget();
        mAudioWidget->resize(320,26);
        setWidget(mAudioWidget);
        haveLinkedImage = false;
    }

    Phonon::createPath(mMediaObject, mAudioOutput);
    
    mSource = Phonon::MediaSource(pMediaFileUrl);
    mMediaObject->setCurrentSource(mSource);

    UBGraphicsMediaItemDelegate* itemDelegate = new UBGraphicsMediaItemDelegate(this, mMediaObject);
    itemDelegate->init();
    setDelegate(itemDelegate);

    mDelegate->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly

    connect(mDelegate, SIGNAL(showOnDisplayChanged(bool)), this, SLOT(showOnDisplayChanged(bool)));
    connect(mMediaObject, SIGNAL(hasVideoChanged(bool)), this, SLOT(hasMediaChanged(bool)));

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
        {
            mMediaObject->pause();
        }
    }
    else if (change == QGraphicsItem::ItemSceneHasChanged)
    {
        if (!scene())
        {
            mMediaObject->stop();
        }
        else
        {
            QString absoluteMediaFilename;

            if(mMediaFileUrl.toLocalFile().startsWith("audios/") || mMediaFileUrl.toLocalFile().startsWith("videos/")){
                absoluteMediaFilename = scene()->document()->persistencePath() + "/"  + mMediaFileUrl.toLocalFile();
            }
            else{
                absoluteMediaFilename = mMediaFileUrl.toLocalFile();
            }

            if (absoluteMediaFilename.length() > 0)
                mMediaObject->setCurrentSource(Phonon::MediaSource(absoluteMediaFilename));

        }
    }

    return UBGraphicsProxyWidget::itemChange(change, value);
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
    mAudioOutput->setMuted(mMuted);
    mMutedByUserAction = mMuted;
    sIsMutedByDefault = mMuted;
}


void UBGraphicsMediaItem::hasMediaChanged(bool hasMedia)
{
    if(hasMedia && mMediaObject->isSeekable())
    {
    Q_UNUSED(hasMedia);
    mMediaObject->seek(mInitialPos);
        UBGraphicsMediaItemDelegate *med = dynamic_cast<UBGraphicsMediaItemDelegate *>(mDelegate);
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
    {
        mMediaObject->pause();
    }
}


void UBGraphicsMediaItem::showOnDisplayChanged(bool shown)
{
    if (!shown)
    {
        mMuted = true;
        mAudioOutput->setMuted(mMuted);
    }
    else if (!mMutedByUserAction)
    {
        mMuted = false;
        mAudioOutput->setMuted(mMuted);
    }
}

UBItem* UBGraphicsMediaItem::deepCopy() const
{
    QUrl url = this->mediaFileUrl();
    UBGraphicsMediaItem *copy;
    
    copy = new UBGraphicsMediaItem(url, parentItem());

    copy->setPos(this->pos());
    copy->setTransform(this->transform());
    copy->setFlag(QGraphicsItem::ItemIsMovable, true);
    copy->setFlag(QGraphicsItem::ItemIsSelectable, true);
    copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    copy->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    copy->setUuid(this->uuid()); // this is OK as long as Videos are imutable
    copy->setSourceUrl(this->sourceUrl());
    copy->resize(this->size());

    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), copy, SLOT(activeSceneChanged()));
    // TODO UB 4.7 complete all members

    return copy;
}

void UBGraphicsMediaItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate)
    {
        mDelegate->mousePressEvent(event);
        if (parentItem() && UBGraphicsGroupContainerItem::Type == parentItem()->type())
        {
            UBGraphicsGroupContainerItem *group = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(parentItem());
            if (group)
            {
                QGraphicsItem *curItem = group->getCurrentItem();
                if (curItem && this != curItem)
                {   
                    group->deselectCurrentItem();    
                }   
                group->setCurrentItem(this);
                this->setSelected(true);
                mDelegate->positionHandles();
            }       

        }
    }

    if (parentItem() && parentItem()->type() == UBGraphicsGroupContainerItem::Type)
    {
        mShouldMove = false;
        if (!Delegate()->mousePressEvent(event))
        {
            event->accept();
        }
    }
    else 
    {
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
            translate(translation.x(), translation.y());
        }

        mMouseMovePos = event->scenePos();
    }

    event->accept();

}
