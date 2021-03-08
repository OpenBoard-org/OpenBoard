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




#ifndef UBGRAPHICSAXES_H
#define UBGRAPHICSAXES_H


#include <QtGui>
#include <QtSvg>

#include "core/UB.h"
#include "domain/UBItem.h"

class UBGraphicsScene;

class UBGraphicsAxes : public QObject, public QGraphicsPolygonItem, public UBItem
{
    Q_OBJECT

    public:
        UBGraphicsAxes();
        virtual ~UBGraphicsAxes();

        enum { Type = UBGraphicsItemType::AxesItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;
        virtual void copyItemParameters(UBItem *copy) const;

        void setRect(qreal x, qreal y, qreal w, qreal h);
        QRectF bounds() const;
        void setShowNumbers(bool showNumbers);
        bool showNumbes() const;

        enum UBGraphicsAxesDirection
        {
                Left = 0,
                Right,
                Bottom,
                Top
        };

    protected:

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        void paintGraduations(QPainter *painter);
        void setRect(const QRectF &rect);

        // Events
        virtual void    mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void    hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void    hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        virtual void    hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    private:
        // Helpers
        void           updateResizeCursor();
        QCursor              resizeCursor() const;
        QCursor               closeCursor() const;
        QCursor                moveCursor() const;
        void                 selectCursor(QGraphicsSceneHoverEvent *event);

        virtual QRectF             resizeLeftRect() const;
        virtual QRectF            resizeRightRect() const;
        virtual QRectF           resizeBottomRect() const;
        virtual QRectF              resizeTopRect() const;
        virtual QRectF            closeButtonRect() const;
        virtual QRectF          numbersButtonRect() const;
        virtual QLineF                      xAxis() const;
        virtual QLineF                      yAxis() const;
        virtual UBGraphicsScene*            scene() const;

        QColor                  drawColor() const;
        QFont                        font() const;

        QGraphicsSvgItem* mCloseSvgItem;
        QGraphicsSvgItem* mNumbersSvgItem;

        bool mResizing;
        UBGraphicsAxesDirection mResizeDirection;
        bool mShowButtons;
        bool mShowNumbers;

        QCursor mResizeCursorH;
        QCursor mResizeCursorV;

        qreal mAntiScaleRatio;
        qreal mPixelsPerCentimeter;
        QRectF mBounds;

        // Constants
        static const QRect     sDefaultRect;

        static const int    sMinLength = 50;    // 1sm
        static const int    sMaxLength = 35000; // 700sm
        static const int  sArrowLength = 12;
        static const int   sArrowWidth = 5;
        static const int       sMargin = 5;
        static const int    sItemWidth = 30;
        static const int                  sDrawTransparency = 255;
        static const QColor       sLightBackgroundDrawColor;
        static const QColor        sDarkBackgroundDrawColor;
};

#endif // UBGRAPHICSAXES_H
