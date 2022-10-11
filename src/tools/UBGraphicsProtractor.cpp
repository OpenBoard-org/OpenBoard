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




#include "UBGraphicsProtractor.h"
#include "core/UBApplication.h"
#include "gui/UBResources.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "UBAbstractDrawRuler.h"

#include "core/memcheck.h"

#include <QtWidgets/QGraphicsView>


const QRectF UBGraphicsProtractor::sDefaultRect = QRectF(-250, -250, 500, 500);
const qreal UBGraphicsProtractor::minRadius = 70;

UBGraphicsProtractor::UBGraphicsProtractor()
        : QGraphicsEllipseItem(sDefaultRect)
        , mCurrentTool(None)
        , mShowButtons(false)
        , mCurrentAngle(0)
        , mSpan(180)
        , mStartAngle(0)
        , mScaleFactor(1)
        , mResetSvgItem(0)
        , mResizeSvgItem(0)
        , mMarkerSvgItem(0)
{
    sFillTransparency = 127;
    sDrawTransparency = 192;
    create(*this);

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    setStartAngle(0);
    setSpanAngle(180 * 16);

    mResetSvgItem = new QGraphicsSvgItem(":/images/resetTool.svg", this);
    mResetSvgItem->setVisible(false);
    mResetSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mResizeSvgItem = new QGraphicsSvgItem(":/images/resizeTool.svg", this);
    mResizeSvgItem->setVisible(false);
    mResizeSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mMarkerSvgItem = new QGraphicsSvgItem(":/images/angleMarker.svg", this);
    mMarkerSvgItem->setVisible(false);
    mMarkerSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));

    mRotateSvgItem = new QGraphicsSvgItem(":/images/rotateTool.svg", this);
    mRotateSvgItem->setVisible(false);
    mRotateSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::CppTool)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    mCloseSvgItem->setPos(closeButtonRect().topLeft());
    mResetSvgItem->setPos(resetButtonRect().topLeft());
    mResizeSvgItem->setPos(resizeButtonRect().topLeft());
    mMarkerSvgItem->setPos(markerButtonRect().topLeft());
    mRotateSvgItem->setPos(rotateButtonRect().topLeft());
}


void UBGraphicsProtractor::paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget)
{
    painter->save();

    Q_UNUSED(styleOption);
    Q_UNUSED(widget);

    QPen pen_(drawColor());
    pen_.setWidth(0); // Line width = 1 pixel regardless of scale / zoom
    painter->setPen(pen_);
    
    painter->setFont(QFont("Arial", 11));
    painter->setBrush(fillBrush());
    painter->drawPie(QRectF(rect().center().x() - radius(), rect().center().y() - radius(), 2 * radius(), 2 * radius()), mStartAngle * 16, mSpan * 16);
    paintGraduations(painter);
    paintButtons(painter);
    paintAngleMarker(painter);

    painter->restore();
}


QVariant UBGraphicsProtractor::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneChange)
    {
        mCloseSvgItem->setParentItem(this);
        mResizeSvgItem->setParentItem(this);
        mResetSvgItem->setParentItem(this);
        mRotateSvgItem->setParentItem(this);
        mMarkerSvgItem->setParentItem(this);
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}


QRectF UBGraphicsProtractor::boundingRect() const
{
    QPointF center = rect().center();
    qreal centerX = center.x();
    qreal centerY = center.y();

    QRectF bounds = resizeButtonRect().adjusted(centerX, centerY, centerX, centerY);
    bounds = bounds.united(closeButtonRect().adjusted(centerX, centerY, centerX, centerY));
    bounds = bounds.united(resetButtonRect().adjusted(centerX, centerY, centerX, centerY));

    QTransform t;
    t.translate(centerX, centerY);
    t.rotate(-mStartAngle);
    t.translate(-centerX, -centerY);
    bounds = t.mapRect(bounds);

    bounds = bounds.united(QGraphicsEllipseItem::boundingRect());

    return bounds;
}


