/*
 * UBGraphicsProxyWidget.h
 *
 *  Created on: Sep 18, 2008
 *      Author: luc
 */

#ifndef UBGRAPHICSPROXYWIDGET_H_
#define UBGRAPHICSPROXYWIDGET_H_

#include <QtGui>


#include "UBItem.h"
#include "UBResizableGraphicsItem.h"

class UBGraphicsItemDelegate;

class UBGraphicsProxyWidget: public QGraphicsProxyWidget, public UBItem, public UBResizableGraphicsItem, public UBGraphicsItem
{
    public:
        UBGraphicsProxyWidget(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsProxyWidget();

        virtual void resize(qreal w, qreal h);
        virtual void resize(const QSizeF & size);

        virtual QSizeF size() const;

        void setDelegate(UBGraphicsItemDelegate* pDelegate);

        virtual UBGraphicsScene* scene();

        virtual void remove();

        UBGraphicsItemDelegate* delegate () { return mDelegate;};

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        UBGraphicsItemDelegate* mDelegate;


};

#endif /* UBGRAPHICSPROXYWIDGET_H_ */
