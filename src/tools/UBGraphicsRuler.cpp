
/*
 * UBGraphicsRuler.cpp
 *
 *  Created on: April 16, 2009
 *      Author: Jerome Marchaud
 */

#include <QPixmap>

#include "tools/UBGraphicsRuler.h"
#include "domain/UBGraphicsScene.h"
#include "core/UBApplication.h"
#include "gui/UBResources.h"
#include "board/UBBoardController.h" // TODO UB 4.x clean that dependency
#include "board/UBDrawingController.h"

const QRect                     UBGraphicsRuler::sDefaultRect = QRect(0, 0, 800, 96);
const QColor UBGraphicsRuler::sLightBackgroundMiddleFillColor = QColor(0x72, 0x72, 0x72, sFillTransparency);
const QColor   UBGraphicsRuler::sLightBackgroundEdgeFillColor = QColor(0xc3, 0xc3, 0xc3, sFillTransparency);
const QColor       UBGraphicsRuler::sLightBackgroundDrawColor = QColor(0x33, 0x33, 0x33, sDrawTransparency);
const QColor  UBGraphicsRuler::sDarkBackgroundMiddleFillColor = QColor(0xb5, 0xb5, 0xb5, sFillTransparency);
const QColor    UBGraphicsRuler::sDarkBackgroundEdgeFillColor = QColor(0xdd, 0xdd, 0xdd, sFillTransparency);
const QColor        UBGraphicsRuler::sDarkBackgroundDrawColor = QColor(0xff, 0xff, 0xff, sDrawTransparency);


UBGraphicsRuler::UBGraphicsRuler()
    : QGraphicsRectItem()
    , mResizing(false)
    , mRotating(false)
    , mShowButtons(false)
    , mCloseSvgItem(0)
    , mRotateSvgItem(0)
    , mResizeSvgItem(0)
    , mAntiScaleRatio(1.0)
{

    setRect(sDefaultRect);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setAcceptsHoverEvents(true);

    mCloseSvgItem = new QGraphicsSvgItem(":/images/closeTool.svg", this);
    mCloseSvgItem->setVisible(false);
    mCloseSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mRotateSvgItem = new QGraphicsSvgItem(":/images/rotateTool.svg", this);
    mRotateSvgItem->setVisible(false);
    mRotateSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mResizeSvgItem = new QGraphicsSvgItem(":/images/resizeRuler.svg", this);
    mResizeSvgItem->setVisible(false);
    mResizeSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    updateResizeCursor();
}

UBGraphicsRuler::~UBGraphicsRuler()
{
    // NOOP
}

UBItem* UBGraphicsRuler::deepCopy() const
{
    UBGraphicsRuler* copy = new UBGraphicsRuler();

    copy->setPos(this->pos());
    copy->setRect(this->rect());
    copy->setZValue(this->zValue());
    copy->setTransform(this->transform());

    // TODO UB 4.7 ... complete all members ?

    return copy;
}

void UBGraphicsRuler::paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget)
{
    Q_UNUSED(styleOption);
    Q_UNUSED(widget);

    mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
    QTransform antiScaleTransform;
    antiScaleTransform.scale(mAntiScaleRatio, mAntiScaleRatio);

    mCloseSvgItem->setTransform(antiScaleTransform);
    mCloseSvgItem->setPos(closeButtonRect().topLeft());

    mRotateSvgItem->setTransform(antiScaleTransform);
    mRotateSvgItem->setPos(rotateButtonRect().topLeft());

    QTransform antiScaleTransform2;
    qreal ratio = mAntiScaleRatio > 1.0 ? mAntiScaleRatio : 1.0;
    antiScaleTransform2.scale(ratio, 1.0);
    mResizeSvgItem->setTransform(antiScaleTransform2);
    mResizeSvgItem->setPos(resizeButtonRect().topLeft());

    painter->setPen(drawColor());
    painter->drawRoundedRect(rect(), sRoundingRadius, sRoundingRadius);
    fillBackground(painter);
    paintGraduations(painter);
    if (mRotating)
        paintRotationCenter(painter);
}


QVariant UBGraphicsRuler::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemVisibleHasChanged)
    {
        mCloseSvgItem->setParentItem(this);
        mResizeSvgItem->setParentItem(this);
        mRotateSvgItem->setParentItem(this);
    }

    return QGraphicsRectItem::itemChange(change, value);
}


