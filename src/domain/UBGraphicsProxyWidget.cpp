/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
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

    mDelegate = new UBGraphicsItemDelegate(this,0, true, false, false);
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
    return QGraphicsProxyWidget::itemChange(change, newValue);
}

void UBGraphicsProxyWidget::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
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

        setSelected(true);
    }
    QGraphicsProxyWidget::mousePressEvent(event);
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

void UBGraphicsProxyWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if( mDelegate->weelEvent(event) )
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
