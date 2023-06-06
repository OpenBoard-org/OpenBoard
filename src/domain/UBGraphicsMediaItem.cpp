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
        , mStopped(false)
        , mFirstLoad(true)
        , mMediaFileUrl(pMediaFileUrl)
        , mLinkedImage(NULL)
        , mInitialPos(0)
{

    mErrorString = "";

    mMediaObject = new QMediaPlayer(this);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    mMediaObject->setSource(pMediaFileUrl);
    QAudioOutput* output = new QAudioOutput(QAudioDevice(), this);
    mMediaObject->setAudioOutput(output);
#else
    mMediaObject->setMedia(pMediaFileUrl);
#endif

    setDelegate(new UBGraphicsMediaItemDelegate(this));

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem));
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);

    connect(mMediaObject, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            Delegate(), SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    connect(mMediaObject, SIGNAL(playbackStateChanged(QMediaPlayer::PlaybackState)),
            Delegate(), SLOT(mediaStateChanged(QMediaPlayer::PlaybackState)));
#else
    connect(mMediaObject, SIGNAL(stateChanged(QMediaPlayer::State)),
            Delegate(), SLOT(mediaStateChanged(QMediaPlayer::State)));
#endif

    connect(mMediaObject, SIGNAL(positionChanged(qint64)),
            Delegate(), SLOT(updateTicker(qint64)));

    connect(mMediaObject, SIGNAL(durationChanged(qint64)),
            Delegate(), SLOT(totalTimeChanged(qint64)));

    connect(Delegate(), SIGNAL(showOnDisplayChanged(bool)),
            this, SLOT(showOnDisplayChanged(bool)));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    connect(mMediaObject, &QMediaPlayer::errorOccurred,
            this, &UBGraphicsMediaItem::mediaError);
#else
    connect(mMediaObject, qOverload<QMediaPlayer::Error>(&QMediaPlayer::error),
            this, &UBGraphicsMediaItem::mediaError);
#endif
}

UBGraphicsAudioItem::UBGraphicsAudioItem(const QUrl &pMediaFileUrl, QGraphicsItem *parent)
    :UBGraphicsMediaItem(pMediaFileUrl, parent)
{
    haveLinkedImage = false;

    Delegate()->createControls();
    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::ResizingHorizontally);

    this->setSize(320, 26);
    this->setMinimumSize(QSize(150, 26));

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    mMediaObject->setNotifyInterval(1000);
#endif
}

UBGraphicsVideoItem::UBGraphicsVideoItem(const QUrl &pMediaFileUrl, QGraphicsItem *parent)
    :UBGraphicsMediaItem(pMediaFileUrl, parent)
{
    haveLinkedImage = true;
    setPlaceholderVisible(true);
    Delegate()->createControls();

    mVideoItem = new QGraphicsVideoItem(this);

    mVideoItem->setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    mVideoItem->setFlag(ItemStacksBehindParent, true);

    /* setVideoOutput has to be called only when the video item is visible on the screen,
     * due to a Qt bug (QTBUG-32522). So instead of calling it here, it is called when the
     * active scene has changed, or when the item is first created.
     * If and when Qt fix this issue, this should be changed back.
     * */
    mMediaObject->setVideoOutput(mVideoItem);
    mHasVideoOutput = true;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    mMediaObject->setNotifyInterval(50);
#endif

    setMinimumSize(QSize(320, 240));
    setSize(320, 240);

    connect(mVideoItem, SIGNAL(nativeSizeChanged(QSizeF)),
            this, SLOT(videoSizeChanged(QSizeF)));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    connect(mMediaObject, &QMediaPlayer::hasVideoChanged,
            this, &UBGraphicsVideoItem::hasVideoChanged);

    connect(mMediaObject, &QMediaPlayer::playbackStateChanged,
            this, &UBGraphicsVideoItem::mediaStateChanged);

    connect(mMediaObject, &QMediaPlayer::errorOccurred,
            this, &UBGraphicsVideoItem::mediaError);
#else
    connect(mMediaObject, SIGNAL(videoAvailableChanged(bool)),
            this, SLOT(hasVideoChanged(bool)));

    connect(mMediaObject, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(mediaStateChanged(QMediaPlayer::State)));

    connect(mMediaObject, qOverload<QMediaPlayer::Error>(&QMediaPlayer::error),
            this, &UBGraphicsVideoItem::mediaError);
#endif

    setAcceptHoverEvents(true);

    update();
}

UBGraphicsMediaItem::~UBGraphicsMediaItem()
{
    if (mMediaObject) {
        mMediaObject->stop();
        delete mMediaObject;
    }
}

