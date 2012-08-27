/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBGRAPHICSGROUPCONTAINERITEM_H
#define UBGRAPHICSGROUPCONTAINERITEM_H

#include <QGraphicsItem>

#include "domain/UBItem.h"

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

    virtual UBGraphicsItemDelegate* Delegate() const { return mDelegate;}

    virtual UBGraphicsScene* scene();
    virtual UBGraphicsGroupContainerItem *deepCopy() const;
    virtual void copyItemParameters(UBItem *copy) const;

    virtual void remove();
    enum { Type = UBGraphicsItemType::groupContainerType };

    virtual int type() const
    {
        return Type;
    }

    virtual void setUuid(const QUuid &pUuid);
    void destroy();

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
