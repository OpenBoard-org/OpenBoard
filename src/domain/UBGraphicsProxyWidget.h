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


#ifndef UBGRAPHICSPROXYWIDGET_H_
#define UBGRAPHICSPROXYWIDGET_H_

#include <QtGui>


#include "UBItem.h"
#include "UBResizableGraphicsItem.h"

class UBGraphicsItemDelegate;

class UBGraphicsProxyWidget: public QGraphicsProxyWidget, public UBItem, public UBResizableGraphicsItem, public UBGraphicsItem
{
    public:
        UBGraphicsProxyWidget(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsProxyWidget();

        virtual void resize(qreal w, qreal h);
        virtual void resize(const QSizeF & size);

        virtual QSizeF size() const;

        void setDelegate(UBGraphicsItemDelegate* pDelegate);

        virtual UBGraphicsScene* scene();

        virtual void remove();

        virtual UBGraphicsItemDelegate* Delegate() const { return mDelegate;}

        virtual void clearSource(){;}
        virtual void setUuid(const QUuid &pUuid);

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);


        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

};

#endif /* UBGRAPHICSPROXYWIDGET_H_ */
