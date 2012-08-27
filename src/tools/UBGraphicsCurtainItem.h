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


#ifndef UBGRAPHICSCURTAINITEM_H_
#define UBGRAPHICSCURTAINITEM_H_

#include <QtGui>

#include "core/UB.h"

#include "domain/UBItem.h"

class UBGraphicsItemDelegate;


class UBGraphicsCurtainItem : public QObject, public QGraphicsRectItem, public UBItem, public UBGraphicsItem
{

    Q_OBJECT

    public:
        UBGraphicsCurtainItem(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsCurtainItem();

        enum { Type = UBGraphicsItemType::CurtainItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;
        virtual void copyItemParameters(UBItem *copy) const;

        virtual void remove();

        //TODO UB 4.x not nice ...
        void triggerRemovedSignal();
        virtual UBGraphicsItemDelegate* Delegate() const {return mDelegate;}
        virtual void clearSource(){;}

        virtual void setUuid(const QUuid &pUuid);

     signals:

        void removed();

     protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        QColor  drawColor() const;
        QColor  opaqueControlColor() const;

//        UBGraphicsItemDelegate* mDelegate;

        static const QColor sDrawColor;
        static const QColor sDarkBackgroundDrawColor;
        static const QColor sOpaqueControlColor;
        static const QColor sDarkBackgroundOpaqueControlColor;
};

#endif /* UBGRAPHICSCURTAINITEM_H_ */
