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


#include <QtGui>
#include <QtWebKit>

#include "UBGraphicsWebView.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"

#include "core/memcheck.h"

UBGraphicsWebView::UBGraphicsWebView(QGraphicsItem* parent)
    : QGraphicsWebView(parent)
{
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    mDelegate = new UBGraphicsItemDelegate(this, 0, true);
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
    if ((change == QGraphicsItem::ItemSelectedHasChanged) &&  scene()) {
        if (isSelected())
            scene()->setActiveWindow(this);
        else
            if(scene()->activeWindow() == this)
                scene()->setActiveWindow(0);
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
    if (!mDelegate->mousePressEvent(event))
        setSelected(true); /* forcing selection */

    QGraphicsWebView::mousePressEvent(event);
}


void UBGraphicsWebView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mDelegate->mouseMoveEvent(event))
        QGraphicsWebView::mouseMoveEvent(event);
}


void UBGraphicsWebView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mDelegate->mouseReleaseEvent(event);
    QGraphicsWebView::mouseReleaseEvent(event);
}

void UBGraphicsWebView::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (mDelegate->weelEvent(event))
    {
        QGraphicsWebView::wheelEvent(event);
        event->accept();
    }
}

void UBGraphicsWebView::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    /* NOOP */
}
void UBGraphicsWebView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    /* NOOP */
}

void UBGraphicsWebView::setDelegate(UBGraphicsItemDelegate* pDelegate)
{
    if (mDelegate)
        delete mDelegate;

    mDelegate = pDelegate;
}


void UBGraphicsWebView::resize(qreal w, qreal h)
{
    UBGraphicsWebView::resize(QSizeF(w, h));
}


void UBGraphicsWebView::resize(const QSizeF & pSize)
{
    if (pSize != size()) {
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
