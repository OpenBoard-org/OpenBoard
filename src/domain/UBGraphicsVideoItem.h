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
    virtual UBGraphicsItemDelegate *Delegate() const {return mDelegate;}


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
