
#include <QGraphicsPolygonItem>
#include <QPolygonF>

#include "tools/UBGraphicsTriangle.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

const QRect UBGraphicsTriangle::sDefaultRect =  QRect(0, 0, 800, 400);
const UBGraphicsTriangle::UBGraphicsTriangleOrientation UBGraphicsTriangle::sDefaultOrientation = 
UBGraphicsTriangle::BottomLeft;

UBGraphicsTriangle::UBGraphicsTriangle()
	:QGraphicsPolygonItem()
{
	setRect(sDefaultRect, sDefaultOrientation);

	create(*this);

    //updateResizeCursor();
}

UBGraphicsTriangle::~UBGraphicsTriangle()
{
}

UBItem* UBGraphicsTriangle::deepCopy(void) const
{
	UBGraphicsTriangle* copy = new UBGraphicsTriangle();

    copy->setPos(this->pos());
	copy->setPolygon(this->polygon());
    copy->setZValue(this->zValue());
    copy->setTransform(this->transform());

    // TODO UB 4.7 ... complete all members ?

    return copy;

}

void UBGraphicsTriangle::setRect(qreal x, qreal y, qreal w, qreal h, UBGraphicsTriangleOrientation orientation)
{
	mRect.setCoords(x, y, x+w, y+h);
	mOrientation = orientation;

	QPolygonF polygon;
	polygon << QPointF(x, y) << QPoint(x, y + h) << QPoint(x+w, y + h) << QPoint(x, y);

	QTransform t;

	switch(orientation)
	{
	case BottomLeft:
		t.setMatrix(1, 0, 0, 0, 1, 0, 0, 0, 1);
		break;
	case BottomRight:
		t.setMatrix(-1, 0, 0, 0, 1, 0, x, 0, 1);
		break;
	case TopLeft:
		t.setMatrix(1, 0, 0, 0, -1, 0, 0, y, 1);
		break;
	case TopRight:
		t.setMatrix(-1, 0, 0, 0, -1, 0, x, y, 1);
		break;
	}


	/*
	switch(orientation)
	{
	case BottomLeft:
		polygon << QPointF(x, y) << QPoint(x, y + h) << QPoint(x+w, y + h) << QPoint(x, y);
		break;
	case BottomRight:
		polygon << QPointF(x, y + h) << QPoint(x + w, y + y) << QPoint(x + w, y) << QPoint(x, y + h);
		break;
	case TopLeft:
		polygon << QPointF(x, y) << QPoint(x, y + h) << QPoint(x + w, y) << QPoint(x, y);
		break;
	case TopRight:
		polygon << QPointF(x, y) << QPoint(x + w, y + h) << QPoint(x+w, y) << QPoint(x, y );
		break;
	}
	*/
	setPolygon(polygon);
	setTransform(t);
}

UBGraphicsScene* UBGraphicsTriangle::scene() const
{
	return static_cast<UBGraphicsScene*>(QGraphicsPolygonItem::scene());
}

void UBGraphicsTriangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget)
{
	QPointF A1(mRect.x(), mRect.y());
	QPointF B1(mRect.x(), mRect.y() + mRect.height());
	QPointF C1(mRect.x() + mRect.width(), mRect.y() + mRect.height());

	qreal d = 70;
	qreal C = sqrt(mRect.width() * mRect.width() + mRect.height() * mRect.height());
	qreal L = (C * d + mRect.width() * d)/ mRect.height();
	qreal K = (C * d + mRect.height() * d)/ mRect.width();

	qreal W1 = mRect.height() * d / C;
	qreal H1 = mRect.width() * d / C;

	QPointF A2(mRect.x() + d, mRect.y() + K);
	QPointF B2(mRect.x() + d, mRect.y() + mRect.height() - d);
	QPointF C2(mRect.x() + mRect.width() - L, mRect.y() + mRect.height() - d);

	QPoint CC(mRect.x() + mRect.width() - L + W1,
		mRect.y() + mRect.height() - d - H1);

	painter->setPen(Qt::NoPen);

	QPolygonF polygon;

	QLinearGradient gradient1(QPointF(A1.x(), 0), QPointF(A2.x(), 0));
	gradient1.setColorAt(0, edgeFillColor());
	gradient1.setColorAt(1, middleFillColor());
	painter->setBrush(gradient1);
	polygon << A1 << A2 << B2 << B1;
	painter->drawPolygon(polygon);
	polygon.clear();

	QLinearGradient gradient2(QPointF(0, B1.y()), QPointF(0, B2.y()));
	gradient2.setColorAt(0, edgeFillColor());
	gradient2.setColorAt(1, middleFillColor());
	painter->setBrush(gradient2);
	polygon << B1 << B2 << C2 << C1;
	painter->drawPolygon(polygon);
	polygon.clear();

	QLinearGradient gradient3(CC, C2);
	gradient3.setColorAt(0, edgeFillColor());
	gradient3.setColorAt(1, middleFillColor());
	painter->setBrush(gradient3);
	polygon << C1 << C2 << A2 << A1;
	painter->drawPolygon(polygon);
	polygon.clear();


	painter->setBrush(Qt::NoBrush);
	painter->setPen(drawColor());

	polygon << A1 << B1 << C1;
	painter->drawPolygon(polygon);
	polygon.clear();

	polygon << A2 << B2 << C2;
	painter->drawPolygon(polygon);

	paintGraduations(painter);
}

void UBGraphicsTriangle::paintGraduations(QPainter *painter)
{
    const int     centimeterGraduationHeight = 15;
    const int halfCentimeterGraduationHeight = 10;
    const int     millimeterGraduationHeight = 5;
    const int       millimetersPerCentimeter = 10;
    const int   millimetersPerHalfCentimeter = 5;

    painter->save();
    painter->setFont(font());
    QFontMetricsF fontMetrics(painter->font());
    for (int millimeters = 0; millimeters < (rect().width() - sLeftEdgeMargin - sRoundingRadius) / sPixelsPerMillimeter; millimeters++)
    {
        int graduationX = topLeftOrigin().x() + sPixelsPerMillimeter * millimeters;
        int graduationHeight = (0 == millimeters % millimetersPerCentimeter) ?
            centimeterGraduationHeight :
            ((0 == millimeters % millimetersPerHalfCentimeter) ?
                halfCentimeterGraduationHeight : millimeterGraduationHeight);

		// Check that grad. line inside triangle
		qreal lineY = rect().bottom() - rect().height()/rect().width()*(rect().width() - graduationX);
		if (lineY >= topLeftOrigin().y() + rect().height() - graduationHeight)
			break;
        
        painter->drawLine(QLine(graduationX, topLeftOrigin().y() + rect().height(), graduationX, topLeftOrigin().y() + rect().height() - graduationHeight));
        if (0 == millimeters % millimetersPerCentimeter)
        {
            QString text = QString("%1").arg((int)(millimeters / millimetersPerCentimeter));
			int textXRight = graduationX + fontMetrics.width(text) / 2;
			qreal textWidth = fontMetrics.width(text);
            qreal textHeight = fontMetrics.tightBoundingRect(text).height() + 5;
			int textY = rect().bottom() - 5 - centimeterGraduationHeight - textHeight;

			lineY = rect().bottom() - rect().height()/rect().width()*(rect().width() - textXRight);

			if (textXRight < rect().right()
				&& lineY < textY)
            {
                painter->drawText(
                    QRectF(graduationX - textWidth / 2, 
					textY, textWidth, textHeight),
                    Qt::AlignVCenter, text);
            }
        }
    }
    painter->restore();
}


void UBGraphicsTriangle::rotateAroundTopLeftOrigin(qreal angle)
{}

QPointF	UBGraphicsTriangle::topLeftOrigin() const
{
	return QPointF(mRect.x() + sLeftEdgeMargin , mRect.y());
}

QRectF	UBGraphicsTriangle::resizeButtonRect() const
{
	return QRectF(0,0,0,0);
}

QRectF	UBGraphicsTriangle::closeButtonRect() const
{
	return QRectF(0,0,0,0);
}

QRectF	UBGraphicsTriangle::rotateButtonRect() const
{
	return QRectF(0,0,0,0);
}
