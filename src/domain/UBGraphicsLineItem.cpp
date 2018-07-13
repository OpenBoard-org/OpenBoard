/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBGraphicsLineItem.h"
#include "customWidgets/UBGraphicsItemAction.h"
#include "UBAbstractGraphicsItem.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"
#include "UBFreeHandle.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

#include <cmath>

UBEditableGraphicsLineItem::UBEditableGraphicsLineItem(QGraphicsItem* parent)
    : UBEditableGraphicsPolygonItem(parent)
{
    // Line has Stroke and Fill capabilities :
    /*Delegate()->setCanReturnInCreationMode(false);*/
    initializeStrokeProperty();
    initializeFillingProperty();

    UBFreeHandle *startHandle = new UBFreeHandle;
    UBFreeHandle *endHandle = new UBFreeHandle;

    endHandle->setId(1);

    startHandle->setParentItem(this);
    endHandle->setParentItem(this);

    startHandle->setEditableObject(this);
    endHandle->setEditableObject(this);

    startHandle->hide();
    endHandle->hide();

    mHandles.push_back(startHandle);
    mHandles.push_back(endHandle);

    mIsMagnetic = true;
}

UBEditableGraphicsLineItem::~UBEditableGraphicsLineItem()
{

}

UBItem *UBEditableGraphicsLineItem::deepCopy() const
{
    UBEditableGraphicsLineItem* copy = new UBEditableGraphicsLineItem();

    copyItemParameters(copy);

    return copy;
}

void UBEditableGraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    painter->drawPath(path());

    drawArrows();
}

QPointF UBEditableGraphicsLineItem::startPoint() const
{
    return path().elementAt(0);
}

QPointF UBEditableGraphicsLineItem::endPoint() const
{
    if(path().elementCount() == 2)
        return path().elementAt(1);
    else
        return path().elementAt(0);
}

void UBEditableGraphicsLineItem::copyItemParameters(UBItem *copy) const
{
    UBAbstractEditableGraphicsPathItem::copyItemParameters(copy);
}


void UBEditableGraphicsLineItem::setStartPoint(QPointF pos)
{
    prepareGeometryChange();

    if(mIsMagnetic){
        forcePointPosition(pos, Start);
    }else{
        QPainterPath p;

        p.moveTo(this->pos());

        if(path().elementCount() == 2)
            p.lineTo(path().elementAt(1));

        setPath(p);

        this->setPos(pos);

        mHandles.at(0)->setPos(pos);
    }
}

void UBEditableGraphicsLineItem::setEndPoint(QPointF pos)
{
    prepareGeometryChange();

    if(mIsMagnetic){
        forcePointPosition(pos, End);
    }else{
        QPainterPath p;

        p.moveTo(path().elementAt(0));

        p.lineTo(pos);

        setPath(p);

        mHandles.at(1)->setPos(pos);
    }
}

void UBEditableGraphicsLineItem::updateHandle(UBAbstractHandle *handle)
{
    prepareGeometryChange();

    if(handle->getId() == 0){
        if(mIsMagnetic){
            forcePointPosition(handle->pos(), Start);
        }else{
            QPainterPath p;

            p.moveTo(handle->pos());

            p.lineTo(path().elementAt(1));

            setPath(p);
        }
    }else if(handle->getId() == 1){
        if(mIsMagnetic){
            forcePointPosition(handle->pos(), End);
        }else{
            QPainterPath p;

            p.moveTo(path().elementAt(0));

            p.lineTo(handle->pos());

            setPath(p);
        }
    }
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

QPainterPath UBEditableGraphicsLineItem::shape() const
{
    QPainterPath p;

    if(mMultiClickState >= 1 || isSelected()){
        p.addRect(boundingRect());
    }else{
        p = path();
    }

    return p;
}

void UBEditableGraphicsLineItem::addPoint(const QPointF &point)
{
    prepareGeometryChange();

    QPainterPath p(mapFromScene(point));

    if(path().elementCount() == 0){
        mHandles.at(0)->setPos(point);
        p.moveTo(point);
    }else{
        //In the other cases we have just to change the last point
        p.moveTo(path().elementAt(0));
        p.lineTo(point);

        mHandles.at(1)->setPos(point);
    }

    setPath(p);
}

void UBEditableGraphicsLineItem::setMagnetic(bool isMagnetic)
{
    mIsMagnetic = isMagnetic;
}

bool UBEditableGraphicsLineItem::isMagnetic() const
{
    return mIsMagnetic;
}

void UBEditableGraphicsLineItem::forcePointPosition(const QPointF& pos, PointPosition pointPosition, int amplitude)
{
    QLineF line;

    int angles[] = {0, 45, 90, 135, 180, 225, 270, 315};

    int size = sizeof(angles) / sizeof(int);

    if(pointPosition == Start){
        line.setP1(pos);
        line.setP2(path().elementAt(1));
    }else{
        line.setP1(path().elementAt(0));
        line.setP2(pos);
    }

    int angle = line.angle();

    const float PI_2 = 4*atan(1.f)*2;

    //for each angle we compute the left and right angle
    //then compute the distance between both
    for(int i = 0; i < size; i++){
        //use the modulo operator to force the angle to stay in [0, 360]
        int leftAmplitude = (angles[i] + amplitude) % 360;
        int rightAmplitude = (angles[i] - amplitude + 360) % 360;

        int leftDist = (leftAmplitude - angle + 360) % 360;
        int rightDist = (angle - rightAmplitude + 360) % 360;

        if(leftDist <= amplitude || rightDist <= amplitude){
            if(pointPosition == End){
                line.setAngle(angles[i]);
            }else{
                //compute the position of p1 by hand
                float angleInRadians = angles[i]*PI_2/360;

                qreal l = line.length();

                const qreal dx = -cos(angleInRadians)*l;
                const qreal dy = sin(angleInRadians)*l;

                line.setP1(QPointF(dx + line.p2().x(), dy + line.p2().y()));
            }
            break;
        }
    }

    QPainterPath p;

    p.moveTo(line.p1());
    p.lineTo(line.p2());

    setPath(p);

    mHandles.at(0)->setPos(line.p1().x(), line.p1().y());
    mHandles.at(1)->setPos(line.p2().x(), line.p2().y());
}