void UBGraphicsRuler::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->pos().x() > resizeButtonRect().left())
    {
        mResizing = true;
        event->accept();
    }
    else if (rotateButtonRect().contains(event->pos()))
    {
        mRotating = true;
        event->accept();
    }
    else
    {
        mResizeSvgItem->setVisible(false);
        mRotateSvgItem->setVisible(false);
        QGraphicsRectItem::mousePressEvent(event);
    }
    mResizeSvgItem->setVisible(mShowButtons && mResizing);
    mRotateSvgItem->setVisible(mShowButtons && mRotating);
    mCloseSvgItem->setVisible(false);
}

void UBGraphicsRuler::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mResizing && !mRotating)
    {
        QGraphicsRectItem::mouseMoveEvent(event);
    }
    else
    {
        if (mResizing)
        {
            QPointF delta = event->pos() - event->lastPos();
            if (rect().width() + delta.x() < sMinLength)
                delta.setX(sMinLength - rect().width());
            setRect(QRectF(rect().topLeft(), QSizeF(rect().width() + delta.x(), rect().height())));
        }
        else
        {
            QLineF currentLine(topLeftOrigin(), event->pos());
            QLineF lastLine(topLeftOrigin(), event->lastPos());
            rotateAroundTopLeftOrigin(currentLine.angleTo(lastLine));
        }

        event->accept();
    }
}

void UBGraphicsRuler::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (mResizing)
    {
        mResizing = false;
        event->accept();
    }
    else if (mRotating)
    {
        mRotating = false;
        updateResizeCursor();
        update(QRectF(topLeftOrigin(), QSizeF(sRotationRadius, sRotationRadius)));
        event->accept();
    }
    else if (closeButtonRect().contains(event->pos()))
    {
        hide();

        emit hidden();

        event->accept();
    }
    else
    {
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

    if (scene())
        scene()->setModified(true);
}

void UBGraphicsRuler::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

	if (currentTool == UBStylusTool::Selector)
	{
		mCloseSvgItem->setParentItem(this);
	    mResizeSvgItem->setParentItem(this);
		mRotateSvgItem->setParentItem(this);

	    mShowButtons = true;
		mCloseSvgItem->setVisible(mShowButtons);
		mResizeSvgItem->setVisible(mShowButtons);
	    mRotateSvgItem->setVisible(mShowButtons);
		if (event->pos().x() >= resizeButtonRect().left())
		{
			setCursor(resizeCursor());
		}
		else
		{
			if (closeButtonRect().contains(event->pos()))
				setCursor(closeCursor());
			else if (rotateButtonRect().contains(event->pos()))
				setCursor(rotateCursor());
			else
				setCursor(moveCursor());
		}
		event->accept();
		update();
	}
	else if (UBDrawingController::drawingController()->isDrawingTool())
	{
		setCursor(drawRulerLineCursor());
		UBDrawingController::drawingController()->mActiveRuler = this;
		event->accept();
	}
}

void UBGraphicsRuler::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mShowButtons = false;
    setCursor(Qt::ArrowCursor);
    mCloseSvgItem->setVisible(mShowButtons);
    mResizeSvgItem->setVisible(mShowButtons);
    mRotateSvgItem->setVisible(mShowButtons);
	UBDrawingController::drawingController()->mActiveRuler = NULL;
    event->accept();
    update();
}

void UBGraphicsRuler::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

	if (currentTool == UBStylusTool::Selector)
	{
		mCloseSvgItem->setVisible(mShowButtons);
		mResizeSvgItem->setVisible(mShowButtons);
		mRotateSvgItem->setVisible(mShowButtons);
		if (event->pos().x() >= resizeButtonRect().left())
		{
			setCursor(resizeCursor());
		}
		else
		{
			if (closeButtonRect().contains(event->pos()))
				setCursor(closeCursor());
			else if (rotateButtonRect().contains(event->pos()))
				setCursor(rotateCursor());
			else
				setCursor(moveCursor());
		}
		event->accept();
	}
	else if (currentTool == UBStylusTool::Pen || currentTool == UBStylusTool::Marker)
	{
		event->accept();
	}
}

