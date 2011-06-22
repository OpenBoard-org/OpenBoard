#include "UBGraphicsAudioItem.h"
#include "UBGraphicsAudioItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"

#include "core/memcheck.h"

UBGraphicsAudioItem::UBGraphicsAudioItem(const QUrl& pAudioFileUrl, QGraphicsItem *parent):
        UBGraphicsMediaItem(pAudioFileUrl,parent)
{
    update();

    mAudioOutput = new Phonon::AudioOutput ( Phonon::MusicCategory, this );
    mMediaObject = new Phonon::MediaObject ( this );
    mMediaObject->setTickInterval ( 1000 );
    Phonon::createPath ( mMediaObject, mAudioOutput );

    mMediaObject->clearQueue();
    mSource = Phonon::MediaSource(pAudioFileUrl);
    mMediaObject->setCurrentSource (mSource );

    connect (mMediaObject,SIGNAL ( tick ( qint64 ) ), this, SLOT ( tick ( qint64 ) ) );
    connect(mMediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(onStateChanged(Phonon::State,Phonon::State)));

    mAudioWidget = new QWidget();

    mSeekSlider = new Phonon::SeekSlider ( mAudioWidget );
    mSeekSlider->setMediaObject ( mMediaObject );

    QPalette palette;
    palette.setBrush ( QPalette::Light, Qt::darkGray );

    mTimeLcd = new QLCDNumber;
    mTimeLcd->setPalette ( palette );
    mTimeLcd->display ( "00:00" );

    QHBoxLayout *seekerLayout = new QHBoxLayout;
    seekerLayout->addWidget ( mSeekSlider );
    seekerLayout->addWidget ( mTimeLcd );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout ( seekerLayout );

    mAudioWidget->setLayout ( mainLayout );
    setWidget ( mAudioWidget );

    UBGraphicsAudioItemDelegate* delegate = new UBGraphicsAudioItemDelegate ( this, mMediaObject );
    delegate->init();
    setDelegate ( delegate );

    mDelegate->frame()->setOperationMode ( UBGraphicsDelegateFrame::Resizing );

}

void UBGraphicsAudioItem::onStateChanged(Phonon::State newState, Phonon::State oldState)
{
    qDebug() << "STATE CHANGED!";
    qDebug() << "old state:" << oldState;
    qDebug() << "new state:" << newState;

    if(oldState == Phonon::ErrorState)
    {
        qDebug() << "ERROR! : " << mMediaObject->errorString();
    }
    else if(newState == Phonon::LoadingState)
    {
        int itotaltime = mMediaObject->totalTime();
        qDebug() << "[Loading State entered!] Total time : " << itotaltime;
    }
}

UBGraphicsAudioItem::~UBGraphicsAudioItem()
{
    //NOOP
}

UBItem* UBGraphicsAudioItem::deepCopy() const
{
    QUrl audioUrl = this->mediaFileUrl();

    UBGraphicsAudioItem *copy = new UBGraphicsAudioItem(audioUrl, parentItem());

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

void UBGraphicsAudioItem::tick ( qint64 time )
{
    QTime displayTime ( 0, ( time / 60000 ) % 60, ( time / 1000 ) % 60 );

    mTimeLcd->display ( displayTime.toString ( "mm:ss" ) );
}
