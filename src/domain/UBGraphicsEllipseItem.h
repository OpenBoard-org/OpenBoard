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



#ifndef UBGRAPHICSELLIPSEITEM_H
#define UBGRAPHICSELLIPSEITEM_H

#include <QGraphicsEllipseItem>
#include "UBAbstractGraphicsItem.h"
#include "UB3HandlesEditable.h"

class UB3HEditableGraphicsEllipseItem : public UB3HEditablesGraphicsBasicShapeItem
{
public:
    UB3HEditableGraphicsEllipseItem(QGraphicsItem* parent = 0);
    virtual ~UB3HEditableGraphicsEllipseItem();

    enum { Type = UBGraphicsItemType::GraphicsShapeItemType };
    virtual int type() const { return Type; }

    virtual UBItem* deepCopy() const;

    virtual void copyItemParameters(UBItem *copy) const;

    QPointF center() const;

    // QGraphicsItem interface
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void updateHandle(UBAbstractHandle *handle);
    virtual QRectF boundingRect() const;
    QPainterPath shape() const;

    virtual void onActivateEditionMode();

    void setRadiusX(qreal radius);

    void setRadiusY(qreal radius);

    void setRect(QRectF rect);

    QRectF rect() const;

    qreal radiusX() const;

    qreal radiusY() const;

private:
    qreal mRadiusX;
    qreal mRadiusY;
};

#endif // UBGRAPHICSELLIPSEITEM_H
