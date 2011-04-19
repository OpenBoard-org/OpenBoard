#include "UBGraphicsVideoItem.h"
#include "UBGraphicsVideoItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"


UBGraphicsVideoItem::UBGraphicsVideoItem(const QUrl& pVideoFileUrl, QGraphicsItem *parent):
        UBGraphicsMediaItem(pVideoFileUrl,parent)
        , mShouldMove(false)
{
    update();

    mMediaObject = new Phonon::MediaObject(this);
    mVideoWidget = new Phonon::VideoWidget(); // owned and destructed by the scene ...

    Phonon::createPath(mMediaObject, mVideoWidget);

    mAudioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    Phonon::createPath(mMediaObject, mAudioOutput);

    /*
     * The VideoVidget should recover the size from the original movie, but this is not always true expecially on
     * windows and linux os. I don't know why?
     * In this case the wiget size is equal to QSize(1,1).
     */

    if(mVideoWidget->sizeHint() == QSize(1,1)){
      mVideoWidget->resize(320,240);
    }

    setWidget(mVideoWidget);

    UBGraphicsVideoItemDelegate* delegate = new UBGraphicsVideoItemDelegate(this, mMediaObject);
    delegate->init();
    setDelegate(delegate);

    mDelegate->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

    connect(mDelegate, SIGNAL(showOnDisplayChanged(bool)), this, SLOT(showOnDisplayChanged(bool)));
    connect(mMediaObject, SIGNAL(hasVideoChanged(bool)), this, SLOT(hasVideoChanged(bool)));
}


UBGraphicsVideoItem::~UBGraphicsVideoItem()
{
    //NOOP
}

UBItem* UBGraphicsVideoItem::deepCopy() const
{
    QUrl videoUrl = this->mediaFileUrl();

    UBGraphicsVideoItem *copy = new UBGraphicsVideoItem(videoUrl, parentItem());

    copy->setPos(this->pos());
    copy->setZValue(this->zValue());
    copy->setTransform(this->transform());
    copy->setFlag(QGraphicsItem::ItemIsMovable, true);
    copy->setFlag(QGraphicsItem::ItemIsSelectable, true);
    copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    copy->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    copy->setUuid(this->uuid()); // this is OK as long as Videos are imutable
    copy->setSourceUrl(this->sourceUrl());

    copy->resize(this->size());

    // TODO UB 4.7 complete all members

    return copy;
}



void UBGraphicsVideoItem::hasVideoChanged(bool hasVideo)
{
    if(hasVideo && mMediaObject->isSeekable())
    {
        hasMediaChanged(hasVideo);
        UBGraphicsVideoItemDelegate *vid = dynamic_cast<UBGraphicsVideoItemDelegate *>(mDelegate);
        if (vid)
            vid->updateTicker(initialPos());
    }
}

void UBGraphicsVideoItem::showOnDisplayChanged(bool shown)
{
    UBGraphicsMediaItem::showOnDisplayChanged(shown);
    UBGraphicsVideoItemDelegate *vid = dynamic_cast<UBGraphicsVideoItemDelegate*>(mDelegate);

    if (vid)
        vid->toggleMute();
}

void UBGraphicsVideoItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QDrag* mDrag = new QDrag(event->widget());
    QMimeData* pMime = new QMimeData();
    mDrag->setMimeData(pMime);
    mDrag->start();

    mShouldMove = (event->buttons() & Qt::LeftButton);
    mMousePressPos = event->scenePos();
    mMouseMovePos = mMousePressPos;

    event->accept();
    setSelected(true);

}

void UBGraphicsVideoItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
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
