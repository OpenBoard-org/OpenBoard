/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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

class UBGraphicsStrokesGroup : public QObject, public QGraphicsItemGroup, public UBItem, public UBGraphicsItem
{
    Q_OBJECT
public:
    enum colorType {
        currentColor = 0
        , colorOnLightBackground
        , colorOnDarkBackground
    };

    UBGraphicsStrokesGroup(QGraphicsItem* parent = 0);
    ~UBGraphicsStrokesGroup();
    virtual UBItem* deepCopy() const;
    virtual void copyItemParameters(UBItem *copy) const;
    enum { Type = UBGraphicsItemType::StrokeItemType };
    virtual int type() const
    {
        return Type;
    }
    virtual void setUuid(const QUuid &pUuid);
    void setColor(const QColor &color, colorType pColorType = currentColor);
    QColor color(colorType pColorType = currentColor) const;

protected:

    virtual QPainterPath shape () const;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    // Graphical display of stroke Z-level
    bool debugTextEnabled;
    QGraphicsSimpleTextItem * mDebugText;
};

#endif // UBGRAPHICSSTROKESGROUP_H
