/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#ifndef UBGRAPHICSCURTAINITEM_H_
#define UBGRAPHICSCURTAINITEM_H_

#include <QtGui>

#include "core/UB.h"

#include "domain/UBItem.h"

class UBGraphicsItemDelegate;


class UBGraphicsCurtainItem : public QObject, public QGraphicsRectItem, public UBItem, public UBGraphicsItem
{

    Q_OBJECT

    public:
        UBGraphicsCurtainItem(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsCurtainItem();

        enum { Type = UBGraphicsItemType::CurtainItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;
        virtual void copyItemParameters(UBItem *copy) const;

        virtual void remove();

        //TODO UB 4.x not nice ...
        void triggerRemovedSignal();
        virtual UBGraphicsItemDelegate* Delegate() const {return mDelegate;}
        virtual void clearSource(){;}

        virtual void setUuid(const QUuid &pUuid);

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

//        UBGraphicsItemDelegate* mDelegate;

        static const QColor sDrawColor;
        static const QColor sDarkBackgroundDrawColor;
        static const QColor sOpaqueControlColor;
        static const QColor sDarkBackgroundOpaqueControlColor;
};

#endif /* UBGRAPHICSCURTAINITEM_H_ */