void UBGraphicsRuler::fillBackground(QPainter *painter)
{
    QRectF rect1(rect().topLeft(), QSizeF(rect().width(), rect().height() / 4));
    QLinearGradient linearGradient1(
        rect1.topLeft(),
        rect1.bottomLeft());
    linearGradient1.setColorAt(0, edgeFillColor());
    linearGradient1.setColorAt(1, middleFillColor());
    painter->fillRect(rect1, linearGradient1);

    QRectF rect2(rect1.bottomLeft(), QSizeF(rect().width(), rect().height() / 2));
    painter->fillRect(rect2, middleFillColor());

    QRectF rect3(rect2.bottomLeft(), rect1.size());
    QLinearGradient linearGradient3(
        rect3.topLeft(),
        rect3.bottomLeft());
    linearGradient3.setColorAt(0, middleFillColor());
    linearGradient3.setColorAt(1, edgeFillColor());
    painter->fillRect(rect3, linearGradient3);
}

void UBGraphicsRuler::paintGraduations(QPainter *painter)
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
        painter->drawLine(QLine(graduationX, topLeftOrigin().y(), graduationX, topLeftOrigin().y() + graduationHeight));
        painter->drawLine(QLine(graduationX, topLeftOrigin().y() + rect().height(), graduationX, topLeftOrigin().y() + rect().height() - graduationHeight));
        if (0 == millimeters % millimetersPerCentimeter)
        {
            QString text = QString("%1").arg((int)(millimeters / millimetersPerCentimeter));
            if (graduationX + fontMetrics.width(text) / 2 < rect().right())
            {
                qreal textWidth = fontMetrics.width(text);
                qreal textHeight = fontMetrics.tightBoundingRect(text).height() + 5;
                painter->drawText(
                    QRectF(graduationX - textWidth / 2, rect().top() + 5 + centimeterGraduationHeight, textWidth, textHeight),
                    Qt::AlignVCenter, text);
                painter->drawText(
                    QRectF(graduationX - textWidth / 2, rect().bottom() - 5 - centimeterGraduationHeight - textHeight, textWidth, textHeight),
                    Qt::AlignVCenter, text);
            }
        }
    }
    painter->restore();
}

void UBGraphicsRuler::paintRotationCenter(QPainter *painter)
{
    painter->drawArc(
        topLeftOrigin().x() - sRotationRadius, topLeftOrigin().y() - sRotationRadius,
        2 * sRotationRadius, 2 * sRotationRadius,
        270 * sDegreeToQtAngleUnit, 90 * sDegreeToQtAngleUnit);
}

void UBGraphicsRuler::rotateAroundTopLeftOrigin(qreal angle)
{
    QTransform transform;
    transform.translate(topLeftOrigin().x(), topLeftOrigin().y());
    transform.rotate(angle);
    transform.translate(- topLeftOrigin().x(), - topLeftOrigin().y());
    setTransform(transform, true);
}

QPointF UBGraphicsRuler::topLeftOrigin() const
{
    return QPointF(rect().x() + sLeftEdgeMargin, rect().y());
}

QCursor UBGraphicsRuler::moveCursor() const
{
    return Qt::SizeAllCursor;
}

QCursor UBGraphicsRuler::resizeCursor() const
{
    return mResizeCursor;
}

QCursor UBGraphicsRuler::rotateCursor() const
{
    return UBResources::resources()->rotateCursor;
}

QCursor UBGraphicsRuler::closeCursor() const
{
    return Qt::ArrowCursor;
}

QRectF UBGraphicsRuler::resizeButtonRect() const
{
    QPixmap resizePixmap(":/images/resizeRuler.svg");
    QSizeF resizeRectSize(
        resizePixmap.rect().width(),
        rect().height());

    qreal ratio = mAntiScaleRatio > 1.0 ? mAntiScaleRatio : 1.0;
    QPointF resizeRectTopLeft(rect().width() - resizeRectSize.width() * ratio, 0);

    QRectF resizeRect(resizeRectTopLeft, resizeRectSize);
    resizeRect.translate(rect().topLeft());

    return resizeRect;
}

