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

#include "UBAbstractEditableGraphicsShapeItem.h"

class UBEditableGraphicsRegularShapeItem : public UBAbstractEditableGraphicsShapeItem
{
public:
    UBEditableGraphicsRegularShapeItem(int nVertices, QPointF startPoint = QPointF(0, 0),
                                       QGraphicsItem* parent = nullptr);
    virtual ~UBEditableGraphicsRegularShapeItem();

    void createGraphicsRegularPathItem();
    virtual void addPoint(const QPointF& point);

    void updatePath(QPointF newPos);
    void setStartPoint(QPointF pos);

    // UBItem interface
    UBItem* deepCopy() const override;
    void copyItemParameters(UBItem* copy) const override;

    enum
    {
        Type = UBGraphicsItemType::GraphicsRegularPathItemType
    };

    virtual int type() const override
    {
        return Type;
    }

    virtual ShapeType shapeType() override;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    virtual QRectF boundingRect() const override;

    virtual QPainterPath painterPath() const override;

    inline const int nVertices() const
    {
        return mNVertices;
    }

    inline const QPointF& startPoint() const
    {
        return mStartPoint;
    }

    void updateHandle(UBAbstractHandle* handle) override;
    virtual void focusHandle(UBAbstractHandle* handle) override;

    QPointF circumscribedCenterCircle() const
    {
        return mCenter;
    }

    qreal circumscribedRadiusCircle() const
    {
        return mRadius;
    }

    void setCircumscribedCenterCircle(const QPointF& center)
    {
        mCenter = center;
    }

    void setCircumscribedRadiusCircle(qreal radius)
    {
        mRadius = radius;
    }

    QPointF correctStartPoint() const;

    void setPath(QPainterPath path)
    {
        mPath = path;
    }

    QPainterPath path() const
    {
        return mPath;
    }

    void onActivateEditionMode() override;

private:
    int mNVertices;
    int mNOriginalVertices{0};
    QList<QPair<double, double>> mVertices;
    QPointF mStartPoint;

    // for the circumscribed circle
    QPointF mCenter;
    qreal mRadius;

    QPainterPath mPath;
};
