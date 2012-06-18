/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
#ifndef UBGRAPHICSMEDIAITEM_H
#define UBGRAPHICSMEDIAITEM_H

#include "UBGraphicsProxyWidget.h"
#include <phonon/AudioOutput>
#include <phonon/MediaObject>
#include <phonon/VideoWidget>
#include "core/UBApplication.h"
#include "board/UBBoardController.h"


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

    void hasMediaChanged(bool hasMedia);
    void showOnDisplayChanged(bool shown);

    virtual QUrl mediaFileUrl() const
    {
        return mMediaFileUrl;
    }

    Phonon::MediaObject* mediaObject() const
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

    Phonon::VideoWidget* videoWidget() const
    {
        return mVideoWidget;
    }

    bool hasLinkedImage(){return haveLinkedImage;}

    mediaType getMediaType() { return mMediaType; }

    virtual UBGraphicsScene* scene();

    virtual UBItem* deepCopy() const;

public slots:

    void toggleMute();
    void activeSceneChanged();

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    virtual void clearSource();

    Phonon::MediaObject *mMediaObject;
    Phonon::VideoWidget *mVideoWidget;
    Phonon::AudioOutput *mAudioOutput;
    Phonon::MediaSource mSource;
    QWidget *mAudioWidget;

private:

    bool mMuted;
    bool mMutedByUserAction;
    static bool sIsMutedByDefault;

    QUrl mMediaFileUrl;
    QString mMediaSource;

    qint64 mInitialPos;

    mediaType mMediaType;

    bool mShouldMove;
    QPointF mMousePressPos;
    QPointF mMouseMovePos;

    bool haveLinkedImage;
    QGraphicsPixmapItem *mLinkedImage;
};


class UBGraphicsUnitedMediaItem : public UBGraphicsMediaItem 
{
public:
    UBGraphicsUnitedMediaItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent = 0);

};

#endif // UBGRAPHICSMEDIAITEM_H
