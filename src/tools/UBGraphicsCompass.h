
/*
 * UBGraphicsCompass.h
 *
 *  Created on: April 28, 2009
 *      Author: Jerome Marchaud
 */

#ifndef UBGRAPHICSCOMPASS_H_
#define UBGRAPHICSCOMPASS_H_

#include <QtGui>
#include <QtSvg>

#include "core/UB.h"
#include "domain/UBItem.h"

class UBGraphicsScene;

class UBGraphicsCompass: public QObject, public QGraphicsRectItem, public UBItem
{
    Q_OBJECT;

    public:
        UBGraphicsCompass();
        virtual ~UBGraphicsCompass();

        enum { Type = UBGraphicsItemType::CompassItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

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

    private slots:
        void penColorChanged();
        void lineWidthChanged();

    private:
        // Helpers
        void            paintAngleDisplay(QPainter *painter);
        void           paintRadiusDisplay(QPainter *painter);
        void           rotateAroundNeedle(qreal angle);
        void                      drawArc();
        void           updateResizeCursor();
        void             updateDrawCursor();
        void             paintCenterCross();

        QCursor                moveCursor() const;
        QCursor              resizeCursor() const;
        QCursor              rotateCursor() const;
        QCursor               closeCursor() const;
        QCursor                drawCursor() const;
        QRectF                  hingeRect() const;
        QPointF            needlePosition() const;
        QPointF            pencilPosition() const;
        QRectF            closeButtonRect() const;
        QRectF           resizeButtonRect() const;
        virtual QPainterPath        shape() const;
        QPainterPath          needleShape() const;
        QPainterPath          pencilShape() const;
        QPainterPath      needleBaseShape() const;
        QPainterPath       needleArmShape() const;
        QPainterPath           hingeShape() const;
        QPainterPath      pencilBaseShape() const;
        QPainterPath       pencilArmShape() const;
        UBGraphicsScene*            scene() const;
        QColor                  drawColor() const;
        QColor            middleFillColor() const;
        QColor              edgeFillColor() const;
        QFont                        font() const;
        qreal              angleInDegrees() const;

        // Members
        QCursor mOuterCursor;
        QCursor mResizeCursor;
        QCursor mDrawCursor;
        bool mResizing;
        bool mRotating;
        bool mDrawing;
        bool mShowButtons;
        qreal mSpanAngleInDegrees;
        QPointF mSceneArcStartPoint;
        bool mDrewCircle;
        QGraphicsSvgItem* mCloseSvgItem;
        QGraphicsSvgItem* mResizeSvgItem;
        qreal mAntiScaleRatio;
        bool mDrewCenterCross;

        // Constants
        static const QRect                     sDefaultRect;
        static const int                      sNeedleLength = 24;
        static const int                  sNeedleBaseLength = 16;
        static const int                      sPencilLength = 16;
        static const int                  sPencilBaseLength = 24;
        static const int                         sMinRadius;
        static const int               sDegreeToQtAngleUnit = 16;
        static const int                  sFillTransparency = 127;
        static const int                  sDrawTransparency = 192;
        static const QColor   sLightBackgroundEdgeFillColor;
        static const QColor sLightBackgroundMiddleFillColor;
        static const QColor       sLightBackgroundDrawColor;
        static const QColor    sDarkBackgroundEdgeFillColor;
        static const QColor  sDarkBackgroundMiddleFillColor;
        static const QColor        sDarkBackgroundDrawColor;
        static const int               sPixelsPerMillimeter = 5;
        static const int sDisplayRadiusOnPencilArmMinLength = 300;
        static const int        sDisplayRadiusUnitMinLength = 250;
};

#endif /* UBGRAPHICSCOMPASS_H_ */
