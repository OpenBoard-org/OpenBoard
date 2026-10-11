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


#pragma once

#include "UBEditableGraphicsPolygonItem.h"

class UBEditableGraphicsLineItem : public UBEditableGraphicsPolygonItem
{
public:
    UBEditableGraphicsLineItem(QGraphicsItem* parent = nullptr);
    virtual ~UBEditableGraphicsLineItem();

    enum
    {
        Type = UBGraphicsItemType::GraphicsShapeItemType
    };

    virtual int type() const override
    {
        return Type;
    }

    virtual ShapeType shapeType() override;

    virtual UBItem* deepCopy() const override;

    QPointF startPoint() const;
    QPointF endPoint() const;

    void setStartPoint(QPointF pos);
    void setEndPoint(QPointF pos);

    // QGraphicsItem interface
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updateHandle(UBAbstractHandle* handle) override;

    void setLine(QPointF start, QPointF end);

    void onActivateEditionMode() override;

    QRectF boundingRect() const override;

    void copyItemParameters(UBItem* copy) const override;

    virtual void addPoint(const QPointF& point) override;
};
