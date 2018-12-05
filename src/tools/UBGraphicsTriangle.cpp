/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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




#include <QPolygonF>
#include <QtWidgets/QGraphicsPolygonItem>

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
    : UBAbstractDrawRuler()
    , QGraphicsPolygonItem()
    , angle(0)
    , mResizing1(false)
    , mResizing2(false)
    , mRotating(false)
    , mShouldPaintInnerTriangle(true)

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

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::CppTool)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemIsSelectable, false);

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

    copyItemParameters(copy);

    // TODO UB 4.7 ... complete all members ?

    return copy;

}

void UBGraphicsTriangle::copyItemParameters(UBItem *copy) const
{
    UBGraphicsTriangle* cp = dynamic_cast<UBGraphicsTriangle*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setPolygon(this->polygon());
        cp->setTransform(this->transform());
    }
}

void UBGraphicsTriangle::setRect(qreal x, qreal y, qreal w, qreal h, UBGraphicsTriangleOrientation orientation)
{
    QPolygonF polygon;
    polygon << QPointF(x, y) << QPoint(x, y + h) << QPoint(x+w, y + h);
    setPolygon(polygon);

    //  Save the bounds rect
    bounds_rect.setX(x);  bounds_rect.setY(y);  bounds_rect.setWidth(w); bounds_rect.setHeight(h);

    setOrientation(orientation);
}

void UBGraphicsTriangle::setOrientation(UBGraphicsTriangleOrientation orientation)
{
    mOrientation = orientation;

    calculatePoints(bounds_rect);

    QPolygonF polygon;
    polygon << A1 << B1 << C1;
    setPolygon(polygon);
}


