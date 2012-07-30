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

#ifndef UBGRAPHICSSVGITEM_H_
#define UBGRAPHICSSVGITEM_H_

#include <QtGui>
#include <QtSvg>

#include "UBItem.h"

#include "core/UB.h"

class UBGraphicsItemDelegate;
class UBGraphicsPixmapItem;

class UBGraphicsSvgItem: public QGraphicsSvgItem, public UBItem, public UBGraphicsItem
{
    public:
        UBGraphicsSvgItem(const QString& pFile, QGraphicsItem* parent = 0);
        UBGraphicsSvgItem(const QByteArray& pFileData, QGraphicsItem* parent = 0);

        void init();

        virtual ~UBGraphicsSvgItem();

        QByteArray fileData() const;

        void setFileData(const QByteArray& pFileData)
        {
            mFileData = pFileData;
        }

        enum { Type = UBGraphicsItemType::SvgItemType };

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

        virtual void setUuid(const QUuid &pUuid);

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

//        UBGraphicsItemDelegate* mDelegate;

        QByteArray mFileData;
};

#endif /* UBGRAPHICSSVGITEM_H_ */