QPainterPath UBGraphicsProtractor::shape() const
{
    QPainterPath path = QGraphicsEllipseItem::shape();
    QPainterPath buttonPath;
    QRectF markerRect = markerButtonRect();

    QPointF center = rect().center();
    qreal centerX = center.x();
    qreal centerY = center.y();

    buttonPath.addRect(resizeButtonRect().adjusted(centerX, centerY, centerX, centerY));
    if (!resizeButtonRect().contains(markerRect))
    {
        buttonPath.addRect(markerRect.adjusted(centerX - markerRect.left() * 2 - markerRect.width(), centerY
                                               , centerX - markerRect.left() * 2 - markerRect.width(), centerY));
        buttonPath.addRect(markerRect.adjusted(centerX, centerY, centerX, centerY));
    }
    buttonPath.addRect(closeButtonRect().adjusted(centerX, centerY, centerX, centerY));
    buttonPath.addRect(resetButtonRect().adjusted(centerX, centerY, centerX, centerY));
    QTransform t;
    t.translate(centerX, centerY);
    t.rotate(-mStartAngle);
    t.translate(-centerX, -centerY);
    buttonPath = t.map(buttonPath);
    buttonPath = buttonPath.subtracted(path);
    path.addPath(buttonPath);

    return path;
}


void UBGraphicsProtractor::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mPreviousMousePos = event->pos();
    mCurrentTool = toolFromPos(event->pos());
    mShowButtons = mCurrentTool == Reset || mCurrentTool == Close;

    if (mCurrentTool == None || mCurrentTool == Move)
        QGraphicsEllipseItem::mousePressEvent(event);
    else
        event->accept();
}

void UBGraphicsProtractor::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF currentPoint = event->pos();
    QLineF startLine(rect().center(), mPreviousMousePos);
    QLineF currentLine(rect().center(), currentPoint);
    qreal angle = startLine.angleTo(currentLine);
    qreal scaleFactor = currentLine.length()/startLine.length();


    switch (mCurrentTool)
    {
    case Rotate :
        prepareGeometryChange();
        mStartAngle = mStartAngle + angle;
        setStartAngle(mStartAngle * 16);
        mPreviousMousePos = currentPoint;

        break;


    case Resize :
        if (radius() * mScaleFactor * scaleFactor > minRadius) {
            prepareGeometryChange();
            setTransform(QTransform::fromTranslate(rect().center().x(), rect().center().y()), true);
            setTransform(QTransform::fromScale(scaleFactor, scaleFactor), true);
            setTransform(QTransform::fromTranslate(-rect().center().x(), -rect().center().y()), true);
            mScaleFactor *= scaleFactor;
        }
        break;

    case MoveMarker :

        mCurrentAngle += angle;
        if ((int)mCurrentAngle % 360 > 270)
            mCurrentAngle = 0;
        else if ((int)mCurrentAngle % 360 >= 180)
            mCurrentAngle = 180;

        mPreviousMousePos = currentPoint;
        update();
        break;

    case Move :
        QGraphicsEllipseItem::mouseMoveEvent(event);
        break;

    default :
        break;
    }

    if (mCurrentTool != Move)
        event->accept();

}


void UBGraphicsProtractor::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    switch (mCurrentTool)
    {
    case Reset :
        setStartAngle(0);
        mStartAngle = 0;
        break;

    case Close :
        hide();
        break;

    case MoveMarker :
        update();
        break;

    case Resize:
        update();
        break;

    default :
        QGraphicsEllipseItem::mouseReleaseEvent(event);
        break;
    }

    if (mCurrentTool != Move)
        event->accept();

    if (scene())
        scene()->setModified(true);

    if (!mShowButtons)
    {
        mShowButtons = true;
        update();
    }

    mCurrentTool = None;
}


void UBGraphicsProtractor::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector && UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Play)
        return;

    if (!mShowButtons)
    {
        mShowButtons = true;

        mCloseSvgItem->setParentItem(this);
        mResizeSvgItem->setParentItem(this);
        mResetSvgItem->setParentItem(this);
        mRotateSvgItem->setParentItem(this);
        mMarkerSvgItem->setParentItem(this);

        update();
    }

    event->accept();
}


