#include "UBGraphicsMediaItem.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsDelegateFrame.h"

#include "document/UBDocumentProxy.h"

#include "core/UBApplication.h"

#include "board/UBBoardController.h"

bool UBGraphicsMediaItem::sIsMutedByDefault = false;

UBGraphicsMediaItem::UBGraphicsMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent)
        : UBGraphicsProxyWidget(parent)
        , mMuted(sIsMutedByDefault)
        , mMutedByUserAction(sIsMutedByDefault)
        , mMediaFileUrl(pMediaFileUrl)
        , mInitialPos(0)
{
    //NOOP
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

            if (mMediaFileUrl.isRelative() && scene()->document())
            {
                absoluteMediaFilename =
                    /*scene()->document()->persistencePath()
                    + "/"  + */mMediaFileUrl.toLocalFile();
            }
            else if (!mMediaFileUrl.isRelative())
            {
                absoluteMediaFilename = mMediaFileUrl.toLocalFile();
            }


            if (absoluteMediaFilename.length() > 0)
                mMediaObject->setCurrentSource(Phonon::MediaSource(absoluteMediaFilename));

        }
    }

    return UBGraphicsProxyWidget::itemChange(change, value);
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
    Q_UNUSED(hasMedia);
    mMediaObject->seek(mInitialPos);
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
