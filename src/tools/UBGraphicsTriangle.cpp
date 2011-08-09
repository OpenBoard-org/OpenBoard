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
	: QGraphicsPolygonItem()
	, UBAbstractDrawRuler()
	, mResizing1(false)
	, mResizing2(false)
	, mRotating(false)
    , angle(0)

{
	setRect(sDefaultRect, sDefaultOrientation);

	create(*this);

	mHFlipSvgItem = new QGraphicsSvgItem(":/images/hflipTool.svg", this);
    mHFlipSvgItem->setVisible(false);
    mHFlipSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));


	mVFlipSvgItem = new QGraphicsSvgItem(":/images/vflipTool.svg", this);
    mVFlipSvgItem->setVisible(false);
    mVFlipSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

	mRotateSvgItem = new QGraphicsSvgItem(":/images/rotateTool.svg", this);
    mRotateSvgItem->setVisible(false);
    mRotateSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    updateResizeCursor();
}


void UBGraphicsTriangle::updateResizeCursor()
{
    QPixmap pix(":/images/cursors/resize.png");
    QTransform itemTransform = sceneTransform();
    QRectF itemRect = boundingRect();

    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QPointF bottomLeft = itemTransform.map(itemRect.bottomLeft());

    QLineF topLine(topLeft, topRight);
    QLineF leftLine(topLeft, bottomLeft);

    qreal angle = topLine.angle();
    QTransform tr1;
    tr1.rotate(- angle);
    mResizeCursor1  = QCursor(pix.transformed(tr1, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);

    angle = leftLine.angle();
    QTransform tr2;
    tr2.rotate(- angle);
    mResizeCursor2  = QCursor(pix.transformed(tr2, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
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
	QPolygonF polygon;
	polygon << QPointF(x, y) << QPoint(x, y + h) << QPoint(x+w, y + h);
	setPolygon(polygon);

	setOrientation(orientation);
}

void UBGraphicsTriangle::setOrientation(UBGraphicsTriangleOrientation orientation)
{
    mOrientation = orientation;
    calculatePoints(boundingRect());

    QPolygonF polygon;
    polygon << A1 << B1 << C1;
    setPolygon(polygon);
}

UBGraphicsScene* UBGraphicsTriangle::scene() const
{
	return static_cast<UBGraphicsScene*>(QGraphicsPolygonItem::scene());
}

void UBGraphicsTriangle::calculatePoints(const QRectF& r)
{
    switch(mOrientation)
    {
        case BottomLeft:
            A1.setX(r.left()); A1.setY(r.top());
            B1.setX(r.left()); B1.setY(r.bottom());
            C1.setX(r.right()); C1.setY(r.bottom());
            break;
        case TopLeft:
            A1.setX(r.left()); A1.setY(r.bottom());
            B1.setX(r.left()); B1.setY(r.top());
            C1.setX(r.right()); C1.setY(r.top());
            break;
        case TopRight:
            A1.setX(r.right()); A1.setY(r.bottom());
            B1.setX(r.right()); B1.setY(r.top());
            C1.setX(r.left()); C1.setY(r.top());
            break;
        case BottomRight:
            A1.setX(r.right()); A1.setY(r.top());
            B1.setX(r.right()); B1.setY(r.bottom());
            C1.setX(r.left()); C1.setY(r.bottom());
            break;
    }

	C = sqrt(rect().width() * rect().width() + rect().height() * rect().height());
	qreal L = (C * d + rect().width() * d)/ rect().height();
	qreal K = (C * d + rect().height() * d)/ rect().width();

    switch(mOrientation)
    {
        case BottomLeft:
            A2.setX(r.left() + d); A2.setY(r.top() + K);
            B2.setX(r.left() + d); B2.setY(r.bottom() - d);
            C2.setX(r.right() - L); C2.setY(r.bottom() - d);
            break;
        case TopLeft:
            A2.setX(r.left() + d); A2.setY(r.bottom() - K);
            B2.setX(r.left() + d); B2.setY(r.top() + d);
            C2.setX(r.right() - L); C2.setY(r.top() + d);
            break;
        case TopRight:
            A2.setX(r.right() - d); A2.setY(r.bottom() - K);
            B2.setX(r.right() - d); B2.setY(r.top() + d);
            C2.setX(r.left() + L); C2.setY(r.top() + d);
            break;
        case BottomRight:
            A2.setX(r.right() - d); A2.setY(r.top() + K);
            B2.setX(r.right() - d); B2.setY(r.bottom() - d);
            C2.setX(r.left() + L); C2.setY(r.bottom() - d);
            break;
    }
	W1 = rect().height() * d / C;
	H1 = rect().width() * d / C;

    switch(mOrientation)
    {
        case BottomLeft:
            CC.setX(r.right() - L + W1); CC.setY(r.bottom() - d - H1);
            break;
        case TopLeft:
            CC.setX(r.right() - L + W1); CC.setY(r.top() + d + H1);
            break;
        case TopRight:
            CC.setX(r.left() + L - W1); CC.setY(r.top() + d + H1);
            break;
        case BottomRight:
            CC.setX(r.left() + L - W1); CC.setY(r.top() - d - H1);
            break;
    }
}

void UBGraphicsTriangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

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

	mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
    QTransform antiScaleTransform;
    antiScaleTransform.scale(mAntiScaleRatio, mAntiScaleRatio);

    mCloseSvgItem->setTransform(antiScaleTransform);
	mHFlipSvgItem->setTransform(antiScaleTransform);
	mVFlipSvgItem->setTransform(antiScaleTransform);
	mRotateSvgItem->setTransform(antiScaleTransform);

    mCloseSvgItem->setPos(closeButtonRect().topLeft());
	mHFlipSvgItem->setPos(hFlipRect().topLeft());
	mVFlipSvgItem->setPos(vFlipRect().topLeft());
	mRotateSvgItem->setPos(rotateRect().topLeft());

	if (mShowButtons || mResizing1 || mResizing2)
	{
		painter->setBrush(QColor(0, 0, 0));
		if (mShowButtons || mResizing1)
			painter->drawPolygon(resize1Polygon());
		if (mShowButtons || mResizing2)
			painter->drawPolygon(resize2Polygon());
	}
}

void UBGraphicsTriangle::paintGraduations(QPainter *painter)
{
    const int     centimeterGraduationHeight = 15;
    const int halfCentimeterGraduationHeight = 10;
    const int     millimeterGraduationHeight = 5;
    const int       millimetersPerCentimeter = 10;
    const int   millimetersPerHalfCentimeter = 5;

    qreal kx = (mOrientation == TopLeft || mOrientation == BottomLeft) ? 1 : -1;
    qreal ky = (mOrientation == BottomLeft || mOrientation == BottomRight) ? 1 : -1;

    painter->save();
    painter->setFont(font());
    QFontMetricsF fontMetrics(painter->font());
    for (int millimeters = 0; millimeters < (rect().width() - sLeftEdgeMargin - sRoundingRadius) / sPixelsPerMillimeter; millimeters++)
    {
        int graduationX = rotationCenter().x() + kx * sPixelsPerMillimeter * millimeters;
        int graduationHeight = (0 == millimeters % millimetersPerCentimeter) ?
            centimeterGraduationHeight :
            ((0 == millimeters % millimetersPerHalfCentimeter) ?
                halfCentimeterGraduationHeight : millimeterGraduationHeight);

		// Check that grad. line inside triangle
        qreal dx = (kx > 0) ? rect().width() - graduationX : graduationX - rect().x();
        qreal lineY = rotationCenter().y() - ky * rect().height()/rect().width() * dx;
        if (mOrientation == BottomLeft || mOrientation == BottomRight)
        {
		    if (lineY >= rotationCenter().y() - ky * graduationHeight)
			    break;
        }
        else
        {
		    if (lineY <= rotationCenter().y() - ky * graduationHeight)
			    break;
        }
        
        painter->drawLine(QLine(graduationX, rotationCenter().y(), graduationX, rotationCenter().y() - ky * graduationHeight));
        if (0 == millimeters % millimetersPerCentimeter)
        {
            QString text = QString("%1").arg((int)(millimeters / millimetersPerCentimeter));
			int textXRight = graduationX + fontMetrics.width(text) / 2;
			qreal textWidth = fontMetrics.width(text);
            qreal textHeight = fontMetrics.tightBoundingRect(text).height() + 5;

			int textY = (ky > 0) ? rotationCenter().y() - 5 - centimeterGraduationHeight - textHeight
                : rotationCenter().y() + 5 + centimeterGraduationHeight;

            bool bText = false;
            switch(mOrientation)
            {
                case BottomLeft:
                    dx = rect().width() - textXRight;
                    lineY = rotationCenter().y() - rect().height()/rect().width() * dx;
                    bText = lineY <= textY;
                    break;
                case TopLeft:
                    dx = rect().width() - textXRight;
                    lineY = rotationCenter().y() + rect().height()/rect().width() * dx;
                    bText = lineY >= textY + textHeight;
                    break;
                case TopRight:
                    dx = textXRight - textWidth - rect().left();
                    lineY = rotationCenter().y() + rect().height()/rect().width() * dx;
                    bText = lineY >= textY + textHeight;
                    break;
                case BottomRight:
                    dx = textXRight - textWidth - rect().left();
                    lineY = rotationCenter().y() - rect().height()/rect().width() * dx;
                    bText = lineY <= textY;
                    break;
            }

            if (bText)
                painter->drawText(
                    QRectF(graduationX - textWidth / 2, 
				    textY, textWidth, textHeight),
                    Qt::AlignVCenter, text);

        }
    }
    painter->restore();
}


void UBGraphicsTriangle::rotateAroundCenter(qreal angle)
{
    qreal oldAngle = this->angle;
    this->angle = angle;
    QTransform transform;
    rotateAroundCenter(transform, rotationCenter());
    setTransform(transform, true);
    this->angle = oldAngle + angle; // We have to store absolute value for FLIP case
}

void UBGraphicsTriangle::rotateAroundCenter(QTransform& transform, QPointF center)
{
    transform.translate(center.x(), center.y());
    transform.rotate(angle);
    transform.translate(- center.x(), - center.y());
}


QPointF	UBGraphicsTriangle::rotationCenter() const
{
    switch(mOrientation)
    {
        case BottomLeft:
        case TopLeft:
            return B1 + QPointF(sLeftEdgeMargin, 0);
        case TopRight:
        case BottomRight:
            return B1 - QPointF(sLeftEdgeMargin, 0);
    }
    return QPointF(0, 0);
}

QRectF	UBGraphicsTriangle::closeButtonRect() const
{
    switch(mOrientation)
    {
        case BottomLeft:
            return QRectF(B2.x() - mCloseSvgItem->boundingRect().width() - 5, 
		      B2.y() - mCloseSvgItem->boundingRect().height() - 5, 
		      mCloseSvgItem->boundingRect().width(),
		      mCloseSvgItem->boundingRect().height());
        case TopLeft:
            return QRectF(B2.x() - mCloseSvgItem->boundingRect().width() - 5, 
		      B2.y() + 5, 
		      mCloseSvgItem->boundingRect().width(),
		      mCloseSvgItem->boundingRect().height());
        case TopRight:
            return QRectF(B2.x() + 5, 
		      B2.y() + 5, 
		      mCloseSvgItem->boundingRect().width(),
		      mCloseSvgItem->boundingRect().height());
        case BottomRight:
            return QRectF(B2.x() + 5, 
		      B2.y() - mCloseSvgItem->boundingRect().height() - 5, 
		      mCloseSvgItem->boundingRect().width(),
		      mCloseSvgItem->boundingRect().height());
    }
    return QRectF(0,0,0,0);
}

QPolygonF UBGraphicsTriangle::resize1Polygon() const
{
    qreal x1, y1;
    switch(mOrientation)
    {
        case BottomLeft:
            x1 = -1;
            y1 = -1;
            break;
        case TopLeft:
            x1 = -1;
            y1 = 1;
            break;
        case TopRight:
            x1 = 1;
            y1 = 1;
            break;
        case BottomRight:
            x1 = 1;
            y1 = -1;
            break;
    }
    QPointF P1(C1.x() + x1 * sArrowLength, C1.y());
    QPointF P2(C1.x() + x1 * sArrowLength * rect().width()/C, C1.y() + y1 * sArrowLength * rect().height() / C);
	QPolygonF p;
    p << C1 << P1 << P2;	
	return p;
}

QPolygonF UBGraphicsTriangle::resize2Polygon() const
{
    qreal x1, y1;
    switch(mOrientation)
    {
        case BottomLeft:
            x1 = 1;
            y1 = 1;
            break;
        case TopLeft:
            x1 = 1;
            y1 = -1;
            break;
        case TopRight:
            x1 = -1;
            y1 = -1;
            break;
        case BottomRight:
            x1 = -1;
            y1 = 1;
            break;
    }
    QPointF P1(A1.x(), A1.y() + y1 * sArrowLength);
    QPointF P2(A1.x() + x1 * sArrowLength * rect().width()/C, 
        A1.y() + y1 * sArrowLength * rect().height() / C);
	QPolygonF p;
    p << A1 << P1 << P2;	
	return p;
}

QRectF	UBGraphicsTriangle::hFlipRect() const
{
    qreal dy = mVFlipSvgItem->boundingRect().height() + mCloseSvgItem->boundingRect().height() + 10;
    switch(mOrientation)
    {
        case BottomLeft:
            return QRectF(B2.x() - mHFlipSvgItem->boundingRect().width() - 5, 
		      B2.y() - mHFlipSvgItem->boundingRect().height() - 5 - dy, 
              mHFlipSvgItem->boundingRect().width(),
		      mHFlipSvgItem->boundingRect().height());
        case TopLeft:
            return QRectF(B2.x() - mHFlipSvgItem->boundingRect().width() - 5, 
		      B2.y() + 5 + dy, 
		      mHFlipSvgItem->boundingRect().width(),
		      mHFlipSvgItem->boundingRect().height());
        case TopRight:
            return QRectF(B2.x() + 5, 
		      B2.y() + 5 + dy, 
		      mHFlipSvgItem->boundingRect().width(),
		      mHFlipSvgItem->boundingRect().height());
        case BottomRight:
            return QRectF(B2.x() + 5, 
		      B2.y() - mHFlipSvgItem->boundingRect().height() - 5 - dy, 
		      mHFlipSvgItem->boundingRect().width(),
		      mHFlipSvgItem->boundingRect().height());
    }
    return QRectF(0,0,0,0);
}

QRectF	UBGraphicsTriangle::vFlipRect() const
{
    qreal dy = mCloseSvgItem->boundingRect().height() + 5;
    switch(mOrientation)
    {
        case BottomLeft:
            return QRectF(B2.x() - mVFlipSvgItem->boundingRect().width() - 5, 
		      B2.y() - mVFlipSvgItem->boundingRect().height() - 5 - dy, 
              mVFlipSvgItem->boundingRect().width(),
		      mVFlipSvgItem->boundingRect().height());
        case TopLeft:
            return QRectF(B2.x() - mVFlipSvgItem->boundingRect().width() - 5, 
		      B2.y() + 5 + dy, 
		      mVFlipSvgItem->boundingRect().width(),
		      mVFlipSvgItem->boundingRect().height());
        case TopRight:
            return QRectF(B2.x() + 5, 
		      B2.y() + 5 + dy, 
		      mVFlipSvgItem->boundingRect().width(),
		      mVFlipSvgItem->boundingRect().height());
        case BottomRight:
            return QRectF(B2.x() + 5, 
		      B2.y() - mVFlipSvgItem->boundingRect().height() - 5 - dy, 
		      mVFlipSvgItem->boundingRect().width(),
		      mVFlipSvgItem->boundingRect().height());
    }
    return QRectF(0,0,0,0);
}

QRectF	UBGraphicsTriangle::rotateRect() const
{
    QPointF p(C2);
    switch(mOrientation)
    {
        case BottomLeft:
            p += QPointF(20, 5);
            break;
        case TopLeft:
            p += QPointF(20, -5 - mRotateSvgItem->boundingRect().height());
            break;
        case TopRight:
            p += QPointF(-20 - mRotateSvgItem->boundingRect().width(), -5 - mRotateSvgItem->boundingRect().height());
            break;
        case BottomRight:
            p += QPointF(-20 - mRotateSvgItem->boundingRect().width(), 5);
            break;
    }
	return QRectF(p, QSizeF(mRotateSvgItem->boundingRect().size()));
}

QCursor	UBGraphicsTriangle::resizeCursor1() const
{
    return mResizeCursor1;
}

QCursor	UBGraphicsTriangle::resizeCursor2() const
{
    return mResizeCursor2;
}

QCursor	UBGraphicsTriangle::flipCursor() const
{
	return Qt::ArrowCursor;
}


void UBGraphicsTriangle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (resize1Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
    {
        mResizing1 = true;
        event->accept();
    }
	else if (resize2Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
    {
		mResizing2 = true;
        event->accept();
    }
	else if(rotateRect().contains(event->pos()))
	{
		mRotating = true;
		event->accept();
	}
    else
    {
		QGraphicsItem::mousePressEvent(event);
    }
	mShowButtons = false;
	mCloseSvgItem->setVisible(false);
	mHFlipSvgItem->setVisible(false);
	mVFlipSvgItem->setVisible(false);
	mRotateSvgItem->setVisible(mRotating);
	update();
}

void UBGraphicsTriangle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mResizing1 && !mResizing2 && !mRotating)
    {
		QGraphicsItem::mouseMoveEvent(event);
    }
    else
    {
		if (mResizing1)
		{
			QPointF delta = event->pos() - event->lastPos();
            if (mOrientation == TopLeft || mOrientation == BottomLeft)
            {
			    if (rect().width() + delta.x() < (qreal)sMinWidth)
				    delta.setX((qreal)sMinWidth - rect().width());
            }
            else
            {
			    if (rect().width() - delta.x() < (qreal)sMinWidth)
				    delta.setX((qreal)sMinWidth - rect().width());
            }
			if (mOrientation == TopLeft || mOrientation == BottomLeft)
			{
				setRect(QRectF(
					rect().topLeft(), 
					QSizeF(rect().width() + delta.x(), 
						rect().height())), 
					mOrientation);
			}
			else
			{
				setRect(QRectF(
					rect().left() + delta.x(),
					rect().top(),
					rect().width() - delta.x(),
					rect().height()),
					mOrientation
					);
			}
		}
		if (mResizing2)
		{
			QPointF delta = event->pos() - event->lastPos();
            if (mOrientation == BottomRight || mOrientation == BottomLeft)
            {
                if (rect().height() - delta.y() < (qreal)sMinHeight)
				    delta.setY((qreal)sMinHeight - rect().height());
            }
            else
            {
			    if (rect().height() + delta.y() < (qreal)sMinHeight)
				    delta.setY((qreal)sMinHeight - rect().height());
            }
            if (mOrientation == BottomRight || mOrientation == BottomLeft)
                setRect(QRectF(
				    rect().left(),
				    rect().top()  + delta.y(),
				    rect().width(),
				    rect().height() - delta.y()),
                    mOrientation);
            else
                setRect(QRectF(
				    rect().left(),
				    rect().top(),
				    rect().width(),
				    rect().height() + delta.y()),
                    mOrientation);
		}
        if (mRotating)
        {
            QLineF currentLine(rotationCenter(), event->pos());
            QLineF lastLine(rotationCenter(), event->lastPos());
            rotateAroundCenter(currentLine.angleTo(lastLine));
        }

        event->accept();
    }
}

void UBGraphicsTriangle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (mResizing1 || mResizing2 || mRotating)
    {
        if (mRotating)
            updateResizeCursor();
        mResizing1 = false;
		mResizing2 = false;
		mRotating = false;
        event->accept();
    }
    else if (closeButtonRect().contains(event->pos()))
    {
	    hide();
		emit hidden();
        event->accept();
    }
	else if (hFlipRect().contains(event->pos()))
	{
		switch(mOrientation)
		{
		case BottomLeft:
			setOrientation(BottomRight);
			break;
		case BottomRight:
			setOrientation(BottomLeft);
			break;
		case TopLeft:
			setOrientation(TopRight);
			break;
		case TopRight:
			setOrientation(TopLeft);
			break;
		}
	}
	else if (vFlipRect().contains(event->pos()))
	{
		switch(mOrientation)
		{
		case BottomLeft:
			setOrientation(TopLeft);
			break;
		case BottomRight:
			setOrientation(TopRight);
			break;
		case TopLeft:
			setOrientation(BottomLeft);
			break;
		case TopRight:
			setOrientation(BottomRight);
			break;
		}
	}
    else
    {
		QGraphicsItem::mouseReleaseEvent(event);
    }
	mShowButtons = true;
	update();
    if (scene())
        scene()->setModified(true);
}

