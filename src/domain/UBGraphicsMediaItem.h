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

class UBGraphicsMediaItem : public UBGraphicsProxyWidget
{
    Q_OBJECT

public:
    typedef enum{
        mediaType_Video,
        mediaType_Audio
    } mediaType;

    UBGraphicsMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent = 0);
    ~UBGraphicsMediaItem();

    enum { Type = UBGraphicsItemType::MediaItemType };

    virtual int type() const
    {
        return Type;
    }

    virtual QUrl mediaFileUrl() const
    {
        return mMediaFileUrl;
    }

    virtual void mediaFileUrl(QUrl url){mMediaFileUrl=url;}

    QMediaPlayer* mediaObject() const
    {
        return mMediaObject;
    }

    void setInitialPos(qint64 p) {
        mInitialPos = p;
    }
    qint64 initialPos() {
        return mInitialPos;
    }

    bool isMuted() const
    {
        return mMuted;
    }

    QGraphicsVideoItem * videoItem() const
    {
        return mVideoItem;
    }

    bool hasLinkedImage(){return haveLinkedImage;}

    mediaType getMediaType() { return mMediaType; }

    virtual UBGraphicsScene* scene();

    virtual UBItem* deepCopy() const;

    virtual void copyItemParameters(UBItem *copy) const;

    virtual void setSourceUrl(const QUrl &pSourceUrl);

    void setSelected(bool selected);


public slots:

    void toggleMute();
    void setMute(bool bMute);
    void activeSceneChanged();
    void hasMediaChanged(bool hasMedia);
    void showOnDisplayChanged(bool shown);
    virtual void resize(qreal w, qreal h);
    virtual void resize(const QSizeF & pSize);

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void clearSource();

    QMediaPlayer *mMediaObject;
    QWidget *mDummyVideoWidget;
    QGraphicsVideoItem *mVideoItem;

    QWidget *mAudioWidget;

private:

    bool mMuted;
    bool mMutedByUserAction;
    static bool sIsMutedByDefault;

    QUrl mMediaFileUrl;
    QString mMediaSource;

    mediaType mMediaType;

    bool mShouldMove;
    QPointF mMousePressPos;
    QPointF mMouseMovePos;

    bool haveLinkedImage;
    QGraphicsPixmapItem *mLinkedImage;

    qint64 mInitialPos;
};
#endif // UBGRAPHICSMEDIAITEM_H
