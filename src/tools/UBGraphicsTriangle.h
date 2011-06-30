
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
		QRectF rect() const {return mRect;}

		UBGraphicsScene* scene() const;
	protected:
		virtual void paint (QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget);

		virtual void rotateAroundTopLeftOrigin(qreal angle);

		virtual QPointF	topLeftOrigin() const;
        virtual QRectF	resizeButtonRect() const;
        virtual QRectF	closeButtonRect() const;
        virtual QRectF	rotateButtonRect() const;


	private:

		static const QRect sDefaultRect;
		static const UBGraphicsTriangleOrientation sDefaultOrientation;

		void paintGraduations(QPainter *painter);


		QRectF mRect;
		UBGraphicsTriangleOrientation mOrientation;
};

#endif /* UBGRAPHICSTRIANGLE_H_ */