void UBGraphicsTriangle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

	if (currentTool == UBStylusTool::Selector)
	{
		mCloseSvgItem->setParentItem(this);

	    mShowButtons = true;
		mCloseSvgItem->setVisible(true);
		mHFlipSvgItem->setVisible(true);
		mVFlipSvgItem->setVisible(true);
		mRotateSvgItem->setVisible(true);

		if (resize1Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
            setCursor(resizeCursor1());
        else if(resize2Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
			setCursor(resizeCursor2());
		else if (closeButtonRect().contains(event->pos()))
			setCursor(closeCursor());
		else if (hFlipRect().contains(event->pos())
			|| vFlipRect().contains(event->pos()))
				setCursor(flipCursor());
		else if (rotateRect().contains(event->pos()))
			setCursor(rotateCursor());
		else
			setCursor(moveCursor());

		event->accept();
		update();
	}
}

void UBGraphicsTriangle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mShowButtons = false;
    setCursor(Qt::ArrowCursor);
    mCloseSvgItem->setVisible(false);
	mVFlipSvgItem->setVisible(false);
	mHFlipSvgItem->setVisible(false);
    mRotateSvgItem->setVisible(false);
    event->accept();
    update();
}

void UBGraphicsTriangle::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

	if (currentTool == UBStylusTool::Selector)
	{
		mCloseSvgItem->setVisible(mShowButtons);
		mVFlipSvgItem->setVisible(mShowButtons);
		mHFlipSvgItem->setVisible(mShowButtons);
		mRotateSvgItem->setVisible(mShowButtons);

		if (resize1Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
            setCursor(resizeCursor1());
        else if (resize2Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
			setCursor(resizeCursor2());
		else if (closeButtonRect().contains(event->pos()))
			setCursor(closeCursor());
		else if (hFlipRect().contains(event->pos())
			|| vFlipRect().contains(event->pos()))
				setCursor(flipCursor());
		else if (rotateRect().contains(event->pos()))
			setCursor(rotateCursor());
		else
			setCursor(moveCursor());

		event->accept();
	}
}
