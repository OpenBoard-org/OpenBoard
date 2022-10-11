/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include <QPixmap>

#include "tools/UBGraphicsAxes.h"
#include "domain/UBGraphicsScene.h"
#include "frameworks/UBGeometryUtils.h"
#include "core/UBApplication.h"
#include "gui/UBResources.h"
#include "board/UBBoardController.h" // TODO UB 4.x clean that dependency
#include "board/UBDrawingController.h"

#include "core/memcheck.h"

const QRect UBGraphicsAxes::sDefaultRect = QRect(-200, -200, 400, 400);

const QColor UBGraphicsAxes::sLightBackgroundDrawColor = QColor(0x33, 0x33, 0x33, sDrawTransparency);
const QColor UBGraphicsAxes::sDarkBackgroundDrawColor = QColor(0xff, 0xff, 0xff, sDrawTransparency);


UBGraphicsAxes::UBGraphicsAxes()
    : QGraphicsPolygonItem()
    , mResizing(false)
{
    setRect(sDefaultRect);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setAcceptHoverEvents(true);

    mCloseSvgItem = new QGraphicsSvgItem(":/images/closeTool.svg", this);
    mCloseSvgItem->setVisible(false);
    mCloseSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mNumbersSvgItem = new QGraphicsSvgItem(":/images/numbersTool.svg", this);
    mNumbersSvgItem->setVisible(false);
    mNumbersSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mShowNumbers = true;

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::CppTool)); //Necessary to set if we want z value to be assigned correctly

    setFlag(QGraphicsItem::ItemIsSelectable, false);
    updateResizeCursor();

    connect(UBApplication::boardController, &UBBoardController::zoomChanged, [this](qreal){
        // recalculate shape when zoom factor changes
        setRect(mBounds);
    });
}

UBGraphicsAxes::~UBGraphicsAxes()
{
    // NOOP
}

UBItem* UBGraphicsAxes::deepCopy() const
{
    UBGraphicsAxes* copy = new UBGraphicsAxes();

    copyItemParameters(copy);

    // TODO UB 4.7 ... complete all members ?

    return copy;
}

void UBGraphicsAxes::copyItemParameters(UBItem *copy) const
{
    UBGraphicsAxes *cp = dynamic_cast<UBGraphicsAxes*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setRect(this->mBounds);
        cp->setTransform(this->transform());
    }
}

void UBGraphicsAxes::setRect(qreal x, qreal y, qreal w, qreal h)
{
    //  Save the bounds rect
    mBounds.setX(x);  mBounds.setY(y);  mBounds.setWidth(w); mBounds.setHeight(h);

    mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
    qreal range = sItemWidth * mAntiScaleRatio;
    QPolygonF polygon;
    polygon << QPointF(-range, -range)
            << QPointF(-range, mBounds.top())
            << QPointF(range, mBounds.top())
            << QPointF(range, -range)
            << QPointF(mBounds.right(), -range)
            << QPointF(mBounds.right(), range)
            << QPointF(range, range)
            << QPointF(range, mBounds.bottom())
            << QPointF(-range, mBounds.bottom())
            << QPointF(-range, range)
            << QPointF(mBounds.left(), range)
            << QPointF(mBounds.left(), -range);
    setPolygon(polygon);
}

QRectF UBGraphicsAxes::bounds() const
{
    return mBounds;
}

void UBGraphicsAxes::setShowNumbers(bool showNumbers)
{
    mShowNumbers = showNumbers;
}

bool UBGraphicsAxes::showNumbes() const
{
    return mShowNumbers;
}

