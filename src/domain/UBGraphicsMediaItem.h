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




#ifndef UBGRAPHICSMEDIAITEM_H
#define UBGRAPHICSMEDIAITEM_H

#include <QtWidgets/QGraphicsView>

// #include <QAudioOutput>
#include <QMediaPlayer>

// #include <QtMultimediaWidgets/QVideoWidget>
// #include <QtMultimedia/QVideoFrame>

#include "board/UBBoardController.h"
#include "domain/UBMediaAssetItem.h"
#include "domain/UBResizableGraphicsItem.h"

class QGraphicsVideoItem;

class UBGraphicsMediaItem : public QObject, public UBMediaAssetItem, public UBGraphicsItem, public QGraphicsRectItem, public UBResizableGraphicsItem
{
    Q_OBJECT

public:
    typedef enum{
        mediaType_Video,
        mediaType_Audio
    } mediaType;

    enum { Type = UBGraphicsItemType::MediaItemType };

    virtual int type() const
    {
        return Type;
    }

    static UBGraphicsMediaItem* createMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem* parent = 0);

    UBGraphicsMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem* parent = 0);
    ~UBGraphicsMediaItem();


    // Getters

    virtual mediaType getMediaType() const = 0;

    virtual std::shared_ptr<UBGraphicsScene> scene() override;
    bool hasLinkedImage() const             { return haveLinkedImage; }
    virtual QUrl mediaFileUrl() const       { return mMediaFileUrl; }
    bool isMuted() const                    { return mMuted; }
    qint64 initialPos() const               { return mInitialPos; }

    bool isMediaSeekable() const;
    qint64 mediaDuration() const;
    qint64 mediaPosition() const;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QMediaPlayer::PlaybackState playerState() const;
    bool isPlaying() const { return (mMediaObject->playbackState() == QMediaPlayer::PlayingState); }
    bool isPaused() const { return (mMediaObject->playbackState() == QMediaPlayer::PausedState); }
#else
    QMediaPlayer::State playerState() const;
    bool isPlaying() const { return (mMediaObject->state() == QMediaPlayer::PlayingState); }
    bool isPaused() const { return (mMediaObject->state() == QMediaPlayer::PausedState); }
#endif

    bool isStopped() const;
    bool firstLoad() const;
    void setFirstLoad(bool firstLoad);

    QRectF boundingRect() const override;

    QSizeF size() const override { return rect().size(); }

    // Setters
    virtual void setMediaAsset(const QString& documentPath, const QString& mediaAsset) override;
    virtual void setMediaFileUrl(QUrl url);
    void setInitialPos(qint64 p);
    void setMediaPos(qint64 p);
    virtual void setSourceUrl(const QUrl &pSourceUrl) override;
    void setSelected(bool selected);
    void setMinimumSize(const QSize& size);

    virtual void copyItemParameters(UBItem *copy) const override;

    virtual void setSize(int width, int height);
    void resize(qreal w, qreal h) override { setSize(w, h); }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;


public slots:

    void toggleMute();
    void setMute(bool bMute);
    void activeSceneChanged();
    void showOnDisplayChanged(bool shown);

    virtual void play();
    virtual void pause();
    virtual void stop();
    virtual void togglePlayPause();

protected slots:
    void mediaError(QMediaPlayer::Error errorCode);

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    QMediaPlayer *mMediaObject;

    QSize mMinimumSize;

    bool mMuted;
    bool mMutedByUserAction;
    static bool sIsMutedByDefault;
    bool mStopped;
    bool mFirstLoad;

    QUrl mMediaFileUrl;
    QString mMediaSource;

    bool mShouldMove;
    QPointF mMousePressPos;
    QPointF mMouseMovePos;

    bool haveLinkedImage;
    QGraphicsPixmapItem *mLinkedImage;

    qint64 mInitialPos;

    QString mErrorString;
};

class UBGraphicsAudioItem: public UBGraphicsMediaItem
{
    Q_OBJECT

public:
    enum { Type = UBGraphicsItemType::AudioItemType};
    virtual int type() const override
    {
        return Type;
    }

    UBGraphicsAudioItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent = 0);

    virtual QList<QString> mediaAssets() const override;

    mediaType getMediaType() const override { return mediaType_Audio; }

    virtual UBItem* deepCopy() const override;
};

class UBGraphicsVideoItem: public UBGraphicsMediaItem
{
    Q_OBJECT

public:
    enum { Type = UBGraphicsItemType::VideoItemType};
    virtual int type() const override
    {
        return Type;
    }

    UBGraphicsVideoItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent = 0);

    virtual QList<QString> mediaAssets() const override;

    mediaType getMediaType() const override { return mediaType_Video; }

    void setSize(int width, int height) override;

    virtual UBItem* deepCopy() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public slots:
    void videoSizeChanged(QSizeF newSize);
    void hasVideoChanged(bool hasVideo);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void mediaStateChanged(QMediaPlayer::PlaybackState state);
#else
    void mediaStateChanged(QMediaPlayer::State state);
#endif

    void activeSceneChanged();

protected slots:
    void mediaError(QMediaPlayer::Error errorCode);

protected:

    QGraphicsVideoItem *mVideoItem;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void setPlaceholderVisible(bool visible);

    bool mHasVideoOutput;
};


#endif // UBGRAPHICSMEDIAITEM_H
