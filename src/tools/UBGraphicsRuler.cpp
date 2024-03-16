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




#include <QLinearGradient>
#include <QBrush>
#include <QPainterPath>
#include <QPixmap>

#include "tools/UBGraphicsRuler.h"
#include "domain/UBGraphicsScene.h"
#include "core/UBApplication.h"
#include "gui/UBResources.h"
#include "board/UBBoardController.h" // TODO UB 4.x clean that dependency
#include "board/UBDrawingController.h"

#include "core/memcheck.h"

const QRect UBGraphicsRuler::sDefaultRect = QRect(0, 0, 800, 96);

UBGraphicsRuler::UBGraphicsRuler()
    : QGraphicsRectItem()
    , mResizing(false)
    , mRotating(false)
{
    setRect(sDefaultRect);

    mResizeSvgItem = new QGraphicsSvgItem(":/images/resizeRuler.svg", this);
    mResizeSvgItem->setVisible(false);
    mResizeSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mRotateSvgItem = new QGraphicsSvgItem(":/images/rotateTool.svg", this);
    mRotateSvgItem->setVisible(false);
    mRotateSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    create(*this);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::CppTool)); //Necessary to set if we want z value to be assigned correctly

    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, true); //needed to receive key events
    updateResizeCursor();
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


UBGraphicsRuler::~UBGraphicsRuler()
{
    // NOOP
}

UBItem* UBGraphicsRuler::deepCopy() const
{
    UBGraphicsRuler* copy = new UBGraphicsRuler();

    copyItemParameters(copy);

    // TODO UB 4.7 ... complete all members ?

    return copy;
}

void UBGraphicsRuler::copyItemParameters(UBItem *copy) const
{
    UBGraphicsRuler *cp = dynamic_cast<UBGraphicsRuler*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setRect(this->rect());
        cp->setTransform(this->transform());
    }
}

