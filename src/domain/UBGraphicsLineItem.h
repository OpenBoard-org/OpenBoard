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

#ifndef UBGRAPHICSLINEITEM_H
#define UBGRAPHICSLINEITEM_H

#include <QGraphicsLineItem>
#include "UBAbstractGraphicsItem.h"

#include "UBEditableGraphicsPolygonItem.h"

class UBEditableGraphicsLineItem : public UBEditableGraphicsPolygonItem
{
    public:
        UBEditableGraphicsLineItem(QGraphicsItem* parent = 0);
        virtual ~UBEditableGraphicsLineItem();

        enum { Type = UBGraphicsItemType::GraphicsShapeItemType };
        virtual int type() const { return Type; }

        virtual UBItem* deepCopy() const;

        QPointF startPoint() const;

        QPointF endPoint() const;

        void setStartPoint(QPointF pos);
        void setEndPoint(QPointF pos);

        // QGraphicsItem interface
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        void updateHandle(UBAbstractHandle *handle);

        void setLine(QPointF start, QPointF end);

        void onActivateEditionMode();

        QPainterPath shape() const;

        void copyItemParameters(UBItem *copy) const;

        virtual void addPoint(const QPointF &point);

        void setMagnetic(bool isMagnetic);

        bool isMagnetic() const;
private:

        bool mIsMagnetic;

        enum PointPosition{
            End,
            Start
        };

        void forcePointPosition(const QPointF& pos, PointPosition pointPosition, int amplitude = 1);
};

#endif // UBGRAPHICSLINEITEM_H