QRectF UBGraphicsRuler::closeButtonRect() const
{
    QPixmap closePixmap(":/images/closeTool.svg");

    QSizeF closeRectSize(
        closePixmap.width() * mAntiScaleRatio,
        closePixmap.height() * mAntiScaleRatio);

    QPointF closeRectCenter(
        rect().left() + sLeftEdgeMargin + sPixelsPerMillimeter * 5,
        rect().center().y());

    QPointF closeRectTopLeft(
        closeRectCenter.x() - closeRectSize.width() / 2,
        closeRectCenter.y() - closeRectSize.height() / 2);

    return QRectF(closeRectTopLeft, closeRectSize);
}

QRectF UBGraphicsRuler::rotateButtonRect() const
{
    QPixmap rotatePixmap(":/images/closeTool.svg");

    QSizeF rotateRectSize(
        rotatePixmap.width() * mAntiScaleRatio,
        rotatePixmap.height() * mAntiScaleRatio);

    int centimeters = (int)(rect().width() - sLeftEdgeMargin - resizeButtonRect().width()) / (int)(10 * sPixelsPerMillimeter);
    QPointF rotateRectCenter(
        rect().left() + sLeftEdgeMargin + (centimeters - 0.5) * 10 * sPixelsPerMillimeter,
        rect().center().y());

    QPointF rotateRectTopLeft(
        rotateRectCenter.x() - rotateRectSize.width() / 2,
        rotateRectCenter.y() - rotateRectSize.height() / 2);

    return QRectF(rotateRectTopLeft, rotateRectSize);
}

UBGraphicsScene* UBGraphicsRuler::scene() const
{
    return static_cast<UBGraphicsScene*>(QGraphicsRectItem::scene());
}

QColor UBGraphicsRuler::drawColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundDrawColor : sLightBackgroundDrawColor;
}

QColor UBGraphicsRuler::middleFillColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundMiddleFillColor : sLightBackgroundMiddleFillColor;
}

QColor UBGraphicsRuler::edgeFillColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundEdgeFillColor : sLightBackgroundEdgeFillColor;
}

QFont UBGraphicsRuler::font() const
{
    QFont font("Arial");
    font.setPixelSize(16);
    return font;
}

void UBGraphicsRuler::updateResizeCursor()
{
    QPixmap pix(":/images/cursors/resize.png");
    QTransform itemTransform = sceneTransform();
    QRectF itemRect = boundingRect();
    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QLineF topLine(topLeft, topRight);
    qreal angle = topLine.angle();
    QTransform tr;
    tr.rotate(- angle);
    QCursor resizeCursor  = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
    mResizeCursor = resizeCursor;
}

QCursor UBGraphicsRuler::drawRulerLineCursor() const
{
	return UBResources::resources()->drawLineRulerCursor;
}

void UBGraphicsRuler::StartLine(const QPointF& scenePos, qreal width)
{
	QPointF itemPos = mapFromScene(scenePos);

	qreal y;

	if (itemPos.y() > rect().y() + rect().height() / 2)
	{
		drawLineDirection = 0;
		y = rect().y() + rect().height() + width / 2;
	}
	else
	{
		drawLineDirection = 1;
		y = rect().y() - width /2;
	}
	
	if (itemPos.x() < rect().x() + sLeftEdgeMargin)
		itemPos.setX(rect().x() + sLeftEdgeMargin);
	if (itemPos.x() > rect().x() + rect().width() - sLeftEdgeMargin)
		itemPos.setX(rect().x() + rect().width() - sLeftEdgeMargin);

	itemPos.setY(y);
	itemPos = mapToScene(itemPos);

	scene()->moveTo(itemPos);
	scene()->drawLineTo(itemPos, width, true);
}

void UBGraphicsRuler::DrawLine(const QPointF& scenePos, qreal width)
{
	QPointF itemPos = mapFromScene(scenePos);

	qreal y;
	if (drawLineDirection == 0)
	{
		y = rect().y() + rect().height() + width / 2;
	}
	else
	{
		y = rect().y() - width /2;
	}
	if (itemPos.x() < rect().x() + sLeftEdgeMargin)
		itemPos.setX(rect().x() + sLeftEdgeMargin);
	if (itemPos.x() > rect().x() + rect().width() - sLeftEdgeMargin)
		itemPos.setX(rect().x() + rect().width() - sLeftEdgeMargin);

	itemPos.setY(y);
	itemPos = mapToScene(itemPos);

	// We have to use "pointed" line for marker tool
	scene()->drawLineTo(itemPos, width, 
		UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Marker);
}

void UBGraphicsRuler::EndLine()
{}