void UBGraphicsRuler::paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget)
{
    Q_UNUSED(styleOption);
    Q_UNUSED(widget);

    UBAbstractDrawRuler::paint();

    QTransform antiScaleTransformResize;
    qreal ratio = mAntiScaleRatio > 1.0 ? mAntiScaleRatio : 1.0;
    antiScaleTransformResize.scale(ratio, 1.0);

    mResizeSvgItem->setTransform(antiScaleTransformResize);
    mResizeSvgItem->setPos(resizeButtonRect().topLeft());

    mRotateSvgItem->setPos(rotateButtonRect().topLeft());

    QPainterPath outline = QPainterPath();
    outline.addRoundedRect(rect(), sRoundingRadius, sRoundingRadius);

    painter->setPen(drawColor());
    painter->setBrush(edgeFillColor());
    painter->setRenderHint(QPainter::Antialiasing, true);

    fillBackground(painter, outline);
    drawBorder(painter, outline);
    paintHelp(painter);
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

void UBGraphicsRuler::paintHelp(QPainter *painter)
{
    if (hasFocus())
    {
         //help message to aknowledge the user that the tool can be moved with the arrow keys
         painter->setPen(drawColor());
         painter->setFont(QFont("Arial",9));
         QFontMetricsF fontMetrics(painter->font());
         int textWidth = fontMetrics.horizontalAdvance(tr("use arrow keys for precise moves"));

         if (rect().width()/2 > textWidth)
         {
             painter->drawText(rect(), Qt::AlignCenter, tr("use arrow keys for precise moves"));

             mMoveToolSvgItem->setPos(
                 rect().center().x() + (textWidth/2) + 5 /* (a 5px margin between the text and the icon) */,
                 rect().center().y() - mMoveToolSvgItem->boundingRect().height() / 2);

             mMoveToolSvgItem->setVisible(true);
         }
         else
         {
             mMoveToolSvgItem->setVisible(false);
         }
    }
    else
    {
        mMoveToolSvgItem->setVisible(false);
    }
}

void UBGraphicsRuler::fillBackground(QPainter *painter, const QPainterPath &path)
{
    QLinearGradient linearGradient = QLinearGradient(
        rect().topLeft(),
        rect().bottomLeft());

    linearGradient.setColorAt(0, edgeFillColor());
    linearGradient.setColorAt(0.25, middleFillColor());
    linearGradient.setColorAt(0.75, middleFillColor());
    linearGradient.setColorAt(1, edgeFillColor());

    QBrush brush = QBrush(linearGradient);
    painter->fillPath(path, brush);
}

void UBGraphicsRuler::drawBorder(QPainter *painter, const QPainterPath &path)
{
    painter->drawPath(path);
}

void UBGraphicsRuler::paintGraduations(QPainter *painter)
{
    painter->save();
    painter->setFont(font());
    QFontMetricsF fontMetrics(painter->font());

    // Update the width of one "centimeter" to correspond to the width of the background grid (whether it is displayed or not)
    sPixelsPerCentimeter = UBApplication::boardController->activeScene()->backgroundGridSize();

    qreal pixelsPerMillimeter = sPixelsPerCentimeter/10.0;
    int rulerLengthInMillimeters = (rect().width() - sLeftEdgeMargin - sRoundingRadius)/pixelsPerMillimeter;

    // When a "centimeter" is too narrow, we only display every 5th number, and every 5th millimeter mark
    double numbersWidth = fontMetrics.horizontalAdvance("00");
    bool shouldDisplayAllNumbers = (numbersWidth <= (sPixelsPerCentimeter - 5));

    for (int millimeters(0); millimeters < rulerLengthInMillimeters; millimeters++) {

        double graduationX = rotationCenter().x() + pixelsPerMillimeter * millimeters;
        double graduationHeight = 0;

        if (millimeters % UBGeometryUtils::millimetersPerCentimeter == 0)
            graduationHeight = UBGeometryUtils::centimeterGraduationHeight;

        else if (millimeters % UBGeometryUtils::millimetersPerHalfCentimeter == 0)
            graduationHeight = UBGeometryUtils::halfCentimeterGraduationHeight;

        else
            graduationHeight = UBGeometryUtils::millimeterGraduationHeight;


        if (shouldDisplayAllNumbers || millimeters % UBGeometryUtils::millimetersPerHalfCentimeter == 0) {
            painter->drawLine(QLineF(graduationX, rotationCenter().y(), graduationX, rotationCenter().y() + graduationHeight));
            painter->drawLine(QLineF(graduationX, rotationCenter().y() + rect().height(), graduationX, rotationCenter().y() + rect().height() - graduationHeight));
        }


        if ((shouldDisplayAllNumbers && millimeters % UBGeometryUtils::millimetersPerCentimeter == 0)
            || millimeters % (UBGeometryUtils::millimetersPerCentimeter*5) == 0)
        {
            QString text = QString("%1").arg((int)(millimeters / UBGeometryUtils::millimetersPerCentimeter));

            if (graduationX + fontMetrics.horizontalAdvance(text) / 2 < rect().right()) {
                qreal textWidth = fontMetrics.horizontalAdvance(text);
                qreal textHeight = fontMetrics.tightBoundingRect(text).height() + 5;
                painter->drawText(
                    QRectF(graduationX - textWidth / 2, rect().top() + 5 + UBGeometryUtils::centimeterGraduationHeight, textWidth, textHeight),
                    Qt::AlignVCenter, text);
                painter->drawText(
                    QRectF(graduationX - textWidth / 2, rect().bottom() - 5 - UBGeometryUtils::centimeterGraduationHeight - textHeight, textWidth, textHeight),
                    Qt::AlignVCenter, text);
            }
        }
    }

    painter->restore();

}

void UBGraphicsRuler::paintRotationCenter(QPainter *painter)
{
    painter->drawArc(
        rotationCenter().x() - sRotationRadius, rotationCenter().y() - sRotationRadius,
        2 * sRotationRadius, 2 * sRotationRadius,
        270 * sDegreeToQtAngleUnit, 90 * sDegreeToQtAngleUnit);
}

void UBGraphicsRuler::rotateAroundCenter(qreal angle)
{
    QTransform transform;
    transform.translate(rotationCenter().x(), rotationCenter().y());
    transform.rotate(angle);
    transform.translate(- rotationCenter().x(), - rotationCenter().y());
    setTransform(transform, true);
}

QPointF UBGraphicsRuler::rotationCenter() const
{
    return QPointF(rect().x() + sLeftEdgeMargin, rect().y());
}


QRectF UBGraphicsRuler::resizeButtonRect() const
{
    QPixmap resizePixmap(":/images/resizeRuler.svg");
    qreal ratio = mAntiScaleRatio > 1.0 ? mAntiScaleRatio : 1.0;
    QSizeF resizeRectSize(
        resizePixmap.rect().width() * ratio,
        rect().height());

    QPointF resizeRectTopLeft(rect().width() - resizeRectSize.width(), 0);

    QRectF resizeRect(resizeRectTopLeft, resizeRectSize);
    resizeRect.translate(rect().topLeft());

    return resizeRect;
}

QRectF UBGraphicsRuler::closeButtonRect() const
{
    QPixmap closePixmap(":/images/closeTool.svg");

    QSizeF closeRectSize(
        closePixmap.width(),
        closePixmap.height());

    QPointF closeRectCenter(
        rect().left() + sLeftEdgeMargin + sPixelsPerCentimeter/2,
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
        rotatePixmap.width(),
        rotatePixmap.height());

    qreal marginToResizeButton = 0.5 * sPixelsPerCentimeter;
    QPointF rotateRectCenter(
        rect().right() - resizeButtonRect().width() - marginToResizeButton,
        rect().center().y());

    QPointF rotateRectTopLeft(
        rotateRectCenter.x() - rotateRectSize.width() / 2,
        rotateRectCenter.y() - rotateRectSize.height() / 2);

    return QRectF(rotateRectTopLeft, rotateRectSize);
}

void UBGraphicsRuler::keyPressEvent(QKeyEvent *event)
{
    QGraphicsItem::keyPressEvent(event);
    switch (event->key())
    {
    case Qt::Key_Up:
        moveBy(0, -1);
        event->accept();;
        break;
    case Qt::Key_Down:
        moveBy(0, 1);
        event->accept();
        break;
    case Qt::Key_Left:
        moveBy(-1, 0);
        event->accept();
        break;
    case Qt::Key_Right:
        moveBy(1, 0);
        event->accept();
        break;
    }
}

void UBGraphicsRuler::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

    if (currentTool == UBStylusTool::Selector || currentTool == UBStylusTool::Play)
    {
        mCloseSvgItem->setVisible(mShowButtons);
        mResizeSvgItem->setVisible(mShowButtons);
        mRotateSvgItem->setVisible(mShowButtons);
        if (resizeButtonRect().contains(event->pos()))
            setCursor(resizeCursor());
        else if (closeButtonRect().contains(event->pos()))
            setCursor(closeCursor());
        else if (rotateButtonRect().contains(event->pos()))
            setCursor(rotateCursor());
        else
            setCursor(moveCursor());

        event->accept();
    }
    else if (UBDrawingController::drawingController()->isDrawingTool())
    {
        event->accept();
    }
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
        QGraphicsItem::mousePressEvent(event);
    }
    mResizeSvgItem->setVisible(mShowButtons && mResizing);
    mRotateSvgItem->setVisible(mShowButtons && mRotating);
    mCloseSvgItem->setVisible(false);
}

