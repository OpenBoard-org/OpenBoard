
/*
 * UBGraphicsRuler.h
 *
 *  Created on: April 16, 2009
 *      Author: Jerome Marchaud
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

		virtual void StartLine(const QPointF& position, qreal width);
		virtual void DrawLine(const QPointF& position, qreal width);
		virtual void EndLine();

    signals:

        void hidden();

    protected:
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        // Events
        virtual void   mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void   hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void   hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void    hoverMoveEvent(QGraphicsSceneHoverEvent *event);

    private:
        // Helpers
        void               fillBackground(QPainter *painter);
        void             paintGraduations(QPainter *painter);
        void          paintRotationCenter(QPainter *painter);
        void    rotateAroundTopLeftOrigin(qreal angle);
        void           updateResizeCursor();

        QPointF             topLeftOrigin() const;
        QCursor                moveCursor() const;
        QCursor              resizeCursor() const;
        QCursor              rotateCursor() const;
        QCursor               closeCursor() const;
		QCursor				drawRulerLineCursor() const;
        QRectF           resizeButtonRect() const;
        QRectF            closeButtonRect() const;
        QRectF           rotateButtonRect() const;
        UBGraphicsScene*            scene() const;
        QColor                  drawColor() const;
        QColor            middleFillColor() const;
        QColor              edgeFillColor() const;
        QFont                        font() const;

		int drawLineDirection;

        // Members
        bool mResizing;
        bool mRotating;
        bool mShowButtons;
        QGraphicsSvgItem* mCloseSvgItem;
        QGraphicsSvgItem* mRotateSvgItem;
        QGraphicsSvgItem* mResizeSvgItem;
        QCursor mResizeCursor;
        qreal mAntiScaleRatio;

		QPointF startDrawPosition;

        // Constants
        static const QRect               sDefaultRect;
        static const int              sLeftEdgeMargin = 10;
        static const int                   sMinLength = 150;
        static const int         sDegreeToQtAngleUnit = 16;
        static const int              sRotationRadius = 15;
        static const int         sPixelsPerMillimeter = 5;
        static const int            sFillTransparency = 127;
        static const int            sDrawTransparency = 192;
        static const int              sRoundingRadius = sLeftEdgeMargin / 2;
        static const QColor   sLightBackgroundEdgeFillColor;
        static const QColor sLightBackgroundMiddleFillColor;
        static const QColor       sLightBackgroundDrawColor;
        static const QColor    sDarkBackgroundEdgeFillColor;
        static const QColor  sDarkBackgroundMiddleFillColor;
        static const QColor        sDarkBackgroundDrawColor;
};

#endif /* UBGRAPHICSRULER_H_ */
