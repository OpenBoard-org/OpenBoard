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


#include "UBEditableGraphicsPolygonItem.h"

#include "UBFreeHandle.h"


UBEditableGraphicsPolygonItem::UBEditableGraphicsPolygonItem(QGraphicsItem* parent)
    : UBAbstractEditableGraphicsPathItem(parent)
{
    initializeStrokeProperty();
    initializeFillingProperty();
}

UBEditableGraphicsPolygonItem::~UBEditableGraphicsPolygonItem()
{
}

void UBEditableGraphicsPolygonItem::addPoint(const QPointF& point)
{
    if (!mIsInCreationMode)
    {
        return;
    }

    prepareGeometryChange();

    QPointF p(mapFromScene(point));

    QPainterPath painterPath = path();

    if (painterPath.elementCount() == 0)
    {
        painterPath.moveTo(p); // For the first point added, we must use moveTo().
        setPath(painterPath);

        mStartEndPoint[0] = p;
    }
    else
    {
        // If click on first point, close the polygon
        QPointF pointDepart(painterPath.elementAt(0).x, painterPath.elementAt(0).y);
        QPointF pointFin(painterPath.elementAt(painterPath.elementCount() - 1).x,
                         painterPath.elementAt(painterPath.elementCount() - 1).y);

        QRectF handle(0, 0, HANDLE_SIZE, HANDLE_SIZE);
        handle.moveCenter(pointDepart);
        QGraphicsEllipseItem poigneeDepart(handle);

        handle.moveCenter(pointFin);
        QGraphicsEllipseItem poigneeFin(handle);

        if (poigneeDepart.contains(p))
        {
            setClosed(true);
        }
        else
        {
            if (poigneeFin.contains(p))
            {
                mIsInCreationMode = false;
                mOpened = true;
            }
            else
            {
                painterPath.lineTo(p);
                setPath(painterPath);
            }
        }

        mStartEndPoint[1] = p;
    }

    if (!mClosed && !mOpened)
    {

        UBFreeHandle* handle = new UBFreeHandle();

        addHandle(handle);

        handle->setParentItem(this);
        handle->setEditableObject(this);
        handle->setPos(p);
        handle->setId(path().elementCount() - 1);
        handle->hide();
    }
}

void UBEditableGraphicsPolygonItem::setIsInCreationMode(bool mode)
{
    mIsInCreationMode = mode;
    update();
}

void UBEditableGraphicsPolygonItem::setOpened(bool opened)
{
    mOpened = opened;
}

void UBEditableGraphicsPolygonItem::setClosed(bool closed)
{
    mClosed = closed;

    QPainterPath painterPath = path();

    if (closed)
    {
        painterPath.closeSubpath(); // Automatically add a last point, identic to the first point.
    }
    else
    {
        // if last point and first point are the same, remove the last one, in order to open the path.
        int nbElements = painterPath.elementCount();
        if (nbElements > 1)
        {
            QPainterPath::Element firstElement = painterPath.elementAt(0);
            QPainterPath::Element lastElement = painterPath.elementAt(nbElements - 1);

            QPointF firstPoint(firstElement.x, firstElement.y);
            QPointF lastPoint(lastElement.x, lastElement.y);

            if (firstPoint == lastPoint)
            {
                // Rebuild the path, excluding the last point.
                QPainterPath newPainterPath(firstPoint);
                for (int iElement = 1; iElement < nbElements - 1; iElement++)
                {
                    newPainterPath.lineTo(painterPath.elementAt(iElement));
                }
                painterPath = newPainterPath;
            }
        }
    }

    setPath(painterPath);
    mIsInCreationMode = false;
}


UBItem* UBEditableGraphicsPolygonItem::deepCopy() const
{
    UBEditableGraphicsPolygonItem* copy = new UBEditableGraphicsPolygonItem();

    copyItemParameters(copy);

    return copy;
}

void UBEditableGraphicsPolygonItem::copyItemParameters(UBItem* copy) const
{
    UBAbstractEditableGraphicsPathItem::copyItemParameters(copy);

    UBEditableGraphicsPolygonItem* cp = dynamic_cast<UBEditableGraphicsPolygonItem*>(copy);

    if (cp)
    {
        qDeleteAll(cp->mHandles);
        cp->mHandles.clear();

        for (int i = 0; i < mHandles.size(); i++)
        {
            UBFreeHandle* handle = new UBFreeHandle();

            handle->setParentItem(cp);
            handle->setEditableObject(cp);
            handle->setPos(mHandles.at(i)->pos());
            handle->setId(mHandles.at(i)->getId());
            handle->hide();

            cp->mHandles.push_back(handle);
        }

        cp->mIsInCreationMode = mIsInCreationMode;
        cp->mClosed = mClosed;
        cp->mStartEndPoint[0] = mStartEndPoint[0];
        cp->mStartEndPoint[1] = mStartEndPoint[1];
    }
}

ShapeType UBEditableGraphicsPolygonItem::shapeType()
{
    return ShapeType::Polygon;
}

QRectF UBEditableGraphicsPolygonItem::boundingRect() const
{
    QRectF rect = UBAbstractEditableGraphicsPathItem::boundingRect();

    const int enlarge = HANDLE_SIZE / 2;

    rect.adjust(-enlarge, -enlarge, enlarge, enlarge);

    return rect;
}

void UBEditableGraphicsPolygonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget)

    setStyle(painter);

    if (!this->isClosed())
        painter->setBrush(QBrush{});

    painter->drawPath(path());

    if (!isClosed())
    {
        QPen penHandles;
        penHandles.setWidth(1);
        penHandles.setColor(Qt::black);
        penHandles.setStyle(Qt::SolidLine);
        painter->setPen(penHandles);

        int hsize = HANDLE_SIZE / 2;

        if (mIsInCreationMode)
        {
            painter->drawEllipse(mStartEndPoint[0].x() - hsize, mStartEndPoint[0].y() - hsize, HANDLE_SIZE,
                                 HANDLE_SIZE);

            if (path().elementCount() >= 2)
                painter->drawEllipse(mStartEndPoint[1].x() - hsize, mStartEndPoint[1].y() - hsize, HANDLE_SIZE,
                                     HANDLE_SIZE);
        }
    }
}

void UBEditableGraphicsPolygonItem::updateHandle(UBAbstractHandle* handle)
{
    prepareGeometryChange();

    int id = handle->getId();

    QPainterPath oldPath = path();

    QPainterPath newPath;

    if (mClosed && id == 0)
    {
        newPath.moveTo(handle->pos());
        for (int i = 1; i < oldPath.elementCount() - 1; i++)
        {
            newPath.lineTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
        }
        newPath.lineTo(handle->pos());
    }
    else
    {
        for (int i = 0; i < oldPath.elementCount(); i++)
        {
            if (i == 0)
            {
                if (i == id)
                {
                    newPath.moveTo(handle->pos());
                }
                else
                {
                    newPath.moveTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
                }
            }
            else
            {
                if (i == id)
                {
                    newPath.lineTo(handle->pos());
                }
                else
                {
                    newPath.lineTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
                }
            }
        }
    }

    setPath(newPath);

    mStartEndPoint[0] = QPointF(path().elementAt(0).x, path().elementAt(0).y);
    mStartEndPoint[1] =
        QPointF(path().elementAt(path().elementCount() - 1).x, path().elementAt(path().elementCount() - 1).y);
    update();
}

bool UBEditableGraphicsPolygonItem::hasFillingProperty() const
{
    return isClosed() && UBAbstractGraphicsItem::hasFillingProperty();
}

QPainterPath UBEditableGraphicsPolygonItem::painterPath() const
{
    return path();
}
