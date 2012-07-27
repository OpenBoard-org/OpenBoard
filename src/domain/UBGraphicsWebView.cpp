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

#include "UBGraphicsWebView.h"

#include <QtGui>
#include <QtWebKit>

#include "UBGraphicsScene.h"
#include "UBGraphicsItemDelegate.h"

#include "UBGraphicsDelegateFrame.h"

#include "core/memcheck.h"

UBGraphicsWebView::UBGraphicsWebView(QGraphicsItem* parent) :
    QGraphicsWebView(parent)
{
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    mDelegate = new UBGraphicsItemDelegate(this,0, true, false, false);
    mDelegate->init();

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    QGraphicsWebView::setAcceptHoverEvents(true);
}


UBGraphicsWebView::~UBGraphicsWebView()
{
    if (mDelegate)
        delete mDelegate;
}


QVariant UBGraphicsWebView::itemChange(GraphicsItemChange change, const QVariant &value)
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

    QVariant newValue = mDelegate->itemChange(change, value);
    return QGraphicsWebView::itemChange(change, newValue);
}

void UBGraphicsWebView::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

void UBGraphicsWebView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mousePressEvent(event))
    {
        //NOOP
    }
    else
    {
        // QT Proxy Widget is a bit lazy, we force the selection ...

        setSelected(true);
    }
    QGraphicsWebView::mousePressEvent(event);
}


void UBGraphicsWebView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mouseMoveEvent(event))
    {
        // NOOP;
    }
    else
    {
        QGraphicsWebView::mouseMoveEvent(event);
    }
}


void UBGraphicsWebView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mDelegate->mouseReleaseEvent(event);
    QGraphicsWebView::mouseReleaseEvent(event);
}

void UBGraphicsWebView::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if( mDelegate->weelEvent(event) )
    {
        QGraphicsWebView::wheelEvent(event);
        event->accept();
    }
}

void UBGraphicsWebView::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
//    NOOP
}
void UBGraphicsWebView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
//    NOOP
}

void UBGraphicsWebView::setDelegate(UBGraphicsItemDelegate* pDelegate)
{
    if (mDelegate)
    {
        delete mDelegate;
    }

    mDelegate = pDelegate;
}


void UBGraphicsWebView::resize(qreal w, qreal h)
{
    UBGraphicsWebView::resize(QSizeF(w, h));
}


void UBGraphicsWebView::resize(const QSizeF & pSize)
{
    if (pSize != size())
    {
        QGraphicsWebView::setMaximumSize(pSize.width(), pSize.height());
        QGraphicsWebView::resize(pSize.width(), pSize.height());
        if (mDelegate)
            mDelegate->positionHandles();
        if (scene())
            scene()->setModified(true);
    }
}

QSizeF UBGraphicsWebView::size() const
{
    return QGraphicsWebView::size();
}


UBGraphicsScene* UBGraphicsWebView::scene()
{
    return static_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsWebView::remove()
{
    if (mDelegate)
        mDelegate->remove(true);
}