void UBGraphicsAxes::paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget)
{
    Q_UNUSED(styleOption);
    Q_UNUSED(widget);

    mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
    QTransform antiScaleTransform;
    antiScaleTransform.scale(mAntiScaleRatio, mAntiScaleRatio);

    mCloseSvgItem->setTransform(antiScaleTransform);
    mCloseSvgItem->setPos(closeButtonRect().topLeft());

    mNumbersSvgItem->setTransform(antiScaleTransform);
    mNumbersSvgItem->setPos(numbersButtonRect().topLeft());

    QTransform antiScaleTransform2;
    qreal ratio = mAntiScaleRatio > 1.0 ? mAntiScaleRatio : 1.0;
    antiScaleTransform2.scale(ratio, 1.0);

    QPen pen(drawColor());
    pen.setWidthF(2);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawLine(xAxis());
    painter->drawLine(yAxis());

    // draw arrows at end
    QPointF tip = xAxis().p2();
    painter->drawLine(tip.x(), tip.y(), tip.x() - sArrowLength, tip.y() + sArrowWidth);
    painter->drawLine(tip.x(), tip.y(), tip.x() - sArrowLength, tip.y() - sArrowWidth);

    tip = yAxis().p2();
    painter->drawLine(tip.x(), tip.y(), tip.x() + sArrowWidth, tip.y() + sArrowLength);
    painter->drawLine(tip.x(), tip.y(), tip.x() - sArrowWidth, tip.y() + sArrowLength);

    pen.setWidthF(1);
    painter->setPen(pen);
    paintGraduations(painter);
}


QVariant UBGraphicsAxes::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemVisibleHasChanged)
    {
        mCloseSvgItem->setParentItem(this);
        mNumbersSvgItem->setParentItem(this);
    }

    return QGraphicsPolygonItem::itemChange(change, value);
}

void UBGraphicsAxes::paintGraduations(QPainter *painter)
{
    painter->save();
    painter->setFont(font());
    QFontMetricsF fontMetrics(painter->font());

    // Update the width of one "centimeter" to correspond to the width of the background grid (whether it is displayed or not)
    mPixelsPerCentimeter = UBApplication::boardController->activeScene()->backgroundGridSize();

    // When a "centimeter" is too narrow, we only display every 5th number
    double numbersWidth = fontMetrics.boundingRect("-00").width();
    bool shouldDisplayAllNumbers = (numbersWidth <= (mPixelsPerCentimeter - 5));

    // draw numbers on x axis
    int fromX = (xAxis().x1() + sMargin) / mPixelsPerCentimeter;
    int toX = (xAxis().x2() - sMargin) / mPixelsPerCentimeter;

    for (int centimeters(fromX); centimeters <= toX; centimeters++)
    {
        bool isImportant = abs(centimeters) == 1 || abs(centimeters) % 5 == 0;
        double graduationX = mPixelsPerCentimeter * centimeters;
        double graduationHeight = UBGeometryUtils::millimeterGraduationHeight;

        painter->drawLine(QLineF(graduationX, graduationHeight, graduationX, -graduationHeight));

        if (mShowNumbers && (shouldDisplayAllNumbers || isImportant) && centimeters != 0)
        {
            QString text = QString("%1").arg(centimeters);

            if (graduationX + fontMetrics.horizontalAdvance(text) / 2 < xAxis().x2())
            {
                qreal textWidth = fontMetrics.horizontalAdvance(text);
                qreal textHeight = fontMetrics.tightBoundingRect(text).height();
                QRectF textRect(graduationX - textWidth / 2, textHeight - 5, textWidth, textHeight);

                // draw numbers only if they are completely within the bounds
                if (mBounds.contains(textRect))
                {
                    painter->drawText(textRect, Qt::AlignVCenter, text);
                }
            }
        }
    }

    // draw numbers on y axis
    int fromY = (-yAxis().y1() + sMargin) / mPixelsPerCentimeter;
    int toY = (-yAxis().y2() - sMargin) / mPixelsPerCentimeter;

    for (int centimeters(fromY); centimeters <= toY; centimeters++)
    {
        bool isImportant = abs(centimeters) == 1 || abs(centimeters) % 5 == 0;
        double graduationY = - mPixelsPerCentimeter * centimeters;
        double graduationHeight = UBGeometryUtils::millimeterGraduationHeight;

        painter->drawLine(QLineF(graduationHeight, graduationY, - graduationHeight, graduationY));

        if (mShowNumbers && (shouldDisplayAllNumbers || isImportant) && centimeters != 0)
        {
            QString text = QString("%1").arg(centimeters);

            qreal textWidth = fontMetrics.horizontalAdvance(text);
            qreal textHeight = fontMetrics.tightBoundingRect(text).height();
            QRectF textRect(- textWidth - 10, graduationY - textHeight / 2, textWidth, textHeight);

            // draw numbers only if they are completely within the bounds
            if (mBounds.contains(textRect))
            {
                painter->drawText(textRect, Qt::AlignVCenter, text);
            }
        }
    }

    painter->restore();
}

