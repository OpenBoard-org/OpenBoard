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

#ifndef UBGRAPHICSWEBVIEW_H_
#define UBGRAPHICSWEBVIEW_H_

#include <QtGui>
#include <QtWebKit>

#include "UBItem.h"
#include "UBResizableGraphicsItem.h"

class UBGraphicsItemDelegate;

class UBGraphicsWebView: public QGraphicsWebView, public UBItem, public UBResizableGraphicsItem, public UBGraphicsItem
{
    public:
        UBGraphicsWebView(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsWebView();

        virtual void resize(qreal w, qreal h);
        virtual void resize(const QSizeF & size);

        virtual QSizeF size() const;

        void setDelegate(UBGraphicsItemDelegate* pDelegate);

        virtual UBGraphicsScene* scene();

        virtual void remove();

        virtual UBGraphicsItemDelegate* Delegate() const { return mDelegate;}

        virtual void clearSource(){;}
        virtual void setUuid(const QUuid &pUuid);

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        //virtual bool event(QEvent *event);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

};

#endif /* UBGRAPHICSWEBVIEW_H_ */
