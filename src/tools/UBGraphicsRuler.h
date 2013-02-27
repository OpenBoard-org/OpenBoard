/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBGRAPHICSRULER_H_
#define UBGRAPHICSRULER_H_

#include <QtGui>
#include <QtSvg>

#include "core/UB.h"
#include "domain/UBItem.h"
#include "tools/UBAbstractDrawRuler.h"

class UBGraphicsScene;

class UBGraphicsRuler : public UBAbstractDrawRuler, public QGraphicsRectItem, public UBItem
{
    Q_OBJECT;

    public:
        UBGraphicsRuler();
        virtual ~UBGraphicsRuler();

        enum { Type = UBGraphicsItemType::RulerItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;
        virtual void copyItemParameters(UBItem *copy) const;

        virtual void StartLine(const QPointF& position, qreal width);
        virtual void DrawLine(const QPointF& position, qreal width);
        virtual void EndLine();

    protected:
        
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        // Events
        virtual void    mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void    hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void    hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void    hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void paintGraduations(QPainter *painter);

    private:

        bool mResizing;
        bool mRotating;


        // Helpers
        void    fillBackground(QPainter *painter);
        void    paintRotationCenter(QPainter *painter);
        virtual void    rotateAroundCenter(qreal angle);

        QGraphicsSvgItem* mRotateSvgItem;
        QGraphicsSvgItem* mResizeSvgItem;

        void updateResizeCursor();
        QCursor resizeCursor() const{return mResizeCursor;}

        virtual QPointF             rotationCenter() const;
        virtual QRectF           resizeButtonRect() const;
        virtual QRectF            closeButtonRect() const;
        virtual QRectF           rotateButtonRect() const;
        virtual UBGraphicsScene*            scene() const;

        QCursor mResizeCursor;

        int drawLineDirection;

        // Constants
        static const QRect               sDefaultRect;

        static const int    sMinLength = 150;   // 3sm
        static const int    sMaxLength = 35000; // 700sm
};

#endif /* UBGRAPHICSRULER_H_ */
