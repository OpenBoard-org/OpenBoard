#ifndef UBGRAPHICSVIDEOITEM_H
#define UBGRAPHICSVIDEOITEM_H

#include <phonon/VideoWidget>
#include "UBGraphicsMediaItem.h"
#include "core/UB.h"

class UBGraphicsVideoItem : public UBGraphicsMediaItem
{
    Q_OBJECT;

public:
    UBGraphicsVideoItem(const QUrl& pMediaFileUrl, QGraphicsItem *parent = 0);
    ~UBGraphicsVideoItem();

    enum { Type = UBGraphicsItemType::VideoItemType };

    virtual int type() const
    {
        return Type;
    }

    virtual UBItem* deepCopy() const;

    Phonon::VideoWidget* videoWidget() const
    {
        return mVideoWidget;
    }


public slots:
    void hasVideoChanged(bool hasVideo);


protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    Phonon::VideoWidget *mVideoWidget;

private slots:
    void showOnDisplayChanged(bool shown);


private:
    bool mShouldMove;
    QPointF mMousePressPos;
    QPointF mMouseMovePos;


};

#endif // UBGRAPHICSVIDEOITEM_H
