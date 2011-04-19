/*
 * UBGraphicsPixmapItem.h
 *
 *  Created on: Sep 18, 2008
 *      Author: luc
 */

#ifndef UBGRAPHICSPIXMAPITEM_H_
#define UBGRAPHICSPIXMAPITEM_H_

#include <QtGui>

#include "core/UB.h"

#include "UBItem.h"

class UBGraphicsItemDelegate;

class UBGraphicsPixmapItem : public QObject, public QGraphicsPixmapItem, public UBItem, public UBGraphicsItem
{
    Q_OBJECT;

    public:
        UBGraphicsPixmapItem(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsPixmapItem();

        enum { Type = UBGraphicsItemType::PixmapItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

        virtual UBGraphicsScene* scene();

        virtual void remove();

        Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity);

        void setOpacity(qreal op);
        qreal opacity() const;

protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        UBGraphicsItemDelegate* mDelegate;

};

#endif /* UBGRAPHICSPIXMAPITEM_H_ */
