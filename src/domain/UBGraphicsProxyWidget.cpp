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

    mDelegate = new UBGraphicsItemDelegate(this,0);
    mDelegate->init();

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    QGraphicsProxyWidget::setAcceptHoverEvents(true);
}


UBGraphicsProxyWidget::~UBGraphicsProxyWidget()
{
    if (mDelegate)
        delete mDelegate;
}


QVariant UBGraphicsProxyWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
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

    QVariant newValue = mDelegate->itemChange(change, value);
    return QGraphicsProxyWidget::itemChange(change, newValue);
}


void UBGraphicsProxyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mousePressEvent(event))
    {
        //NOOP
    }
    else
    {
        // QT Proxy Widget is a bit lazy, we force the selection ...
        QGraphicsProxyWidget::mousePressEvent(event);
        setSelected(true);
    }
}


void UBGraphicsProxyWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mouseMoveEvent(event))
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
    mDelegate->mouseReleaseEvent(event);

    QGraphicsProxyWidget::mouseReleaseEvent(event);
}


void UBGraphicsProxyWidget::setDelegate(UBGraphicsItemDelegate* pDelegate)
{
    if (mDelegate)
    {
        delete mDelegate;
    }

    mDelegate = pDelegate;
}


void UBGraphicsProxyWidget::resize(qreal w, qreal h)
{
    UBGraphicsProxyWidget::resize(QSizeF(w, h));
}


void UBGraphicsProxyWidget::resize(const QSizeF & pSize)
{
    if (pSize != size())
    {
        QGraphicsProxyWidget::setMaximumSize(pSize.width(), pSize.height());
        QGraphicsProxyWidget::resize(pSize.width(), pSize.height());
        if (widget())
            widget()->resize(pSize.width(), pSize.height());
        if (mDelegate)
            mDelegate->positionHandles();
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


void UBGraphicsProxyWidget::remove()
{
    if (mDelegate)
        mDelegate->remove(true);
}
