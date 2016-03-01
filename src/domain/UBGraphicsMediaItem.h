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




#ifndef UBGRAPHICSMEDIAITEM_H
#define UBGRAPHICSMEDIAITEM_H

#include <QtWidgets/QGraphicsView>
#include "UBGraphicsProxyWidget.h"

#include <QAudioOutput>
#include <QMediaObject>
#include <QMediaPlayer>
#include <QMediaService>

#include <QtMultimediaWidgets/QVideoWidget>
#include <QtMultimedia/QVideoFrame>

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "frameworks/UBFileSystemUtils.h"

class QGraphicsVideoItem;

class UBGraphicsMediaItem : public QObject, public UBItem, public UBGraphicsItem, public QGraphicsRectItem, public UBResizableGraphicsItem
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

    virtual UBGraphicsScene* scene();
    bool hasLinkedImage() const             { return haveLinkedImage; }
    virtual QUrl mediaFileUrl() const       { return mMediaFileUrl; }
    bool isMuted() const                    { return mMuted; }
    qint64 initialPos() const               { return mInitialPos; }

    bool isMediaSeekable() const;
    qint64 mediaDuration() const;
    qint64 mediaPosition() const;

    QMediaPlayer::State playerState() const;

    QRectF boundingRect() const;

    QSizeF size() const { return rect().size(); } 

    // Setters
    virtual void setMediaFileUrl(QUrl url);
    void setInitialPos(qint64 p);
    void setMediaPos(qint64 p);
    virtual void setSourceUrl(const QUrl &pSourceUrl);
    void setSelected(bool selected);
    void setMinimumSize(const QSize& size);

    virtual void copyItemParameters(UBItem *copy) const;

    virtual void setSize(int width, int height);
    void resize(qreal w, qreal h) { setSize(w, h); }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


public slots:

    void toggleMute();
    void setMute(bool bMute);
    void activeSceneChanged();
    void hasMediaChanged(bool hasMedia);
    void showOnDisplayChanged(bool shown);

    virtual void play();
    virtual void pause();
    virtual void stop();
    virtual void togglePlayPause();

protected slots:
    void mediaError(QMediaPlayer::Error errorCode);

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    virtual void clearSource();

    QMediaPlayer *mMediaObject;

    QSize mMinimumSize;

    bool mMuted;
    bool mMutedByUserAction;
    static bool sIsMutedByDefault;

    QUrl mMediaFileUrl;
    QString mMediaSource;

    bool mShouldMove;
    QPointF mMousePressPos;
    QPointF mMouseMovePos;

    bool haveLinkedImage;
    QGraphicsPixmapItem *mLinkedImage;

    qint64 mInitialPos;
};

class UBGraphicsAudioItem: public UBGraphicsMediaItem
{
    Q_OBJECT

public:

    UBGraphicsAudioItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent = 0);
    mediaType getMediaType() const { return mediaType_Audio; }

    virtual UBItem* deepCopy() const;
};

class UBGraphicsVideoItem: public UBGraphicsMediaItem
{
    Q_OBJECT

public:

    UBGraphicsVideoItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent = 0);

    mediaType getMediaType() const { return mediaType_Video; }

    void setSize(int width, int height);

    virtual UBItem* deepCopy() const;

public slots:
    void videoSizeChanged(QSizeF newSize);

protected:

    QGraphicsVideoItem *mVideoItem;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
};


#endif // UBGRAPHICSMEDIAITEM_H
