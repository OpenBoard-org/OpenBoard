
/*
 * UBGraphicsRuler.h
 *
 *  Created on: April 16, 2009
 *      Author: Jerome Marchaud
 */

#ifndef UBGRAPHICSTRIANGLE_H_
#define UBGRAPHICSTRIANGLE_H_

#include <QtGui>
#include <QtSvg>
#include <QGraphicsPolygonItem>

#include "core/UB.h"
#include "domain/UBItem.h"
#include "tools/UBAbstractDrawRuler.h"


class UBGraphicsScene;
class UBItem;

class UBGraphicsTriangle : public UBAbstractDrawRuler, public QGraphicsPolygonItem, public UBItem
{
    Q_OBJECT;

    public:
        UBGraphicsTriangle();
        virtual ~UBGraphicsTriangle();

		enum { Type = UBGraphicsItemType::TriangleItemType };

        virtual int type() const
        {
            return Type;
        }


		virtual UBItem* deepCopy(void) const;

		enum UBGraphicsTriangleOrientation
		{
				BottomLeft = 0,
				BottomRight,
				TopLeft,
				TopRight
		};

		static UBGraphicsTriangleOrientation orientationFromStr(QStringRef& str)
		{
			if (str == "BottomLeft") return BottomLeft;
			if (str == "BottomRight") return BottomRight;
			if (str == "TopLeft") return TopLeft;
			if (str == "TopRight") return TopRight;
			return sDefaultOrientation;
		}
		void setRect(const QRectF &rect, UBGraphicsTriangleOrientation orientation)
		{
			setRect(rect.x(), rect.y(), rect.width(), rect.height(), orientation);
		}
		void setRect(qreal x, qreal y, qreal w, qreal h, UBGraphicsTriangleOrientation orientation);
		void setOrientation(UBGraphicsTriangleOrientation orientation);
		QRectF rect() const {return boundingRect();}

		UBGraphicsScene* scene() const;

	protected:

        void updateResizeCursor();

		virtual void paint (QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);

		virtual void rotateAroundCenter(qreal angle);

		virtual QPointF	rotationCenter() const;

        virtual QRectF	closeButtonRect() const;
		QPolygonF resize1Polygon() const;
		QPolygonF resize2Polygon() const;
		QRectF	hFlipRect() const;
		QRectF	vFlipRect() const;
		QRectF	rotateRect() const;

		QCursor	resizeCursor1() const;
        QCursor	resizeCursor2() const;

		QCursor	flipCursor() const;

        virtual void	mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void    mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void	mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void	hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void	hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
		virtual void    hoverMoveEvent(QGraphicsSceneHoverEvent *event);

	private:

        QCursor mResizeCursor1;
        QCursor mResizeCursor2;

        QTransform calculateRotationTransform();
        qreal angle;
        void rotateAroundCenter(QTransform& transform, QPointF center);

		bool mResizing1;
		bool mResizing2;
		bool mRotating;

		QGraphicsSvgItem* mHFlipSvgItem;
		QGraphicsSvgItem* mVFlipSvgItem;
		QGraphicsSvgItem* mRotateSvgItem;

		static const QRect sDefaultRect;
		static const UBGraphicsTriangleOrientation sDefaultOrientation;

		void paintGraduations(QPainter *painter);


		UBGraphicsTriangleOrientation mOrientation;

        QPointF A1, B1, C1, A2, B2, C2; // coordinates of points in ext and int triangles
        qreal C;
        qreal W1, H1; // Neccessary for filling
        QPointF CC; // Hyp. fillining gradient - top point
        void calculatePoints(const QRectF& rect);

		static const int d = 70; // width of triangle border
		static const int sArrowLength = 30;
		static const int sMinWidth = 380;
		static const int sMinHeight = 200;
};

#endif /* UBGRAPHICSTRIANGLE_H_ */
