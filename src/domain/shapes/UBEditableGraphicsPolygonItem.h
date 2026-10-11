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

#include "UBAbstractEditableGraphicsPathItem.h"

class UBEditableGraphicsPolygonItem : public UBAbstractEditableGraphicsPathItem
{
public:
    UBEditableGraphicsPolygonItem(QGraphicsItem* parent = nullptr);
    ~UBEditableGraphicsPolygonItem();

    virtual void addPoint(const QPointF& point) override;

    inline bool isClosed() const
    {
        return mClosed;
    }

    inline void setClosed(bool closed);

    inline bool isOpened() const
    {
        return mOpened;
    }

    void setOpened(bool opened);

    void setIsInCreationMode(bool mode);

    // UBItem interface
    UBItem* deepCopy() const override;
    void copyItemParameters(UBItem* copy) const override;

    // QGraphicsItem interface
    enum
    {
        Type = UBGraphicsItemType::GraphicsPathItemType
    };

    virtual int type() const override
    {
        return Type;
    }

    virtual ShapeType shapeType() override;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    virtual QRectF boundingRect() const override;
    virtual QPainterPath painterPath() const override;

    virtual void updateHandle(UBAbstractHandle* handle) override;
    virtual bool hasFillingProperty() const override;

private:
    bool mClosed{false};
    bool mOpened{false};
    bool mIsInCreationMode{true};

    QPointF mStartEndPoint[2];

    int HANDLE_SIZE{20}; // in pixel
};
