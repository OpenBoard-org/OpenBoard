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




#ifndef UBGRAPHICSGROUPCONTAINERITEM_H
#define UBGRAPHICSGROUPCONTAINERITEM_H

#include <QGraphicsItem>

#include "domain/UBItem.h"
#include "frameworks/UBCoreGraphicsScene.h"

class UBGraphicsGroupContainerItem : public QGraphicsItem, public UBItem, public UBGraphicsItem
{

public:
    UBGraphicsGroupContainerItem (QGraphicsItem *parent = 0);
    virtual ~UBGraphicsGroupContainerItem();

    void addToGroup(QGraphicsItem *item);
    void removeFromGroup(QGraphicsItem *item);
    void setCurrentItem(QGraphicsItem *item){mCurrentItem = item;}
    QGraphicsItem *getCurrentItem() const {return mCurrentItem;}
    void deselectCurrentItem();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual UBCoreGraphicsScene *corescene();
    UBGraphicsGroupContainerItem *deepCopyNoChildDuplication() const;
    virtual UBGraphicsGroupContainerItem *deepCopy() const;
    virtual void copyItemParameters(UBItem *copy) const;

    enum { Type = UBGraphicsItemType::groupContainerType };

    virtual int type() const
    {
        return Type;
    }

    virtual void setUuid(const QUuid &pUuid);
    void destroy(bool canUndo = true);

    virtual void clearSource();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void pRemoveFromGroup(QGraphicsItem *item);

private:

    QRectF itemsBoundingRect;
    QGraphicsItem *mCurrentItem;

};

#endif // UBGRAPHICSGROUPCONTAINERITEM_H
