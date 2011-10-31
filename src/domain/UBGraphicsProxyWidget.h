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

        virtual UBGraphicsItemDelegate* delegate() const { return mDelegate;}

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);


        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

//        UBGraphicsItemDelegate* mDelegate;
};

#endif /* UBGRAPHICSPROXYWIDGET_H_ */
