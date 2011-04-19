/*
 * UBGraphicsCurtainItem.h
 *
 *  Created on: May 25, 2009
 *      Author: Patrick
 */

#ifndef UBGRAPHICSCURTAINITEM_H_
#define UBGRAPHICSCURTAINITEM_H_

#include <QtGui>

#include "core/UB.h"

#include "domain/UBItem.h"

class UBGraphicsItemDelegate;


class UBGraphicsCurtainItem : public QObject, public QGraphicsRectItem, public UBItem, public UBGraphicsItem
{

    Q_OBJECT;

    public:
        UBGraphicsCurtainItem(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsCurtainItem();

        enum { Type = UBGraphicsItemType::CurtainItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

        virtual void remove();

        //TODO UB 4.x not nice ...
        void triggerRemovedSignal();

     signals:

        void removed();

     protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        QColor  drawColor() const;
        QColor  opaqueControlColor() const;

        UBGraphicsItemDelegate* mDelegate;

        static const QColor sDrawColor;
        static const QColor sDarkBackgroundDrawColor;
        static const QColor sOpaqueControlColor;
        static const QColor sDarkBackgroundOpaqueControlColor;
};

#endif /* UBGRAPHICSCURTAINITEM_H_ */
