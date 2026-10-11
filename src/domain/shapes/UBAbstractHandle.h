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

#include <QGraphicsItem>

#include "domain/UBItem.h"

class UBAbstractEditable;
class QPainter;

enum HandleId
{
    Free,
    Horizontal,
    Vertical,
    Diagonal,
    Stretch,
    Other
};

class UBAbstractHandle : public QGraphicsItem, public UBItem
{
public:
    UBAbstractHandle(UBAbstractHandle* const src);
    virtual ~UBAbstractHandle() = default;

    void setId(int id)
    {
        mId = id;
    }

    int getId()
    {
        return mId;
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QRectF boundingRect() const override;

    void setEditableObject(UBAbstractEditable *eo)
    {
        mEditableObject = eo;
    }

    UBAbstractEditable *editableObject()
    {
        return mEditableObject;
    }

    int radius() const
    {
        return this->mRadius;
    }

    virtual std::shared_ptr<UBGraphicsScene> scene() override;

protected:
    UBAbstractHandle();

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

protected:
    int mId;
    bool mClick;
    int mRadius;

    UBAbstractEditable *mEditableObject;
};