void UBGraphicsRuler::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mResizing && !mRotating)
    {
        QGraphicsItem::mouseMoveEvent(event);
    }
    else
    {
        if (mResizing)
        {
            QPointF delta = event->pos() - event->lastPos();
            if (rect().width() + delta.x() < sMinLength)
                delta.setX(sMinLength - rect().width());

            if (rect().width() + delta.x() > sMaxLength)
                delta.setX(sMaxLength - rect().width());

            setRect(QRectF(rect().topLeft(), QSizeF(rect().width() + delta.x(), rect().height())));
        }
        else
        {
            QLineF currentLine(rotationCenter(), event->pos());
            QLineF lastLine(rotationCenter(), event->lastPos());
            rotateAroundCenter(currentLine.angleTo(lastLine));
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
        update(QRectF(rotationCenter(), QSizeF(sRotationRadius, sRotationRadius)));
        event->accept();
    }
    else if (closeButtonRect().contains(event->pos()))
    {
        hide();
        event->accept();
    }
    else
    {
        QGraphicsItem::mouseReleaseEvent(event);
    }

    if (scene())
        scene()->setModified(true);
}

void UBGraphicsRuler::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

    if (currentTool == UBStylusTool::Selector ||
        currentTool == UBStylusTool::Play)
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
        UBDrawingController::drawingController()->setActiveRuler(this);
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
    UBDrawingController::drawingController()->setActiveRuler(nullptr);
    event->accept();
    update();
}



std::shared_ptr<UBGraphicsScene> UBGraphicsRuler::scene() const
{
    auto scenePtr = dynamic_cast<UBGraphicsScene*>(QGraphicsRectItem::scene());
    return scenePtr ? scenePtr->shared_from_this() : nullptr;
}

void UBGraphicsRuler::StartLine(const QPointF& scenePos, qreal width)
{
    Q_UNUSED(width);

    QPointF itemPos = mapFromScene(scenePos);

    mStrokeWidth = UBDrawingController::drawingController()->currentToolWidth();

    qreal y;

    if (itemPos.y() > rect().y() + rect().height() / 2)
    {
        drawLineDirection = 0;
        y = rect().y() + rect().height() + mStrokeWidth / 2;
    }
    else
    {
        drawLineDirection = 1;
        y = rect().y() - mStrokeWidth /2;
    }

    if (itemPos.x() < rect().x() + sLeftEdgeMargin)
        itemPos.setX(rect().x() + sLeftEdgeMargin);
    if (itemPos.x() > rect().x() + rect().width() - sLeftEdgeMargin)
        itemPos.setX(rect().x() + rect().width() - sLeftEdgeMargin);

    itemPos.setY(y);
    itemPos = mapToScene(itemPos);

    scene()->moveTo(itemPos);
    scene()->drawLineTo(itemPos, mStrokeWidth, true);
}

void UBGraphicsRuler::DrawLine(const QPointF& scenePos, qreal width)
{
    Q_UNUSED(width);
    QPointF itemPos = mapFromScene(scenePos);

    qreal y;
    if (drawLineDirection == 0)
    {
        y = rect().y() + rect().height() + mStrokeWidth / 2;
    }
    else
    {
        y = rect().y() - mStrokeWidth /2;
    }
    if (itemPos.x() < rect().x() + sLeftEdgeMargin)
        itemPos.setX(rect().x() + sLeftEdgeMargin);
    if (itemPos.x() > rect().x() + rect().width() - sLeftEdgeMargin)
        itemPos.setX(rect().x() + rect().width() - sLeftEdgeMargin);

    itemPos.setY(y);
    itemPos = mapToScene(itemPos);

    // We have to use "pointed" line for marker tool
    scene()->drawLineTo(itemPos, mStrokeWidth, UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Marker);
}

void UBGraphicsRuler::EndLine()
{
    // We never come to this place
    scene()->inputDeviceRelease();
}