QVariant UBGraphicsMediaItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == QGraphicsItem::ItemEnabledChange)
            || (change == QGraphicsItem::ItemSceneChange)
            || (change == QGraphicsItem::ItemVisibleChange))
    {
        if (mMediaObject && (!isEnabled() || !isVisible() || !scene()))
           pause();
    }
    else if (change == QGraphicsItem::ItemSceneHasChanged)
    {
        if (!scene())
            stop();
        else {
            QString absoluteMediaFilename;

            if(mMediaFileUrl.toLocalFile().startsWith("audios/") || mMediaFileUrl.toLocalFile().startsWith("videos/"))
                absoluteMediaFilename = scene()->document()->persistencePath() + "/"  + mMediaFileUrl.toLocalFile();
            else
                absoluteMediaFilename = mMediaFileUrl.toLocalFile();

            if (absoluteMediaFilename.length() > 0)
            {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                mMediaObject->setSource(QUrl::fromLocalFile(absoluteMediaFilename));
#else
                mMediaObject->setMedia(QUrl::fromLocalFile(absoluteMediaFilename));
#endif
            }
        }
    }

    if (Delegate()) {
        QVariant newValue = Delegate()->itemChange(change, value);
        return QGraphicsRectItem::itemChange(change, newValue);
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void UBGraphicsMediaItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    //painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    Delegate()->postpaint(painter, option, widget);
    painter->restore();
}


#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
QMediaPlayer::PlaybackState UBGraphicsMediaItem::playerState() const
{
    return mMediaObject->playbackState();
}
#else
QMediaPlayer::State UBGraphicsMediaItem::playerState() const
{
    return mMediaObject->state();
}
#endif

/**
 * @brief Returns true if the video was manually stopped, false otherwise.
 */
bool UBGraphicsMediaItem::isStopped() const
{
    return mStopped;
}

bool UBGraphicsMediaItem::firstLoad() const
{
    return mFirstLoad;
}

void UBGraphicsMediaItem::setFirstLoad(bool firstLoad)
{
    mFirstLoad = firstLoad;
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

void UBGraphicsMediaItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid));
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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    mMediaObject->audioOutput()->setMuted(mMuted);
#else
    mMediaObject->setMuted(mMuted);
#endif
    mMutedByUserAction = mMuted;
    sIsMutedByDefault = mMuted;
}

std::shared_ptr<UBGraphicsScene> UBGraphicsMediaItem::scene()
{
    auto scenePtr = dynamic_cast<UBGraphicsScene*>(QGraphicsItem::scene());
    return scenePtr ? scenePtr->shared_from_this() : nullptr;
}


void UBGraphicsMediaItem::activeSceneChanged()
{
    if (UBApplication::boardController->activeScene() != scene())
        pause();
}


void UBGraphicsMediaItem::showOnDisplayChanged(bool shown)
{
    if (!shown) {
        mMuted = true;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        mMediaObject->audioOutput()->setMuted(mMuted);
#else
        mMediaObject->setMuted(mMuted);
#endif
    }
    else if (!mMutedByUserAction) {
        mMuted = false;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        mMediaObject->audioOutput()->setMuted(mMuted);
#else
        mMediaObject->setMuted(mMuted);
#endif
    }
}
void UBGraphicsMediaItem::play()
{
    mMediaObject->play();
    mStopped = false;
}

void UBGraphicsMediaItem::pause()
{
    mMediaObject->pause();
    mStopped = false;
}

void UBGraphicsMediaItem::stop()
{
    mMediaObject->stop();
    mStopped = true;
}

void UBGraphicsMediaItem::togglePlayPause()
{
    if (!mErrorString.isEmpty()) {
        UBApplication::showMessage("Can't play media: " + mErrorString);
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QMediaPlayer::PlaybackState state = mMediaObject->playbackState();
#else
    QMediaPlayer::State state = mMediaObject->state();
#endif

    if (state == QMediaPlayer::StoppedState)
    {
        play();
    }

    else if (state == QMediaPlayer::PlayingState) {

        if ((mMediaObject->duration() - mMediaObject->position()) <= 0) {
            stop();
            play();
        }

        else {
            pause();
            if(scene())
                scene()->setModified(true);
        }
    }

    else if (state == QMediaPlayer::PausedState) {
        if ((mMediaObject->duration() - mMediaObject->position()) <= 0)
            stop();

        play();
    }

    else  if ( mMediaObject->mediaStatus() == QMediaPlayer::LoadingMedia) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        mMediaObject->setSource(mediaFileUrl());
#else
        mMediaObject->setMedia(mediaFileUrl());
#endif
        play();
    }
}

void UBGraphicsMediaItem::mediaError(QMediaPlayer::Error errorCode)
{
    // QMediaPlayer::errorString() isn't very descriptive, so we generate our own message

    switch (errorCode) {
        case QMediaPlayer::NoError:
            mErrorString = "";
            break;
        case QMediaPlayer::ResourceError:
            mErrorString = tr("Media resource couldn't be resolved");
            break;
        case QMediaPlayer::FormatError:
            mErrorString = tr("Unsupported media format");
            break;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        case QMediaPlayer::ServiceMissingError:
            mErrorString = tr("Media playback service not found");
            break;
        default:
            mErrorString = tr("Media error: ") + QString(errorCode) + " (" + mMediaObject->errorString() + ")";
#else
        default:
            mErrorString = tr("Media error: ") + QMetaEnum::fromType<QMediaPlayer::Error>().valueToKey(errorCode) + " (" + mMediaObject->errorString() + ")";
#endif
    }

    if (!mErrorString.isEmpty() ) {
        UBApplication::showMessage(mErrorString);
        qDebug() << mErrorString;
        mErrorString.clear();
    }
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
        cp->setData(UBGraphicsItemData::ItemIsHiddenOnDisplay, this->data(UBGraphicsItemData::ItemIsHiddenOnDisplay));
        cp->setSourceUrl(this->sourceUrl());
        cp->setSize(rect().width(), rect().height());

        cp->setZValue(this->zValue());

        connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), cp, SLOT(activeSceneChanged()));
        // TODO UB 4.7 complete all members
    }
}

void UBGraphicsMediaItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()) {
        Delegate()->mousePressEvent(event);
        if (parentItem() && UBGraphicsGroupContainerItem::Type == parentItem()->type()) {
            UBGraphicsGroupContainerItem *group = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(parentItem());
            if (group) {
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
    /* Depending on the platform/video backend, video size information becomes
     * available at different times (either when the file is loaded, or when
     * playback begins), so this slot is needed to resize the video item as
     * soon as the information is available.
     */


    // We don't want the video item to resize when the video is stopped or finished;
    // and in those cases, the new size is reported as (0, 0).

    // In OSX, nativeSizeChanged is called when the video starts, but with the initial size of the video,
    // not (0,0), so we lose user own resizes if we perform it.
#ifndef Q_OS_OSX
    if (newSize != QSizeF(0,0))
        this->setSize(newSize.width(), newSize.height());
    else // Make sure the toolbar doesn't disappear
        Delegate()->showToolBar(false);
#else
    Q_UNUSED(newSize);
#endif
}

void UBGraphicsVideoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // When selected, a QGraphicsRectItem is drawn with a dashed line border. We don't want this
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;

    QGraphicsRectItem::paint(painter, &styleOption, widget);
    UBGraphicsMediaItem::paint(painter, option, widget);

}

QVariant UBGraphicsVideoItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == QGraphicsItem::ItemVisibleChange
            && value.toBool()
            && !mHasVideoOutput
            && UBApplication::app()->boardController
            && UBApplication::app()->boardController->activeScene() == scene())
    {
        //qDebug() << "Item change, setting video output";

        mMediaObject->setVideoOutput(mVideoItem);
        mHasVideoOutput = true;
    }

    return UBGraphicsMediaItem::itemChange(change, value);
}

void UBGraphicsVideoItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // Display the seek bar
    Delegate()->showToolBar();
    QGraphicsRectItem::hoverEnterEvent(event);
}

void UBGraphicsVideoItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    Delegate()->showToolBar();
    QGraphicsRectItem::hoverMoveEvent(event);
}

void UBGraphicsVideoItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsRectItem::hoverLeaveEvent(event);
}

void UBGraphicsVideoItem::hasVideoChanged(bool hasVideo)
{
    // On Linux, this is called (with hasVideo == true) when the video is first played
    // and when it finishes (hasVideo == false). But on Windows and OS X, it isn't called when
    // the video finishes, so those platforms require another solution to showing/hiding the
    // placeholder black rectangle.

    setPlaceholderVisible(!hasVideo);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void UBGraphicsVideoItem::mediaStateChanged(QMediaPlayer::PlaybackState state)
#else
void UBGraphicsVideoItem::mediaStateChanged(QMediaPlayer::State state)
#endif
{

#if defined(Q_OS_OSX) || defined(Q_OS_WIN)
    setPlaceholderVisible((state == QMediaPlayer::StoppedState));
#else
    Q_UNUSED(state);
#endif

}

void UBGraphicsVideoItem::activeSceneChanged()
{
    //qDebug() << "Active scene changed";

    // Update the visibility of the placeholder, to prevent it being hidden when switching pages
    setPlaceholderVisible(!mErrorString.isEmpty());

    // Call setVideoOutput, if the video is visible and if it hasn't been called already
    if (!mHasVideoOutput && UBApplication::boardController->activeScene() == scene()) {
        //qDebug() << "setting video output";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        mMediaObject->setSource(mMediaFileUrl);
#else
        mMediaObject->setMedia(mMediaFileUrl);
#endif
        mMediaObject->setVideoOutput(mVideoItem);
        mHasVideoOutput = true;
    }

    UBGraphicsMediaItem::activeSceneChanged();
}

void UBGraphicsVideoItem::mediaError(QMediaPlayer::Error errorCode)
{
    setPlaceholderVisible(errorCode != QMediaPlayer::NoError);
}

/**
 * @brief Set the brush and fill to display a black rectangle
 * @param visible If true, a black rectangle is displayed in place of the video
 *
 * Depending on platforms, when a video is finished or stopped, the video might be
 * removed altogether. To avoid just having the controls bar visible at that point,
 * we can display a "fake" black video frame in its place using this method.
 */
void UBGraphicsVideoItem::setPlaceholderVisible(bool visible)
{
    if (visible) {
        setBrush(QColor(Qt::black));
        setPen(QColor(Qt::white));
    }
    else {
        setBrush(QColor(Qt::transparent));
        setPen(QColor(Qt::transparent));
    }

}