void UBGraphicsProtractor::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mShowButtons = false;
    unsetCursor();
    update();
    event->accept();
}


void UBGraphicsProtractor::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector)
        return;

    Tool currentTool = toolFromPos(event->pos());

    if (currentTool == Move)
        setCursor(Qt::SizeAllCursor);
    else if (currentTool == Rotate)
        setCursor(rotateCursor());
    else
        setCursor(Qt::ArrowCursor);

    event->accept();
}


qreal UBGraphicsProtractor::antiScale() const
{
    return 1 / (mScaleFactor * UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
}


QRectF UBGraphicsProtractor::resetButtonRect () const
{
    qreal refWidth = buttonSizeReference().width();
    qreal width = mResetSvgItem->boundingRect().width();
    qreal height = mResetSvgItem->boundingRect().height();

    return QRectF(-refWidth * 7, -height/2.0, width, height);

    // Note: these hardcoded position values make it impossible
    // to use setScale() on the buttons without affecting their
    // relative position on the tool.
}


QRectF UBGraphicsProtractor::closeButtonRect () const
{
    qreal refWidth = buttonSizeReference().width();
    qreal width = mCloseSvgItem->boundingRect().width();
    qreal height = mCloseSvgItem->boundingRect().height();

    return QRectF(-refWidth * 9, -height/2.0, width, height);
}


QRectF UBGraphicsProtractor::resizeButtonRect () const
{
    qreal refWidth = buttonSizeReference().width();
    qreal width = mResizeSvgItem->boundingRect().width();
    qreal height = mResizeSvgItem->boundingRect().height();

    return QRectF(refWidth * 8, -height/2.0, width, height);
}


QRectF UBGraphicsProtractor::rotateButtonRect () const
{
    qreal refWidth = buttonSizeReference().width();
    qreal width = mRotateSvgItem->boundingRect().width();
    qreal height = mRotateSvgItem->boundingRect().height();

    return QRectF(refWidth * 5.5, -refWidth * 5, width, height);


}
QRectF UBGraphicsProtractor::markerButtonRect () const
{
    qreal width = mMarkerSvgItem->boundingRect().width();
    qreal height = mMarkerSvgItem->boundingRect().height();

    return QRectF(radius() + 3, -height/2, width, height);
}

void UBGraphicsProtractor::paintGraduations(QPainter *painter)
{
    painter->save();

    const int  tenDegreeGraduationLength = 22;
    const int fiveDegreeGraduationLength = 15;
    const int  oneDegreeGraduationLength = 7;

    QFont font1 = painter->font();

#ifdef Q_OS_OSX
    font1.setPointSizeF(font1.pointSizeF() + 3);
    font1.setWeight(QFont::Thin);
#endif
    QFontMetricsF fm1(font1);

    //Font for internal arc
    QFont font2 = painter->font();
    font2.setPointSizeF(font1.pointSizeF()/1.5);
    QFontMetricsF fm2(font2);

    qreal rad = radius();

    QPointF center = rect().center();
    painter->drawArc(QRectF(center.x() - rad/2, center.y() - rad/2, rad, rad), mStartAngle*16, mSpan*16);

    for (int angle = 1; angle < mSpan; angle++)
    {
        int graduationLength = (0 == angle % 10) ? tenDegreeGraduationLength : ((0 == angle % 5) ? fiveDegreeGraduationLength : oneDegreeGraduationLength);
        qreal co = cos(((qreal)angle + mStartAngle) * PI/180);
        qreal si = sin(((qreal)angle + mStartAngle) * PI/180);
        if (0 == angle % 90)
            painter->drawLine(QLineF(QPointF(center.x(), center.y()), 
                        QPointF(center.x() + co*tenDegreeGraduationLength, center.y() - si*tenDegreeGraduationLength)));

        //external arc
        painter->drawLine(QLineF(QPointF(center.x()+ rad*co, center.y() - rad*si),
                                 QPointF(center.x()+ (rad - graduationLength)*co, center.y() - (rad - graduationLength)*si)));
        //internal arc
        painter->drawLine(QLineF(QPointF(center.x()+ rad/2*co, center.y() - rad/2*si),
                                 QPointF(center.x()+ (rad/2 + graduationLength)*co,
                                         center.y() - (rad/2 + graduationLength)*si)));

        if (0 == angle % 10)
        {
            //external arc
            painter->setFont(font1);
            QString grad = QString("%1").arg((int)(angle));
            QString grad2 = QString("%1").arg((int)(mSpan - angle));

            painter->drawText(QRectF(center.x() + (rad - graduationLength*1.5)*co  - fm1.horizontalAdvance(grad)/2,
                                     center.y() - (rad - graduationLength*1.5)*si - fm1.height()/2,
                                     fm1.horizontalAdvance(grad), fm1.height()), Qt::AlignTop, grad);

            //internal arc
            painter->setFont(font2);
            painter->drawText(QRectF(center.x() + (rad/2 + graduationLength*1.5)*co  - fm2.horizontalAdvance(grad2)/2,
                                     center.y() - (rad/2 + graduationLength*1.5)*si - fm2.height()/2,
                                     fm2.horizontalAdvance(grad2), fm2.height()), Qt::AlignTop, grad2);
            painter->setFont(font1);

        }
    }

    painter->restore();
}


void UBGraphicsProtractor::paintButtons(QPainter *painter)
{
    Q_UNUSED(painter);

    if (mShowButtons)
    {        
        // The buttons (close, reset, resize & rotate) are rotated around the center
        // of the protractor, to follow it as it is rotated.
        // As coordinates are local to each QGraphicsItem, the position of the rotation 
        // center relative to each button is the inverse of that button's position.

        mCloseSvgItem->setTransformOriginPoint(closeButtonRect().topLeft() * -1); 
        mCloseSvgItem->setRotation(rotation() - mStartAngle);

        mResetSvgItem->setTransformOriginPoint(resetButtonRect().topLeft() * -1);
        mResetSvgItem->setRotation(rotation() - mStartAngle);

        mResizeSvgItem->setTransformOriginPoint(resizeButtonRect().topLeft() * -1);
        mResizeSvgItem->setRotation(rotation() - mStartAngle);

        mRotateSvgItem->setTransformOriginPoint(rotateButtonRect().topLeft() * -1);
        mRotateSvgItem->setRotation(rotation() - mStartAngle);
    }

    mMarkerSvgItem->setTransformOriginPoint(markerButtonRect().topLeft() * -1);
    mMarkerSvgItem->setRotation(rotation() - mStartAngle - mCurrentAngle);

    mCloseSvgItem->setVisible(mShowButtons);
    mResetSvgItem->setVisible(mShowButtons);
    mResizeSvgItem->setVisible(mShowButtons);
    mRotateSvgItem->setVisible(mShowButtons);
    mMarkerSvgItem->setVisible(true);
}


void UBGraphicsProtractor::paintAngleMarker(QPainter *painter)
{
    painter->save();
    painter->translate(rect().center());
    painter->rotate(-mStartAngle);
    painter->translate(-rect().center().x(), -rect().center().y());
    qreal co = cos(mCurrentAngle * PI/180);
    qreal si = sin(mCurrentAngle * PI/180);
    qreal rad = radius();

    painter->drawLine(QLineF(rect().center(), QPointF(rect().center().x()+ (rad+ 20)*co, rect().center().y() - (rad + 20)*si)));
    QPointF center = rect().center();
    painter->drawArc(QRectF(center.x() - rad/8, center.y() - rad/8, rad / 4, rad / 4), 0
                     , (mCurrentAngle - (int)(mCurrentAngle/360)*360)*16);
    painter->translate(rect().center());
    painter->rotate(-mCurrentAngle);
    painter->translate(-rect().center().x(), -rect().center().y());

    //Paint Angle text (horizontally)

    //restore transformations
    painter->translate(rect().center());
    painter->rotate(mCurrentAngle);
    painter->rotate(mStartAngle);
    painter->translate(-rect().center().x(), -rect().center().y());

    qreal angle = mCurrentAngle - (int)(mCurrentAngle/360)*360;

    if (angle != 0)
    {
        QString ang = QString("%1°").arg(angle,0, 'f', 1);
        QFont font2 = painter->font();
        font2.setBold(true);
        QFontMetricsF fm2(font2);
        painter->setFont(font2);
        if (angle < 50)
            angle = 90;
        else
            angle = angle / 2;

        co = cos((mStartAngle + angle) * PI/180);
        si = sin((mStartAngle + angle) * PI/180);
        painter->drawText(QRectF(rect().center().x() + (rad*2.5/10)*co  - fm2.horizontalAdvance(ang)/2,
                                 rect().center().y() - (rad*2.5/10)*si - fm2.height()/2,
                                 fm2.horizontalAdvance(ang), fm2.height()), Qt::AlignTop, ang);
    }

    painter->restore();
}

UBGraphicsProtractor::Tool UBGraphicsProtractor::toolFromPos(QPointF pos)
{
    pos = pos - rect().center();
    QLineF line(QPointF(0,0), pos);
    QTransform t;
    t.rotate(mStartAngle);
    QPointF p1 = t.map(pos);
    t.rotate(mCurrentAngle);
    QPointF p2 = t.map(pos);


    if (resizeButtonRect().contains(p1.x(),p1.y()))
        return Resize;

    else if (closeButtonRect().contains(p1))
        return Close;

    else if (resetButtonRect().contains(p1))
        return Reset;

    else if (rotateButtonRect().contains(p1))
        return Rotate;

    else if (markerButtonRect().contains(p2))
        return MoveMarker;

    else if (line.length() <= radius())
        return Move;

    else
        return None;
}


UBGraphicsScene* UBGraphicsProtractor::scene() const
{
    return static_cast<UBGraphicsScene*>(QGraphicsEllipseItem::scene());
}

QBrush UBGraphicsProtractor::fillBrush() const
{
    QColor fillColor = edgeFillColor();// scene()->isDarkBackground() ? sDarkBackgroundFillColor : sFillColor;
    QColor fillColorCenter = middleFillColor();//scene()->isDarkBackground() ? sDarkBackgroundFillColorCenter : sFillColorCenter;
    QColor transparentWhite = Qt::white;
    transparentWhite.setAlpha(scene()->isDarkBackground() ? sDrawTransparency : sFillTransparency);
    QRadialGradient radialGradient(rect().center(), radius(), rect().center());
    radialGradient.setColorAt(0, fillColorCenter);
    radialGradient.setColorAt(1, fillColor);
    return radialGradient;
}


UBItem* UBGraphicsProtractor::deepCopy() const
{
    UBGraphicsProtractor* copy = new UBGraphicsProtractor();

    copyItemParameters(copy);

    // TODO UB 4.7 ... complete all members ?

    return copy;
}
void UBGraphicsProtractor::copyItemParameters(UBItem *copy) const
{
    UBGraphicsProtractor *cp = dynamic_cast<UBGraphicsProtractor*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setRect(this->rect());
        cp->setTransform(this->transform());

        cp->mCurrentAngle = this->mCurrentAngle;
        cp->mSpan = this->mSpan;
        cp->mStartAngle = this->mStartAngle;
        cp->mScaleFactor = this->mScaleFactor;
    }
}

void UBGraphicsProtractor::rotateAroundCenter(qreal angle)
{
    // Q_UNUSED(angle);

    // Align the implementation with the others apps objects.
    QTransform transform;
    transform.translate(rotationCenter().x(), rotationCenter().y());
    transform.rotate(angle);
    transform.translate(- rotationCenter().x(), - rotationCenter().y());
    setTransform(transform, true);

}

QPointF UBGraphicsProtractor::rotationCenter() const
{
    return QPointF(rect().x(), rect().y());

}
