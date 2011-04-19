/*
 * UBGraphicsCurtainItemDelegate.h
 *
 *  Created on: June 15, 2009
 *      Author: Patrick
 */

#ifndef UBGRAPHICSCURTAINITEMDELEGATE_H_
#define UBGRAPHICSCURTAINITEMDELEGATE_H_

#include <QtGui>

#include <QtSvg>

#include "core/UB.h"
#include "domain/UBGraphicsItemDelegate.h"

class QGraphicsSceneMouseEvent;
class QGraphicsItem;
class UBGraphicsCurtainItem;


class UBGraphicsCurtainItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT;

    public:
        UBGraphicsCurtainItemDelegate(UBGraphicsCurtainItem* pDelegated, QObject * parent = 0);
        virtual ~UBGraphicsCurtainItemDelegate();

        virtual bool mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);

   public slots:

        virtual void remove(bool checked, bool canUndo = true);

   protected:
        virtual void init();

};

#endif /* UBGRAPHICSCURTAINITEMDELEGATE_H_ */
