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


#include "UBLineHandle.h"
#include "domain/UBGraphicsScene.h"
#include "domain/shapes/UBAbstractEditable.h"


UBLineHandle::UBLineHandle()
{
}

UBLineHandle::UBLineHandle(UBLineHandle* const src)
    : UBAbstractHandle(src)
{
}

void UBLineHandle::setOppositeHandle(UBLineHandle* oppositeHandle)
{
    mOppositeHandle = oppositeHandle;
}

UBItem* UBLineHandle::deepCopy() const
{
    UBLineHandle* copy = new UBLineHandle;
    copyItemParameters(copy);

    return copy;
}

void UBLineHandle::copyItemParameters(UBItem* copy) const
{
    UBLineHandle* cp = dynamic_cast<UBLineHandle*>(copy);

    if (cp)
    {
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));

        cp->setPos(pos());
        cp->setEditableObject(cp->editableObject());
    }
}

void UBLineHandle::oppositeHandleMoved()
{
    if (mOppositeHandle)
    {
        const auto angle = QLineF(pos(), mOppositeHandle->pos()).angle();
        QTransform transform;
        transform.rotate(-angle);
        setTransform(transform);
    }
}

QVariant UBLineHandle::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemVisibleChange == value.toBool())
    {
        oppositeHandleMoved();
    }

    return QGraphicsItem::itemChange(change, value);
}

void UBLineHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    auto scenePos = event->scenePos();

    if (mGuided && mOppositeHandle)
    {
        // make line longer or shorter, keeping the angle
        QLineF line{mOppositeHandle->pos(), pos()};
        const auto angle = line.angle();
        line.setP2(parentItem()->mapFromScene(scenePos));
        line.setAngle(angle);
        setPos(line.p2());
    }
    else
    {
        std::shared_ptr<UBGraphicsScene> scenePtr = scene();

        if (scenePtr->isSnapping())
        {
            const auto step = UBSettings::settings()->rotationAngleStep->get().toDouble();
            const auto lineStartPoint = parentItem()->mapToScene(mOppositeHandle->pos());
            QLineF radius(lineStartPoint, scenePos);
            auto angle = radius.angle();
            angle = qRound(angle / step) * step;
            radius.setAngle(angle);
            auto altPosition = radius.p2();

            QPointF gridSnapPoint;
            scenePos += scenePtr->snap(scenePos, nullptr, altPosition, &gridSnapPoint);

            if (scenePos != gridSnapPoint)
            {
                const auto angle1 = QLineF{lineStartPoint, scenePos}.angle();
                const auto angle2 = QLineF{lineStartPoint, gridSnapPoint}.angle();

                if (std::fmod(std::fabs(angle1 - angle2), 360.) < 0.01)
                {
                    scenePos = gridSnapPoint;
                }
            }
        }

        QPointF p = parentItem()->mapFromScene(scenePos);
        setPos(p);
    }

    if (mOppositeHandle)
    {
        oppositeHandleMoved();
        mOppositeHandle->oppositeHandleMoved();
    }

    mEditableObject->updateHandle(this);
    mEditableObject->setModified();
}

void UBLineHandle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    UBAbstractHandle::mousePressEvent(event);

    mGuided = !UBAbstractHandle::boundingRect().contains(event->pos());
}

QRectF UBLineHandle::boundingRect() const
{
    static const int gap = 7;
    const double width = 2 * mRadius + gap;
    const double height = mRadius + 2;

    return QRectF{QPointF{-width, -height}, QPointF{width, height}};
}

void UBLineHandle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    UBAbstractHandle::paint(painter, option, widget);

    const auto rect = boundingRect();
    const auto botttomRight = rect.bottomRight();
    const auto dist = botttomRight.x() - botttomRight.y();

    QPainterPath path;

    path.moveTo(dist, rect.top());
    path.lineTo(rect.right(), 0);
    path.lineTo(dist, rect.bottom());
    path.closeSubpath();

    path.moveTo(-dist, rect.top());
    path.lineTo(rect.left(), 0);
    path.lineTo(-dist, rect.bottom());
    path.closeSubpath();

    painter->drawPath(path);
}
