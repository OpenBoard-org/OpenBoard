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

#include "UBGraphicsProxyWidget.h"

#include <QtGui>

#include "UBGraphicsScene.h"
#include "UBGraphicsItemDelegate.h"

#include "UBGraphicsDelegateFrame.h"

#include "core/memcheck.h"

UBGraphicsProxyWidget::UBGraphicsProxyWidget(QGraphicsItem* parent)
    : QGraphicsProxyWidget(parent, Qt::FramelessWindowHint)
{
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    //UBGraphicsItemDelegate* delegate = new UBGraphicsItemDelegate(this,0, true, false, false);
    //delegate->init();
    //setDelegate(delegate);

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    QGraphicsProxyWidget::setAcceptHoverEvents(true);
}


UBGraphicsProxyWidget::~UBGraphicsProxyWidget()
{
}


QVariant UBGraphicsProxyWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemCursorHasChanged &&  scene())
    {
        unsetCursor();
    }
    if ((change == QGraphicsItem::ItemSelectedHasChanged)
              &&  scene())
    {
        if (isSelected())
        {
            scene()->setActiveWindow(this);
        }
        else
        {
            if(scene()->activeWindow() == this)
            {
                scene()->setActiveWindow(0);
            }
        }
    }

    QVariant newValue = Delegate()->itemChange(change, value);
    return QGraphicsProxyWidget::itemChange(change, newValue);
}

void UBGraphicsProxyWidget::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

void UBGraphicsProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mousePressEvent(event))
    {
        //NOOP
    }
    else
    {
        // QT Proxy Widget is a bit lazy, we force the selection ...

        setSelected(true);
    }
    QGraphicsProxyWidget::mousePressEvent(event);
}


void UBGraphicsProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mouseMoveEvent(event))
    {
        // NOOP;
    }
    else
    {
        QGraphicsProxyWidget::mouseMoveEvent(event);
    }
}


void UBGraphicsProxyWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Delegate()->mouseReleaseEvent(event);
    QGraphicsProxyWidget::mouseReleaseEvent(event);
}

void UBGraphicsProxyWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if( Delegate()->weelEvent(event) )
    {
        QGraphicsProxyWidget::wheelEvent(event);
        event->accept();
    }
}

void UBGraphicsProxyWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
//    NOOP
}
void UBGraphicsProxyWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
//    NOOP
}

void UBGraphicsProxyWidget::resize(qreal w, qreal h)
{
    UBGraphicsProxyWidget::resize(QSizeF(w, h));
}


void UBGraphicsProxyWidget::resize(const QSizeF & pSize)
{
    if (pSize != size())
    {
        qreal sizeX = 0;
        qreal sizeY = 0;

        if (widget())
        {
            
            QSizeF minimumItemSize(widget()->minimumSize());
            if (minimumItemSize.width() > pSize.width())
                sizeX = minimumItemSize.width();
            else
                sizeX = pSize.width();

            if (minimumItemSize.height() > pSize.height())
                sizeY = minimumItemSize.height();
            else
                sizeY = pSize.height();
        }
        QSizeF size(sizeX, sizeY);


        QGraphicsProxyWidget::setMaximumSize(size.width(), size.height());
        QGraphicsProxyWidget::resize(size.width(), size.height());
        if (widget())
            widget()->resize(size.width(), size.height());
        if (Delegate())
            Delegate()->positionHandles();
        if (scene())
            scene()->setModified(true);
    }
}


QSizeF UBGraphicsProxyWidget::size() const
{
    return QGraphicsProxyWidget::size();
}


UBGraphicsScene* UBGraphicsProxyWidget::scene()
{
    return static_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


