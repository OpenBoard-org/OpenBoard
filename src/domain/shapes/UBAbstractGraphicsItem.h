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

#include "domain/UBItem.h"
#include "domain/UBItemStyle.h"
#include "domain/UBStyledItem.h"
#include "domain/shapes/UBShapeFactory.h"


#include <QAbstractGraphicsShapeItem>

class UBAbstractGraphicsItem
    : public UBItem
    , public UBGraphicsItem
    , public QAbstractGraphicsShapeItem
    , public UBStyledItem
{
public:
    UBAbstractGraphicsItem(QGraphicsItem* parent = nullptr);
    virtual ~UBAbstractGraphicsItem();

    virtual ShapeType shapeType() = 0;

    virtual void applyItemStyle(const UBItemStyle& style, bool isDark) override;
    virtual bool isShape() const override;
    virtual bool isMarker() const override;

    virtual bool hasFillingProperty() const;

    bool hasStrokeProperty() const;

    void setStyle(Qt::PenStyle penStyle);

    void setFillColor(const QColor& color);

    void setStrokeColor(const QColor& color);

    void setStrokeWidth(double width);

    // get the path of the shape in local coordinates
    virtual QPainterPath painterPath() const = 0;

    // get the path of the outline of the shape, including pen width and optional filling
    virtual QPainterPath shape() const override;

    void initializeStrokeProperty();
    void initializeFillingProperty();

    // disambiguation from UBGraphicsItem and QabstractGraphicsShapeItem
    virtual int type() const override = 0;

    // must be defined, because the delegate use it
    virtual QRectF boundingRect() const override
    {
        return QRect();
    }

    virtual void copyItemParameters(UBItem* copy) const override;

    virtual std::shared_ptr<UBGraphicsScene> scene() override;

protected:
    void setStyle(QPainter* painter);

    QRectF adjustBoundingRect(QRectF rect) const;

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual void addExtraShapes(QPainterPath& path) const = 0;
};
