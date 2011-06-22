
/*
 * UBGraphicsProtractor.h
 *
 *  Created on: April 23, 2009
 *      Author: Patrick LeclËre
 */

#ifndef UBGRAPHICSPROTRACTOR_H_
#define UBGRAPHICSPROTRACTOR_H_

#include <QtGui>
#include <QtSvg>

#include "core/UB.h"

#include "domain/UBItem.h"

class UBGraphicsScene;

class UBGraphicsProtractor : public QObject, public QGraphicsEllipseItem, public UBItem
{

    Q_OBJECT;

    public:
        UBGraphicsProtractor ();
		enum Tool {None, Move, Resize, Rotate, Reset, Close, MoveMarker};

        qreal angle () { return mStartAngle; }
        qreal markerAngle () { return mCurrentAngle; }
        void  setAngle (qreal angle) { mStartAngle = angle; setStartAngle(mStartAngle * 16); }
        void  setMarkerAngle (qreal angle) { mCurrentAngle = angle; }

        virtual UBItem* deepCopy() const;

        enum { Type = UBGraphicsItemType::ProtractorItemType };

        virtual int type() const
        {
            return Type;
        }

    signals:

        void hidden();

    protected:
        virtual void paint (QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        virtual void   mousePressEvent (QGraphicsSceneMouseEvent *event);
        virtual void    mouseMoveEvent (QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent *event);
        virtual void   hoverEnterEvent (QGraphicsSceneHoverEvent *event);
        virtual void   hoverLeaveEvent (QGraphicsSceneHoverEvent *event);
        virtual void    hoverMoveEvent (QGraphicsSceneHoverEvent *event);
        virtual QPainterPath shape() const;
        QRectF boundingRect() const;
    private:
        // Helpers
        void paintGraduations (QPainter *painter);
        void     paintButtons (QPainter *painter);
        void paintAngleMarker (QPainter *painter);
                Tool      toolFromPos (QPointF pos);
        qreal       antiScale () const;
        UBGraphicsScene*            scene() const;
        QColor                  drawColor() const;
        QBrush                  fillBrush() const;

        QSizeF buttonSizeReference () const{return QSizeF(radius() / 10, mCloseSvgItem->boundingRect().height() * radius()/(10 * mCloseSvgItem->boundingRect().width()));}
        QSizeF markerSizeReference () const{return QSizeF(radius() / 10, mMarkerSvgItem->boundingRect().height() * radius()/(10 * mMarkerSvgItem->boundingRect().width()));}
        QRectF	 resetButtonBounds () const;
        QRectF	 closeButtonBounds () const;
        QRectF	resizeButtonBounds () const;
        QRectF	rotateButtonBounds () const{return QRectF(buttonSizeReference().width() * 5.5, -buttonSizeReference().width() * 5, buttonSizeReference().width(), buttonSizeReference().width());}
        QRectF	markerButtonBounds () const{return QRectF(radius() + 3, -markerSizeReference().height() / 2 , markerSizeReference().width(), markerSizeReference().height());}
                inline qreal               radius () const{return rect().height() / 2 - 20;}

        // Members
        QPointF mPreviousMousePos;
        Tool    mCurrentTool;
        bool	mShowButtons;
        qreal   mCurrentAngle;
        qreal   mSpan;
        qreal   mStartAngle;
        qreal   mScaleFactor;

        QGraphicsSvgItem* mCloseSvgItem;
        QGraphicsSvgItem* mResetSvgItem;
        QGraphicsSvgItem* mResizeSvgItem;
        QGraphicsSvgItem* mRotateSvgItem;
        QGraphicsSvgItem* mMarkerSvgItem;

        static const int                 sFillTransparency;
        static const int                 sDrawTransparency;
        static const QRectF                   sDefaultRect;
        static const QColor                     sFillColor;
        static const QColor               sFillColorCenter;
        static const QColor                     sDrawColor;
        static const QColor       sDarkBackgroundFillColor;
        static const QColor sDarkBackgroundFillColorCenter;
        static const QColor       sDarkBackgroundDrawColor;
};

#endif /* UBGRAPHICSPROTRACTOR_H_ */
