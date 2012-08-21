/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UBGRAPHICSARISTO_H_
#define UBGRAPHICSARISTO_H_

#include <QtGui>
#include <QtSvg>
#include <QGraphicsPolygonItem>

#include "core/UB.h"
#include "domain/UBItem.h"
#include "tools/UBAbstractDrawRuler.h"


class UBGraphicsScene;
class UBItem;

class UBGraphicsAristo : public UBAbstractDrawRuler, public QGraphicsPolygonItem, public UBItem
{
    Q_OBJECT

    public:
        UBGraphicsAristo();
        virtual ~UBGraphicsAristo();

        enum { Type = UBGraphicsItemType::AristoItemType };
        enum Tool {None, Move, Resize, Rotate, Close, MoveMarker, HorizontalFlip};

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy(void) const;
        virtual void copyItemParameters(UBItem *copy) const;

        virtual void StartLine(const QPointF& scenePos, qreal width);
        virtual void DrawLine(const QPointF& position, qreal width);
        virtual void EndLine();

        enum UBGraphicsAristoOrientation
        {
                Bottom = 0,
                Top
        };

        static UBGraphicsAristoOrientation orientationFromStr(QStringRef& str)
        {
            if (str == "Bottom") return Bottom;
            if (str == "Top") return Top;
            return sDefaultOrientation;
        }
        static QString orientationToStr(UBGraphicsAristoOrientation orientation)
        {
            QString result;
            if (orientation == 0) result = "Bottom";
            else if (orientation == 1) result = "Top";

            return result;
        }

        void setRect(const QRectF &rect, UBGraphicsAristoOrientation orientation)
        {
            setRect(rect.x(), rect.y(), rect.width(), rect.height(), orientation);
        }

        void setRect(qreal x, qreal y, qreal w, qreal h, UBGraphicsAristoOrientation orientation);

        void setOrientation(UBGraphicsAristoOrientation orientation);

        UBGraphicsAristoOrientation getOrientation() const {return mOrientation;}

        QRectF rect() const {return boundingRect();}

        UBGraphicsScene* scene() const;

    protected:

        virtual void paint (QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);
        virtual QPainterPath shape() const;

        virtual void rotateAroundCenter(qreal angle);
        virtual void resize(qreal factor);

        virtual QPointF    rotationCenter() const;

        virtual QRectF    closeButtonRect() const;
        QRectF  resizeButtonRect () const;
        QRectF    hFlipRect() const;
        QRectF    rotateRect() const;
        QRectF markerButtonRect() const;

        QCursor    flipCursor() const;
        QCursor    resizeCursor() const;
        QCursor markerCursor() const;

        virtual void    mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void    hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void    hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void    hoverMoveEvent(QGraphicsSceneHoverEvent *event);

    private:
        UBGraphicsAristo::Tool toolFromPos(QPointF pos);
        QTransform calculateRotationTransform();
        qreal angle;
        
        void rotateAroundCenter(QTransform& transform, QPointF center);

        bool mResizing;
        bool mRotating;
        bool mMarking;
        QRect lastRect;
        qreal mSpan;

        // Coordinates are transformed....
        QPoint lastPos;
        QGraphicsSvgItem* mHFlipSvgItem;
        QGraphicsSvgItem* mRotateSvgItem;
        QGraphicsSvgItem* mResizeSvgItem;
        QGraphicsSvgItem* mMarkerSvgItem;
        qreal   mStartAngle;
        qreal mCurrentAngle;

        static const QRect sDefaultRect;
        static const UBGraphicsAristoOrientation sDefaultOrientation;

        void paintGraduations(QPainter *painter);
        void paintRulerGraduations(QPainter *painter);
        void paintProtractorGraduations(QPainter* painter);
        void paintMarker(QPainter *painter);
        inline qreal radius () const
        {
            return sqrt(((B.x() - A.x())*(B.x() - A.x()))+((B.y() - A.y())*(B.y() - A.y()))) * 9 / 16 - 20;
        }


        UBGraphicsAristoOrientation mOrientation;

        void calculatePoints(const QRectF& rect);

        QPointF A, B, C;

        static const int d = 70; // width of triangle border
        static const int sArrowLength = 30;
        static const int sMinWidth = 380;
        static const int sMinHeight = 200;
        static const int sArcAngleMargin = 5;
};

#endif /* UBGRAPHICSARISTO_H_ */
