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

#ifndef UBGRAPHICSPDFITEM_H_
#define UBGRAPHICSPDFITEM_H_

#include <QtGui>

#include "UBItem.h"

#include "core/UB.h"
#include "pdf/GraphicsPDFItem.h"

class UBGraphicsItemDelegate;
class UBGraphicsPixmapItem;

class UBGraphicsPDFItem: public GraphicsPDFItem, public UBItem, public UBGraphicsItem
{
    public:
        UBGraphicsPDFItem(PDFRenderer *renderer, int pageNumber, QGraphicsItem* parent = 0);
        virtual ~UBGraphicsPDFItem();

        enum { Type = UBGraphicsItemType::PDFItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

        virtual void setRenderingQuality(RenderingQuality pRenderingQuality);

        virtual UBGraphicsScene* scene();

        virtual void remove();

        virtual UBGraphicsPixmapItem* toPixmapItem() const;
        virtual UBGraphicsItemDelegate *Delegate() const {return mDelegate;}

        virtual void clearSource(){;}
        virtual void setUuid(const QUuid &pUuid);

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

//        UBGraphicsItemDelegate* mDelegate;
};

#endif /* UBGRAPHICSPDFITEM_H_ */