void UBGraphicsAxes::setRect(const QRectF &rect)
{
    setRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void UBGraphicsAxes::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (resizeLeftRect().contains(event->pos()))
    {
        mResizeDirection = Left;
        mResizing = true;
        event->accept();
    }
    else if (resizeRightRect().contains(event->pos()))
    {
        mResizeDirection = Right;
        mResizing = true;
        event->accept();
    }
    else if (resizeTopRect().contains(event->pos()))
    {
        mResizeDirection = Top;
        mResizing = true;
        event->accept();
    }
    else if (resizeBottomRect().contains(event->pos()))
    {
        mResizeDirection = Bottom;
        mResizing = true;
        event->accept();
    }
    else
    {
        mResizing = false;
        QGraphicsItem::mousePressEvent(event);
    }
    mCloseSvgItem->setVisible(false);
    mNumbersSvgItem->setVisible(false);
}

void UBGraphicsAxes::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mResizing)
    {
        QGraphicsItem::mouseMoveEvent(event);
    }
    else
    {
        QPointF delta = event->pos() - event->lastPos();
        prepareGeometryChange();

        switch (mResizeDirection)
        {
        case Left:
            delta.setY(0);
            if (delta.x() > -mBounds.left() - sMinLength)
                delta.setX(-mBounds.left() - sMinLength);
            setRect(mBounds.x() + delta.x(), mBounds.y(), mBounds.width() - delta.x(), mBounds.height());
            break;

        case Right:
            if (-delta.x() > mBounds.right() - sMinLength)
                delta.setX(sMinLength - mBounds.right());
            setRect(mBounds.x(), mBounds.y(), mBounds.width() + delta.x(), mBounds.height());
            break;

        case Top:
            delta.setX(0);
            if (delta.y() > -mBounds.top() - sMinLength)
                delta.setY(-mBounds.top() - sMinLength);
            setRect(mBounds.x(), mBounds.y() + delta.y(), mBounds.width(), mBounds.height() - delta.y());
            break;

        case Bottom:
            if (-delta.y() > mBounds.bottom() - sMinLength)
                delta.setY(sMinLength - mBounds.bottom());
            setRect(mBounds.x(), mBounds.y(), mBounds.width(), mBounds.height() + delta.y());
            break;
        }

        event->accept();
    }
}

void UBGraphicsAxes::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (mResizing)
    {
        mResizing = false;
        event->accept();
    }
    else if (closeButtonRect().contains(event->pos()))
    {
        hide();
        event->accept();
    }
    else if (numbersButtonRect().contains(event->pos()))
    {
        mShowNumbers = !mShowNumbers;
        update(boundingRect());
        event->accept();
    }
    else
    {
        // snap to grid
        if (true) {
            QPointF delta = pos();
            qreal gridSize = scene()->backgroundGridSize();
            qreal deltaX = delta.x() - round(delta.x() / gridSize) * gridSize;
            qreal deltaY = delta.y() - round(delta.y() / gridSize) * gridSize;
            setPos(pos() - QPointF(deltaX, deltaY));
        }

        QGraphicsItem::mouseReleaseEvent(event);
    }

    if (scene())
        scene()->setModified(true);
}

void UBGraphicsAxes::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

    if (currentTool == UBStylusTool::Selector ||
        currentTool == UBStylusTool::Play)
    {
        mCloseSvgItem->setParentItem(this);
        mNumbersSvgItem->setParentItem(this);

        mShowButtons = true;
        mCloseSvgItem->setVisible(mShowButtons);
        mNumbersSvgItem->setVisible(mShowButtons);
        selectCursor(event);
        event->accept();
        update();
    }
}

void UBGraphicsAxes::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

    if (currentTool == UBStylusTool::Selector || currentTool == UBStylusTool::Play)
    {
        mCloseSvgItem->setVisible(mShowButtons);
        mNumbersSvgItem->setVisible(mShowButtons);
        selectCursor(event);
        event->accept();
    }
}

void UBGraphicsAxes::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mShowButtons = false;
    setCursor(Qt::ArrowCursor);
    mCloseSvgItem->setVisible(mShowButtons);
    mNumbersSvgItem->setVisible(mShowButtons);
    event->accept();
    update();
}

