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
