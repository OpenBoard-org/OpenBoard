/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBEditableGraphicsLineItem.h"

#include "UBLineHandle.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBApplication.h"


UBEditableGraphicsLineItem::UBEditableGraphicsLineItem(QGraphicsItem* parent)
    : UBEditableGraphicsPolygonItem(parent)
{
    // Line has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    UBLineHandle* startHandle = new UBLineHandle;
    UBLineHandle* endHandle = new UBLineHandle;

    endHandle->setId(1);

    startHandle->setParentItem(this);
    endHandle->setParentItem(this);

    startHandle->setEditableObject(this);
    endHandle->setEditableObject(this);

    startHandle->setOppositeHandle(endHandle);
    endHandle->setOppositeHandle(startHandle);

    startHandle->hide();
    endHandle->hide();

    mHandles.push_back(startHandle);
    mHandles.push_back(endHandle);
}

UBEditableGraphicsLineItem::~UBEditableGraphicsLineItem()
{
}

ShapeType UBEditableGraphicsLineItem::shapeType()
{
    return ShapeType::Line;
}

UBItem* UBEditableGraphicsLineItem::deepCopy() const
{
    UBEditableGraphicsLineItem* copy = new UBEditableGraphicsLineItem();

    copyItemParameters(copy);

    return copy;
}

void UBEditableGraphicsLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    painter->drawPath(path());
}

QPointF UBEditableGraphicsLineItem::startPoint() const
{
    return path().elementAt(0);
}

QPointF UBEditableGraphicsLineItem::endPoint() const
{
    if (path().elementCount() == 2)
        return path().elementAt(1);
    else
        return path().elementAt(0);
}

void UBEditableGraphicsLineItem::copyItemParameters(UBItem* copy) const
{
    UBAbstractEditableGraphicsPathItem::copyItemParameters(copy);
}


void UBEditableGraphicsLineItem::setStartPoint(QPointF pos)
{
    prepareGeometryChange();

    QPainterPath p;

    p.moveTo(this->pos());

    if (path().elementCount() == 2)
        p.lineTo(path().elementAt(1));

    setPath(p);

    this->setPos(pos);

    mHandles.at(0)->setPos(pos);
}

void UBEditableGraphicsLineItem::setEndPoint(QPointF pos)
{
    prepareGeometryChange();

    QPainterPath p;

    p.moveTo(path().elementAt(0));

    p.lineTo(pos);

    setPath(p);

    mHandles.at(1)->setPos(pos);
}

void UBEditableGraphicsLineItem::updateHandle(UBAbstractHandle* handle)
{
    prepareGeometryChange();

    if (handle->getId() == 0)
    {
        QPainterPath p;

        p.moveTo(handle->pos());

        p.lineTo(path().elementAt(1));

        setPath(p);
    }
    else if (handle->getId() == 1)
    {
        QPainterPath p;

        p.moveTo(path().elementAt(0));

        p.lineTo(handle->pos());

        setPath(p);
    }

    const auto p1 = path().elementAt(1 - handle->getId());
    const auto p2 = path().elementAt((handle->getId()));

    QLineF line{p1, p2};
    QLineF viewRadius{UBApplication::boardController->controlView()->mapFromScene(line.p1()),
                      UBApplication::boardController->controlView()->mapFromScene(line.p2())};
    QPoint offset = -viewRadius.p2().toPoint();
    viewRadius.setLength(viewRadius.length() + 60);
    offset += viewRadius.p2().toPoint();

    UBApplication::boardController->setCursorFromAngle(line.angle(), offset);
}

void UBEditableGraphicsLineItem::setLine(QPointF start, QPointF end)
{
    prepareGeometryChange();

    QPainterPath p;
    p.moveTo(start);
    p.lineTo(end);

    setPath(p);
}

void UBEditableGraphicsLineItem::onActivateEditionMode()
{
    mHandles.at(0)->setPos(startPoint());
    mHandles.at(1)->setPos(endPoint());
}

QRectF UBEditableGraphicsLineItem::boundingRect() const
{
    if (isInEditMode())
    {
        return shape().boundingRect();
    }

    return UBEditableGraphicsPolygonItem::boundingRect();
}

void UBEditableGraphicsLineItem::addPoint(const QPointF& point)
{
    prepareGeometryChange();

    QPainterPath p(mapFromScene(point));

    if (path().elementCount() == 0)
    {
        mHandles.at(0)->setPos(point);
        p.moveTo(point);
    }
    else
    {
        // In the other cases we have just to change the last point
        p.moveTo(path().elementAt(0));
        p.lineTo(point);

        mHandles.at(1)->setPos(point);
    }

    setPath(p);
}