void UBGraphicsAxes::updateResizeCursor()
{
    QPixmap pix(":/images/cursors/resize.png");
    QTransform tr;
    tr.rotate(-90);
    mResizeCursorH = QCursor(pix, pix.width() / 2,  pix.height() / 2);
    mResizeCursorV = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
}

QCursor UBGraphicsAxes::resizeCursor() const
{
    switch (mResizeDirection)
    {
    case Left:
    case Right:
        return mResizeCursorH;

    default:
        break;
    }

    return mResizeCursorV;
}

QCursor UBGraphicsAxes::closeCursor() const
{
    return Qt::ArrowCursor;
}

QCursor UBGraphicsAxes::moveCursor() const
{
    return Qt::SizeAllCursor;
}

void UBGraphicsAxes::selectCursor(QGraphicsSceneHoverEvent *event)
{
    if (resizeLeftRect().contains(event->pos()))
    {
        mResizeDirection = Left;
        setCursor(resizeCursor());
    }
    else if (resizeRightRect().contains(event->pos()))
    {
        mResizeDirection = Right;
        setCursor(resizeCursor());
    }
    else if (resizeTopRect().contains(event->pos()))
    {
        mResizeDirection = Top;
        setCursor(resizeCursor());
    }
    else if (resizeBottomRect().contains(event->pos()))
    {
        mResizeDirection = Bottom;
        setCursor(resizeCursor());
    } else if (closeButtonRect().contains(event->pos()))
        setCursor(closeCursor());
    else if (numbersButtonRect().contains(event->pos())) {
        setCursor(closeCursor());
    } else {
        setCursor(moveCursor());
    }
}

QRectF UBGraphicsAxes::resizeLeftRect() const
{
    qreal range = sItemWidth * mAntiScaleRatio;
    return QRectF(xAxis().x1(), xAxis().y1() - range, sArrowLength, 2 * range);
}

QRectF UBGraphicsAxes::resizeRightRect() const
{
    qreal range = sItemWidth * mAntiScaleRatio;
    return QRectF(xAxis().x2() - sArrowLength, xAxis().y2() - range, sArrowLength, 2 * range);
}

QRectF UBGraphicsAxes::resizeBottomRect() const
{
    qreal range = sItemWidth * mAntiScaleRatio;
    return QRectF(yAxis().x1() - range, yAxis().y1() - sArrowLength, 2 * range, sArrowLength);
}

QRectF UBGraphicsAxes::resizeTopRect() const
{
    qreal range = sItemWidth * mAntiScaleRatio;
    return QRectF(yAxis().x2() - range, yAxis().y2(), 2 * range, sArrowLength);
}

QRectF UBGraphicsAxes::closeButtonRect() const
{
    QPixmap closePixmap(":/images/closeTool.svg");

    QSizeF closeRectSize(
        closePixmap.width() * mAntiScaleRatio,
        closePixmap.height() * mAntiScaleRatio);

    QPointF closeRectTopLeft(
        -sItemWidth * mAntiScaleRatio,
        -sItemWidth * mAntiScaleRatio);

    return QRectF(closeRectTopLeft, closeRectSize);
}

QRectF UBGraphicsAxes::numbersButtonRect() const
{
    QPixmap numbersPixmap(":/images/numbersTool.svg");

    QSizeF numbersRectSize(
        numbersPixmap.width() * mAntiScaleRatio,
        numbersPixmap.height() * mAntiScaleRatio);

    QPointF numbersRectTopLeft(
        sItemWidth * mAntiScaleRatio - numbersRectSize.width(),
        sItemWidth * mAntiScaleRatio - numbersRectSize.height());

    return QRectF(numbersRectTopLeft, numbersRectSize);
}

QLineF UBGraphicsAxes::xAxis() const
{
    return QLineF(mBounds.left(), 0, mBounds.right(), 0);
}

QLineF UBGraphicsAxes::yAxis() const
{
    return QLineF(0, mBounds.bottom(), 0, mBounds.top());
}

UBGraphicsScene* UBGraphicsAxes::scene() const
{
    return static_cast<UBGraphicsScene*>(QGraphicsPolygonItem::scene());
}

QColor UBGraphicsAxes::drawColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundDrawColor : sLightBackgroundDrawColor;
}

QFont UBGraphicsAxes::font() const
{
    QFont font("Arial");
    font.setPixelSize(16);
    font.setBold(true);
    return font;
}
