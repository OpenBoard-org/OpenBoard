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


#ifndef UBGRAPHICSPIXMAPITEM_H_
#define UBGRAPHICSPIXMAPITEM_H_

#include <QtGui>

#include "core/UB.h"

#include "UBItem.h"

class UBGraphicsItemDelegate;

class UBGraphicsPixmapItem : public QObject, public QGraphicsPixmapItem, public UBItem, public UBGraphicsItem
{
    Q_OBJECT

    public:
        UBGraphicsPixmapItem(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsPixmapItem();

        enum { Type = UBGraphicsItemType::PixmapItemType };

        virtual int type() const
        {
            return Type;
        }
        virtual UBItem* deepCopy() const;

        virtual void copyItemParameters(UBItem *copy) const;

        virtual UBGraphicsScene* scene();

        Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

        void setOpacity(qreal op);
        qreal opacity() const;

        virtual void clearSource(){;}

        virtual void setUuid(const QUuid &pUuid);

protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif /* UBGRAPHICSPIXMAPITEM_H_ */
