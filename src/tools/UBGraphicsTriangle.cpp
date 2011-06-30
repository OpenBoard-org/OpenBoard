
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
	QPolygonF polygon;
	polygon << QPointF(x, y) << QPoint(x, y + h) << QPoint(x+w, y + h) << QPoint(x, y);
	setPolygon(polygon);

	setOrientation(orientation);
}

void UBGraphicsTriangle::setOrientation(UBGraphicsTriangleOrientation orientation)
{
	mOrientation = orientation;
	QTransform t;
	switch(orientation)
	{
	case BottomLeft:
		t.setMatrix(1, 0, 0, 0, 1, 0, 0, 0, 1);
		break;
	case BottomRight:
		t.setMatrix(-1, 0, 0, 0, 1, 0, boundingRect().right(), 0, 1);
		break;
	case TopLeft:
		t.setMatrix(1, 0, 0, 0, -1, 0, 0, boundingRect().bottom(), 1);
		break;
	case TopRight:
		t.setMatrix(-1, 0, 0, 0, -1, 0, boundingRect().right(), boundingRect().bottom(), 1);
		break;
	}
	setTransform(t, true);
}

UBGraphicsScene* UBGraphicsTriangle::scene() const
{
	return static_cast<UBGraphicsScene*>(QGraphicsPolygonItem::scene());
}

void UBGraphicsTriangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	QPointF A1(rect().x(), rect().y());
	QPointF B1(rect().x(), rect().y() + rect().height());
	QPointF C1(rect().x() + rect().width(), rect().y() + rect().height());

	qreal C = sqrt(rect().width() * rect().width() + rect().height() * rect().height());
	qreal L = (C * d + rect().width() * d)/ rect().height();
	qreal K = (C * d + rect().height() * d)/ rect().width();

	qreal W1 = rect().height() * d / C;
	qreal H1 = rect().width() * d / C;

	QPointF A2(rect().x() + d, rect().y() + K);
	QPointF B2(rect().x() + d, rect().y() + rect().height() - d);
	QPointF C2(rect().x() + rect().width() - L, rect().y() + rect().height() - d);

	QPoint CC(rect().x() + rect().width() - L + W1,
		rect().y() + rect().height() - d - H1);

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

    painter->save();
    painter->setFont(font());
    QFontMetricsF fontMetrics(painter->font());
    for (int millimeters = 0; millimeters < (rect().width() - sLeftEdgeMargin - sRoundingRadius) / sPixelsPerMillimeter; millimeters++)
    {
        int graduationX = rotationCenter().x() + sPixelsPerMillimeter * millimeters;
        int graduationHeight = (0 == millimeters % millimetersPerCentimeter) ?
            centimeterGraduationHeight :
            ((0 == millimeters % millimetersPerHalfCentimeter) ?
                halfCentimeterGraduationHeight : millimeterGraduationHeight);

		// Check that grad. line inside triangle
		qreal lineY = rect().bottom() - rect().height()/rect().width()*(rect().width() - graduationX);
		if (lineY >= rotationCenter().y() + rect().height() - graduationHeight)
			break;
        
        painter->drawLine(QLine(graduationX, rotationCenter().y(), graduationX, rotationCenter().y() - graduationHeight));
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


void UBGraphicsTriangle::rotateAroundCenter(qreal angle)
{
    QTransform transform;
    transform.translate(rotationCenter().x(), rotationCenter().y());
    transform.rotate(angle);
    transform.translate(- rotationCenter().x(), - rotationCenter().y());
    setTransform(transform, true);
}

QPointF	UBGraphicsTriangle::rotationCenter() const
{
    return QPointF(rect().x() + sLeftEdgeMargin , rect().bottom());
}

QRectF	UBGraphicsTriangle::closeButtonRect() const
{
	return QRectF(rect().x() + d - mCloseSvgItem->boundingRect().width() - 5, 
		rect().bottom() - d - mCloseSvgItem->boundingRect().height() - 5, 
		mCloseSvgItem->boundingRect().width(),
		mCloseSvgItem->boundingRect().height());
}

QPolygonF UBGraphicsTriangle::resize1Polygon() const
{
	qreal C = sqrt(rect().width() * rect().width() + rect().height() * rect().height());

	QPolygonF p;
	
	QPointF P1(rect().right() - sArrowLength, rect().bottom());
	QPointF P2(rect().right() - sArrowLength * rect().width() / C,
		rect().bottom() - sArrowLength * rect().height() / C );

	p << QPointF(rect().right(), rect().bottom()) << P1 << P2;

	return p;
}

QPolygonF UBGraphicsTriangle::resize2Polygon() const
{
	qreal C = sqrt(rect().width() * rect().width() + rect().height() * rect().height());

	QPolygonF p;
	
	QPointF P1(rect().left(), rect().top() + sArrowLength);
	QPointF P2(rect().left() + sArrowLength * rect().width() / C,
		rect().top() + sArrowLength * rect().height() / C );

	p << QPointF(rect().left(), rect().top()) << P1 << P2;

	return p;
}

QRectF	UBGraphicsTriangle::hFlipRect() const
{
	return QRectF(rect().x() + d - mHFlipSvgItem->boundingRect().width() - 5, 
		rect().bottom() - d - mCloseSvgItem->boundingRect().height() - mVFlipSvgItem->boundingRect().height() -
			mVFlipSvgItem->boundingRect().height() - 15, 
		mHFlipSvgItem->boundingRect().width(),
		mHFlipSvgItem->boundingRect().height());
}

QRectF	UBGraphicsTriangle::vFlipRect() const
{
	return QRectF(rect().x() + d - mVFlipSvgItem->boundingRect().width() - 5, 
		rect().bottom() - d - mCloseSvgItem->boundingRect().height() - mVFlipSvgItem->boundingRect().height() - 10, 
		mVFlipSvgItem->boundingRect().width(),
		mVFlipSvgItem->boundingRect().height());
}

QRectF	UBGraphicsTriangle::rotateRect() const
{
	QPointF A1(rect().x(), rect().y());
	QPointF B1(rect().x(), rect().y() + rect().height());
	QPointF C1(rect().x() + rect().width(), rect().y() + rect().height());

	qreal C = sqrt(rect().width() * rect().width() + rect().height() * rect().height());
	qreal L = (C * d + rect().width() * d)/ rect().height();

	QPointF C2(rect().x() + rect().width() - L, rect().y() + rect().height() - d);

	return QRectF(C2 + QPointF(20, 5), QSizeF(mRotateSvgItem->boundingRect().size()));

}

QCursor	UBGraphicsTriangle::moveResizeCursor() const
{
	return Qt::OpenHandCursor;
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
			if (rect().width() + delta.x() < sMinWidth)
				delta.setX(sMinWidth - rect().width());
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
				setRect(
					rect().left() - delta.x(),
					rect().top(),
					rect().width() + delta.x(),
					rect().height(),
					mOrientation
					);
			}
		}
		if (mResizing2)
		{
			QPointF delta = event->pos() - event->lastPos();
			if (rect().height() + delta.y() < sMinHeight)
				delta.setY(sMinHeight - rect().height());
			qDebug() << delta;
			setRect(QRect(
				rect().left(),
				rect().top() + delta.y(),
				rect().width(),
				rect().height() - delta.y()),
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

		if (resize1Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill) ||
			resize2Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
			setCursor(moveResizeCursor());
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

		if (resize1Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill) ||
			resize2Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
			setCursor(moveResizeCursor());
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