QRectF UBGraphicsTriangle::bounding_Rect() const
{

    QRectF bounds = QRectF(0,0,0,0);
    bounds.setX(bounds_rect.x());
    bounds.setY(bounds_rect.y());
    bounds.setWidth(bounds_rect.width());
    bounds.setHeight(bounds_rect.height());


    return bounds;
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

    bool paintInnerTriangle = true;
    switch(mOrientation)
    {
        case BottomLeft:
            if (B2.x() > C2.x() || B2.y() < A2.y())
                paintInnerTriangle = false;
            break;
        case TopLeft:
            if (B2.x() > C2.x() || B2.y() > A2.y())
                paintInnerTriangle = false;
            break;
        case TopRight:
            if (B2.x() < C2.x() || B2.y() > A2.y())
                paintInnerTriangle = false;
            break;
        case BottomRight:
            if (B2.x() < C2.x() || B2.y() < A2.y())
                paintInnerTriangle = false;
            break;
    }
    mShouldPaintInnerTriangle = paintInnerTriangle;

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

    if (mShouldPaintInnerTriangle) {
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
    }

    else {
        QLinearGradient gradient(QPointF(A1.x(), 0), QPointF(C1.x(), 0));
        gradient.setColorAt(0, edgeFillColor());
        gradient.setColorAt(1, middleFillColor());
        painter->setBrush(gradient);
        painter->setPen(drawColor());
        polygon << A1 << B1 << C1;
        painter->drawPolygon(polygon);
        polygon.clear();
    }


    paintGraduations(painter);

    mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
    QTransform antiScaleTransform;
    antiScaleTransform.scale(mAntiScaleRatio, mAntiScaleRatio);

    mCloseSvgItem->setTransform(antiScaleTransform);
    mHFlipSvgItem->setTransform(antiScaleTransform);
    mVFlipSvgItem->setTransform(antiScaleTransform);
    mRotateSvgItem->setTransform(antiScaleTransform);

    mCloseSvgItem->setPos(closeButtonRect().topLeft());

    //qDebug() << "UBGraphicsTriangle Paint"<<"closeButtonRect().topLeft()="
    //<<closeButtonRect().topLeft();

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

QPainterPath UBGraphicsTriangle::shape() const
{
    QPainterPath tShape;
    QPolygonF tPolygon;

    //qDebug() << "UBGraphicsTriangle shape()"<<"A1 ="<<A1<<"B1 ="<<B1<<"C1 ="<<C1;

    tPolygon << A1 << B1 << C1;
    tShape.addPolygon(tPolygon);
    tPolygon.clear();

    if (mShouldPaintInnerTriangle) {
        tPolygon << A2 << B2 << C2;
        tShape.addPolygon(tPolygon);
        tPolygon.clear();
    }

    //qDebug() << "UBGraphicsTriangle shape()"<<"A1 ="<<A1<<"B1 ="<<B1<<"C1 ="<<C1;
    //qDebug() << "UBGraphicsTriangle shape()"<<"A2 ="<<A2<<"B2 ="<<B2<<"C2 ="<<C2;

    return tShape;
}

void UBGraphicsTriangle::paintGraduations(QPainter *painter)
{
    const int SEPARATOR = 5;
    qreal kx = (mOrientation == TopLeft || mOrientation == BottomLeft) ? 1 : -1;
    qreal ky = (mOrientation == BottomLeft || mOrientation == BottomRight) ? 1 : -1;

    painter->save();
    painter->setFont(font());
    QFontMetricsF fontMetrics(painter->font());

    // Update the width of one "centimeter" to correspond to the width of the background grid (whether it is displayed or not)
    sPixelsPerCentimeter = UBApplication::boardController->activeScene()->backgroundGridSize();
    double pixelsPerMillimeter = sPixelsPerCentimeter/10.0;

    // When a "centimeter" is too narrow, we only display every 5th number, and every 5th millimeter mark
    double numbersWidth = fontMetrics.width("00");
    bool shouldDisplayAllNumbers = (numbersWidth <= (sPixelsPerCentimeter - 5));

    for (int millimeters = 0; millimeters < (rect().width() - sLeftEdgeMargin - sRoundingRadius) / pixelsPerMillimeter; millimeters++)
    {
        double graduationX = rotationCenter().x() + kx * pixelsPerMillimeter * millimeters;
        double graduationHeight = 0;

        if (millimeters % UBGeometryUtils::millimetersPerCentimeter == 0)
            graduationHeight = UBGeometryUtils::centimeterGraduationHeight;

        else if (millimeters % UBGeometryUtils::millimetersPerHalfCentimeter == 0)
            graduationHeight = UBGeometryUtils::halfCentimeterGraduationHeight;

        else
            graduationHeight = UBGeometryUtils::millimeterGraduationHeight;

        qreal requiredSpace = graduationHeight + SEPARATOR ;
        /*     B____C
                |  /
               D|_/E <-- availableSpace Thalès
                |/
                A
        */

        qreal AD;
        switch(mOrientation)
        {
            case BottomLeft:
                AD = QLineF(rect().bottomRight(), QPointF(graduationX, rotationCenter().y())).length();
                break;
            case TopLeft:
                AD = QLineF(rect().topRight(), QPointF(graduationX, rotationCenter().y())).length();
                break;
            case TopRight:
                AD = QLineF(rect().topLeft(), QPointF(graduationX, rotationCenter().y())).length();
                break;
            case BottomRight:
                AD = QLineF(rect().bottomLeft(), QPointF(graduationX , rotationCenter().y())).length();
                break;
        }

        qreal AB = rect().width();
        qreal BC = rect().height();
        qreal DE =  AD * BC / AB, availableSpace = DE;

        if (availableSpace < requiredSpace)
            break;

        if (shouldDisplayAllNumbers || millimeters % UBGeometryUtils::millimetersPerHalfCentimeter == 0)
            painter->drawLine(QLineF(graduationX, rotationCenter().y(), graduationX, rotationCenter().y() - ky * graduationHeight));

        if ((shouldDisplayAllNumbers && millimeters % UBGeometryUtils::millimetersPerCentimeter == 0)
            || millimeters % (UBGeometryUtils::millimetersPerCentimeter*5) == 0)
        {
            QString text = QString("%1").arg((int)(millimeters / UBGeometryUtils::millimetersPerCentimeter));
            qreal textWidth = fontMetrics.width(text);
            qreal textHeight = fontMetrics.tightBoundingRect(text).height();

            requiredSpace = graduationHeight + textHeight + textWidth + SEPARATOR ;

            if (requiredSpace <= availableSpace)
            {
                int textY = (ky > 0) ? rotationCenter().y() - SEPARATOR - UBGeometryUtils::centimeterGraduationHeight - textHeight
                    : rotationCenter().y() + SEPARATOR + UBGeometryUtils::centimeterGraduationHeight;
                painter->drawText(QRectF(graduationX - textWidth / 2, textY, textWidth, textHeight),Qt::AlignVCenter, text);
            }
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


QPointF    UBGraphicsTriangle::rotationCenter() const
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

QRectF    UBGraphicsTriangle::closeButtonRect() const
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

QRectF    UBGraphicsTriangle::hFlipRect() const
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

QRectF    UBGraphicsTriangle::vFlipRect() const
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

QRectF    UBGraphicsTriangle::rotateRect() const
{
    QPointF p(C2);
    qreal buttonsX = vFlipRect().left();

    switch(mOrientation)
    {
        case BottomLeft:
            p = QPointF(qMax(p.x() + 20, buttonsX), p.y() + 5);
            break;
        case TopLeft:
            p = QPointF(qMax(p.x() + 20, buttonsX), p.y() -5 - mRotateSvgItem->boundingRect().height());
            break;
        case TopRight:
            p = QPointF(qMin(p.x() -20 - mRotateSvgItem->boundingRect().width(), buttonsX), p.y() - 5 - mRotateSvgItem->boundingRect().height());
            break;
        case BottomRight:
            p = QPointF(qMin(p.x() -20 - mRotateSvgItem->boundingRect().width(), buttonsX), p.y() + 5);
            break;
    }
    return QRectF(p, QSizeF(mRotateSvgItem->boundingRect().size()));
}

QCursor    UBGraphicsTriangle::resizeCursor1() const
{
    return mResizeCursor1;
}

QCursor    UBGraphicsTriangle::resizeCursor2() const
{
    return mResizeCursor2;
}

QCursor    UBGraphicsTriangle::flipCursor() const
{
    return Qt::ArrowCursor;
}


void UBGraphicsTriangle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    lastRect = rect().toRect();
    lastPos = sceneTransform().inverted().map(event->screenPos());

    if (resize1Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
    {
        mResizing1 = true;
        event->accept();
    }
    else
    if (resize2Polygon().containsPoint(event->pos().toPoint(), Qt::OddEvenFill))
    {
        mResizing2 = true;
        event->accept();
    }
    else
    if(rotateRect().contains(event->pos()))
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
        QPoint currPos = sceneTransform().inverted().map(event->screenPos());

        if (mResizing1)
        {
            if (mOrientation == TopLeft || mOrientation == BottomLeft)
            {
                int deltaX = currPos.x() - lastPos.x();
                if (lastRect.width() + deltaX < sMinWidth)
                    deltaX = sMinWidth - lastRect.width();

                setRect(QRectF(lastRect.left(), lastRect.top(),
                    lastRect.width() + deltaX, lastRect.height()), mOrientation);
            }
            else
            {
                int deltaX = lastPos.x() - currPos.x();
                if (lastRect.width() + deltaX < sMinWidth)
                    deltaX = sMinWidth - lastRect.width();

                setRect(QRectF(lastRect.left() - deltaX, lastRect.top(),
                    lastRect.width() + deltaX, lastRect.height()), mOrientation);
            }
        }

        //-----------------------------------------------//

        if (mResizing2)
        {
            if (mOrientation == BottomRight || mOrientation == BottomLeft)
            {
                int deltaY = lastPos.y() - currPos.y();
                if (lastRect.height() + deltaY < sMinHeight)
                    deltaY = sMinHeight - lastRect.height();

                setRect(QRectF(lastRect.left(), lastRect.top() - deltaY,
                        lastRect.width(), lastRect.height() + deltaY), mOrientation);
            }
            else
            {
                int deltaY = currPos.y() - lastPos.y();
                if (lastRect.height() + deltaY < sMinHeight)
                    deltaY = sMinHeight - lastRect.height();

                setRect(QRectF(lastRect.left(), lastRect.top(),
                        lastRect.width(), lastRect.height() + deltaY), mOrientation);
            }

        }

        //-----------------------------------------------//

        if (mRotating)
        {
            QLineF currentLine(rotationCenter(), event->pos());
            QLineF lastLine(rotationCenter(), event->lastPos());
            rotateAroundCenter(currentLine.angleTo(lastLine));
        }

        //-----------------------------------------------//

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

    if (currentTool == UBStylusTool::Selector ||
        currentTool == UBStylusTool::Play)
    {
        mCloseSvgItem->setParentItem(this);

        mShowButtons = true;
        mCloseSvgItem->setVisible(true);
        mHFlipSvgItem->setVisible(contains(hFlipRect()));
        mVFlipSvgItem->setVisible(contains(vFlipRect()));
        mRotateSvgItem->setVisible(contains(rotateRect()));

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

    } else if (UBDrawingController::drawingController()->isDrawingTool())  {
            setCursor(drawRulerLineCursor());
            UBDrawingController::drawingController()->mActiveRuler = this;
            event->accept();
    }

    //
    //event->accept(); //**
    //update(); //**
}

void UBGraphicsTriangle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mResizing1 = mResizing2 = mRotating = false;
    mShowButtons = false;
    setCursor(Qt::ArrowCursor);
    mCloseSvgItem->setVisible(false);
    mVFlipSvgItem->setVisible(false);
    mHFlipSvgItem->setVisible(false);
    mRotateSvgItem->setVisible(false);
    UBDrawingController::drawingController()->mActiveRuler = NULL;
    event->accept();
    update();
}

void UBGraphicsTriangle::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

    if (currentTool == UBStylusTool::Selector ||
        currentTool == UBStylusTool::Play)
    {
        mCloseSvgItem->setVisible(mShowButtons);
        mVFlipSvgItem->setVisible(mShowButtons && contains(vFlipRect()));
        mHFlipSvgItem->setVisible(mShowButtons && contains(hFlipRect()));
        mRotateSvgItem->setVisible(mShowButtons && contains(rotateRect()));

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
    }  else if (UBDrawingController::drawingController()->isDrawingTool())  {
        event->accept();
    }
}
void UBGraphicsTriangle::StartLine(const QPointF &scenePos, qreal width)
{
    Q_UNUSED(width);
    QPointF itemPos = mapFromScene(scenePos);
    mStrokeWidth = UBDrawingController::drawingController()->currentToolWidth();

    qreal y;

    if (mOrientation == 0 || mOrientation == 1) {
        y = rect().y() + rect().height() + mStrokeWidth / 2;
    } else if (mOrientation == 2 || mOrientation == 3) {
        y = rect().y() - mStrokeWidth / 2;
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

void UBGraphicsTriangle::DrawLine(const QPointF &scenePos, qreal width)
{
    Q_UNUSED(width);
    QPointF itemPos = mapFromScene(scenePos);

    qreal y;

    if (mOrientation == 0 || mOrientation == 1) {
        y = rect().y() + rect().height() + mStrokeWidth / 2;
    } else if (mOrientation == 2 || mOrientation == 3) {
        y = rect().y() - mStrokeWidth / 2;
    }

    if (itemPos.x() < rect().x() + sLeftEdgeMargin)
            itemPos.setX(rect().x() + sLeftEdgeMargin);
    if (itemPos.x() > rect().x() + rect().width() - sLeftEdgeMargin)
            itemPos.setX(rect().x() + rect().width() - sLeftEdgeMargin);

    itemPos.setY(y);
    itemPos = mapToScene(itemPos);

    // We have to use "pointed" line for marker tool
    scene()->drawLineTo(itemPos, mStrokeWidth,
            UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Marker);
}

/**
 * @brief Check whether a given QRectF is inside the triangle (A1, B1, C1).
 *
 * Returns true if any corner of the rectangle is within the triangle or, if strict is set to true,
 * if all corners of the rectangle are within the triangle.
 */
bool UBGraphicsTriangle::contains(const QRectF &rect, bool strict)
{
    QPolygonF poly;
    poly << A1 << B1 << C1 << A1;

    QPainterPath path;
    path.addPolygon(poly);

    QList<QPointF> points;
    points << rect.bottomRight() << rect.topLeft() << rect.topRight();

    bool inside = path.contains(rect.bottomLeft());

    foreach(QPointF p, points) {
        if (strict)
            inside = inside && path.contains(p);
        else
            inside = inside || path.contains(p);
    }

    return inside;
}

void UBGraphicsTriangle::EndLine()
{
}

