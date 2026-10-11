/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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




#ifndef UBGRAPHICSSTROKESGROUP_H
#define UBGRAPHICSSTROKESGROUP_H

#include <QGraphicsItemGroup>
#include <QGraphicsSceneMouseEvent>

#include "core/UB.h"
#include "UBItem.h"
#include "domain/UBStyledItem.h"

class UBGraphicsStrokesGroup : public QGraphicsItemGroup, public UBItem, public UBGraphicsItem, public UBStyledItem
{
public:
    UBGraphicsStrokesGroup(QGraphicsItem* parent = 0);
    ~UBGraphicsStrokesGroup();
    virtual UBItem* deepCopy() const override;
    virtual void copyItemParameters(UBItem *copy) const override;
    enum { Type = UBGraphicsItemType::StrokeItemType };
    virtual int type() const override
    {
        return Type;
    }

    virtual UBItemStyle itemStyle() const override;
    virtual void applyItemStyle(const UBItemStyle& style, bool isDark) override;
    virtual bool isShape() const override;
    virtual bool isMarker() const override;

protected:

    virtual QPainterPath shape () const override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    // Graphical display of stroke Z-level
    bool debugTextEnabled;
    QGraphicsSimpleTextItem * mDebugText;
};

#endif // UBGRAPHICSSTROKESGROUP_H
