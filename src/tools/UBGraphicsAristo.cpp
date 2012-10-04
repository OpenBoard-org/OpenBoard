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

#include "UBGraphicsAristo.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "domain/UBGraphicsScene.h"

#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsItem>
#include <QLineF>
#include <QPolygonF>
#include <QRadialGradient>
#include <QString>

#include "core/memcheck.h"

const QRectF UBGraphicsAristo::sDefaultRect =  QRectF(0, 0, 800, 500);
const UBGraphicsAristo::Orientation UBGraphicsAristo::sDefaultOrientation = UBGraphicsAristo::Top;

UBGraphicsAristo::UBGraphicsAristo()
    : UBAbstractDrawRuler()
    , QGraphicsPathItem()
    , mMarking(false)
    , mResizing(false)
    , mRotating(false)
    , mOrientation(Undefined)
    , mRotatedAngle(0)
    , mMarkerAngle(0)
    , mStartAngle(0)
    , mSpan(180)
    , mHFlipSvgItem(0)
    , mMarkerSvgItem(0)
    , mResizeSvgItem(0)
    , mRotateSvgItem(0)
{
    mHFlipSvgItem = new QGraphicsSvgItem(":/images/vflipTool.svg", this);
    mHFlipSvgItem->setVisible(false);
    mHFlipSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mResizeSvgItem = new QGraphicsSvgItem(":/images/resizeTool.svg", this);
    mResizeSvgItem->setVisible(false);
    mResizeSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mRotateSvgItem = new QGraphicsSvgItem(":/images/rotateTool.svg", this);
    mRotateSvgItem->setVisible(false);
    mRotateSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mMarkerSvgItem = new QGraphicsSvgItem(":/images/angleMarker.svg", this);
    mMarkerSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));
    mMarkerSvgItem->setVisible(true);

    create(*this);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::CppTool)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    setOrientation(sDefaultOrientation);
}

UBGraphicsAristo::~UBGraphicsAristo()
{
    /* NOOP */
}

/*
 * setOrientation() modify the tool orientation.
 * Apexes coordinates are alors recomputed.
 */
void UBGraphicsAristo::setOrientation(Orientation orientation)
{
    /* substracting difference to zero [2pi] twice, to obtain the desired angle */
    mMarkerAngle -= 2 * (mMarkerAngle - (int)(mMarkerAngle/360)*360) - 360;

    bool wasUndefined = mOrientation == Undefined;
    mOrientation = orientation;
    
    if (wasUndefined)
        calculatePoints(sDefaultRect);
    else
        calculatePoints(boundingRect());
}

/* calculatePoints() is used to calculate polygon's apexes coordinates.
 * This function handles orientation changes too.
 * Items are repositionated and path is redeterminates.
 */
void UBGraphicsAristo::calculatePoints(QRectF bounds)
{
    switch (mOrientation) {
    case Bottom:
        C.setX(bounds.center().x());
        C.setY(bounds.bottom());

        A.setX(bounds.left());
        A.setY(bounds.bottom() - bounds.width() / 2);

        B.setX(bounds.right());
        B.setY(bounds.bottom() - bounds.width() / 2);
        break;
    case Top:
        C.setX(bounds.center().x());
        C.setY(bounds.top());

        A.setX(bounds.left());
        A.setY(bounds.top() + bounds.width() / 2);

        B.setX(bounds.right());
        B.setY(bounds.top() + bounds.width() / 2);
        break;
    default:
        break;
    }

    setItemsPos();
    setPath(determinePath());
}

/*
 * setItemsPos() places control items according to A, B and C positions.
 * Call this function after A, B or C position modification, mostly after calling calculatePoints().
 * These positions has to be set when calling setPath() to allow hover events on items which are not into the main polygon.
 */
void UBGraphicsAristo::setItemsPos()
{
    mCloseSvgItem->setPos(closeButtonRect().topLeft() + rotationCenter());
    mHFlipSvgItem->setPos(hFlipRect().topLeft() + rotationCenter());
    mRotateSvgItem->setPos(rotateRect().topLeft() + rotationCenter());
    mResizeSvgItem->setPos(resizeButtonRect().topLeft() + rotationCenter()); 
    mMarkerSvgItem->setPos(markerButtonRect().topLeft() + rotationCenter());
}

/*
 * determinePath() modify the shape according to apexes coordinates and control item positions.
 * This is useful when orientation is modified.
 * Returns the painter path corresponding to object parameters.
 */
QPainterPath UBGraphicsAristo::determinePath()
{
    QPainterPath path;

    QPolygonF polygon;
    polygon << A << B << C;

    qreal rotationAngle = mOrientation == Bottom ? mMarkerAngle : mMarkerAngle - 360 * (int)(mMarkerAngle / 360);
    QTransform t;
    t.rotate(rotationAngle);
    bool markerIsInPolygon = true;
    if ((mOrientation == Bottom && mMarkerAngle < 90) || (mOrientation == Top && mMarkerAngle < 270))
        markerIsInPolygon = polygon.containsPoint(t.map(markerButtonRect().topLeft()) + rotationCenter(), Qt::OddEvenFill);
    else
        markerIsInPolygon = polygon.containsPoint(t.map(markerButtonRect().bottomLeft()) + rotationCenter(), Qt::OddEvenFill);

    path.moveTo(A);

    QRectF mappedRect = t.mapRect(markerButtonRect());
    path.lineTo(QPointF(mappedRect.translated(rotationCenter()).left(), A.y()));
    if (!markerIsInPolygon) {
        if (mOrientation == Top) {
            path.lineTo(QPointF(mappedRect.translated(rotationCenter()).left(), mappedRect.translated(rotationCenter()).bottom()));
            path.lineTo(QPointF(mappedRect.translated(rotationCenter()).right(), mappedRect.translated(rotationCenter()).bottom()));
        }
        else if (mOrientation == Bottom) {
            path.lineTo(QPointF(mappedRect.translated(rotationCenter()).left(), mappedRect.translated(rotationCenter()).top()));
            path.lineTo(QPointF(mappedRect.translated(rotationCenter()).right(), mappedRect.translated(rotationCenter()).top()));
        }
    }
    path.lineTo(QPointF(mappedRect.translated(rotationCenter()).right(), A.y()));

    path.lineTo(QPointF(resizeButtonRect().translated(rotationCenter()).left(), A.y()));
    if (mOrientation == Top) {
        path.lineTo(QPointF(resizeButtonRect().translated(rotationCenter()).left(), resizeButtonRect().translated(rotationCenter()).bottom()));
        path.lineTo(QPointF(resizeButtonRect().translated(rotationCenter()).right(), resizeButtonRect().translated(rotationCenter()).bottom()));
    }
    else if (mOrientation == Bottom) {
        path.lineTo(QPointF(resizeButtonRect().translated(rotationCenter()).left(), resizeButtonRect().translated(rotationCenter()).top()));
        path.lineTo(QPointF(resizeButtonRect().translated(rotationCenter()).right(), resizeButtonRect().translated(rotationCenter()).top()));
    }
    path.lineTo(QPointF(resizeButtonRect().translated(rotationCenter()).right(), A.y()));

    path.lineTo(B);
    path.lineTo(C);
    path.lineTo(A);

    return path;
}

UBItem* UBGraphicsAristo::deepCopy(void) const
{
    UBGraphicsAristo* copy = new UBGraphicsAristo();
    copyItemParameters(copy);
    return copy;
}

void UBGraphicsAristo::copyItemParameters(UBItem *copy) const
{
    UBGraphicsAristo* cp = dynamic_cast<UBGraphicsAristo*>(copy);
    if (cp)
    {   
        /* TODO: copy all members */
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setOrientation(mOrientation);
        cp->mRotatedAngle = mRotatedAngle;
        cp->mMarkerAngle = mMarkerAngle;
    }
}


void UBGraphicsAristo::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPolygonF polygon;

    painter->setBrush(fillBrush());
    painter->setPen(drawColor());

    polygon << A << B << C;
    painter->drawPolygon(polygon);
    polygon.clear();

    paintGraduations(painter);
    paintMarker(painter);    
}

QBrush UBGraphicsAristo::fillBrush() const
{
    QColor fillColor = edgeFillColor();// scene()->isDarkBackground() ? sDarkBackgroundFillColor : sFillColor;
    QColor fillColorCenter = middleFillColor();//scene()->isDarkBackground() ? sDarkBackgroundFillColorCenter : sFillColorCenter;
    QColor transparentWhite = Qt::white;
    transparentWhite.setAlpha(scene()->isDarkBackground() ? sDrawTransparency : sFillTransparency);
    QRadialGradient radialGradient(boundingRect().center(), radius(), boundingRect().center());
    radialGradient.setColorAt(0, fillColorCenter);
    radialGradient.setColorAt(1, fillColor);
    return radialGradient;
}

/* paintGraduations() paints graduations on the ruler side (length graduations) and the two other sides (angle graduation) */
void UBGraphicsAristo::paintGraduations(QPainter *painter)
{
    paintRulerGraduations(painter);
    paintProtractorGraduations(painter);
}

void UBGraphicsAristo::paintRulerGraduations(QPainter *painter)
{
    /* defining useful constants */
    const int centimeterGraduationHeight = 15;
    const int halfCentimeterGraduationHeight = 10;
    const int millimeterGraduationHeight = 5;
    const int millimetersPerCentimeter = 10;
    const int millimetersPerHalfCentimeter = 5;

    painter->save();
    painter->setFont(font());
    QFontMetricsF fontMetrics(painter->font());

    /* Browsing milliters in half width of ruler side */
    for (int millimeters = 0; millimeters < (boundingRect().width() / 2 - sLeftEdgeMargin - sRoundingRadius) / sPixelsPerMillimeter; millimeters++)
    {
        /* defining graduationHeight ; values are different to draw bigger lines if millimiter considered is a centimeter or a half centimeter */
        int graduationHeight = (0 == millimeters % millimetersPerCentimeter) ?
            centimeterGraduationHeight :
            ((0 == millimeters % millimetersPerHalfCentimeter) ?
                halfCentimeterGraduationHeight : millimeterGraduationHeight);

        /* correcting graduationHeight: draw the line in the other direction in case ruler is top-oriented, to stay inside the tool and inside the rect */
        graduationHeight = mOrientation == Bottom ? graduationHeight : - graduationHeight;
        
        /* drawing graduation to the left and to the right of origin, which is the center of graduated side */
        painter->drawLine(QLine(rotationCenter().x() + sPixelsPerMillimeter * millimeters, rotationCenter().y(), rotationCenter().x() + sPixelsPerMillimeter * millimeters, rotationCenter().y() + graduationHeight));
        if (millimeters != 0)
            painter->drawLine(QLine(rotationCenter().x() - sPixelsPerMillimeter * millimeters, rotationCenter().y(), rotationCenter().x() - sPixelsPerMillimeter * millimeters, rotationCenter().y() + graduationHeight));

        /* drawing associated value if considered graduation is a centimeter */
        if (0 == millimeters % millimetersPerCentimeter)
        {
            /* defining graduation value */
            QString text = QString("%1").arg((int)(millimeters / millimetersPerCentimeter));
            
            /* staying inside polygon */
            if (rotationCenter().x() + sPixelsPerMillimeter * millimeters + fontMetrics.width(text) / 2 < boundingRect().right())
            {
                qreal textWidth = fontMetrics.width(text);
                qreal textHeight = fontMetrics.tightBoundingRect(text).height() + 5;
                
                /* text y-coordinate is different according to tool's orientation */
                qreal textY = mOrientation == Bottom ? A.y() + 5 + centimeterGraduationHeight : A.y() - 5 - centimeterGraduationHeight + graduationHeight;
                
                /* if text's rect is not out of polygon's bounds, drawing value below or above graduation */
                QPointF intersectionPoint;
                
                bool paint = false;

                if (mOrientation == Bottom && QLineF(QPointF(rotationCenter().x() - sPixelsPerMillimeter * millimeters - textWidth / 2, rotationCenter().y()), QPointF(rotationCenter().x() - sPixelsPerMillimeter * millimeters - textWidth / 2, textY + textHeight)).intersect(QLineF(A, C), &intersectionPoint) != QLineF::BoundedIntersection && QLineF(QPointF(rotationCenter().x() - sPixelsPerMillimeter * millimeters + textWidth / 2, rotationCenter().y()), QPointF(rotationCenter().x() - sPixelsPerMillimeter * millimeters + textWidth / 2, textY + textHeight)).intersect(QLineF(A, C), &intersectionPoint) != QLineF::BoundedIntersection) {
                    paint = true;
                }
                else if (mOrientation == Top && QLineF(QPointF(rotationCenter().x() - sPixelsPerMillimeter * millimeters - textWidth / 2, rotationCenter().y()), QPointF(rotationCenter().x() - sPixelsPerMillimeter * millimeters - textWidth / 2, textY - textHeight)).intersect(QLineF(A, C), &intersectionPoint) != QLineF::BoundedIntersection && QLineF(QPointF(rotationCenter().x() - sPixelsPerMillimeter * millimeters + textWidth / 2, rotationCenter().y()), QPointF(rotationCenter().x() - sPixelsPerMillimeter * millimeters + textWidth / 2, textY - textHeight)).intersect(QLineF(A, C), &intersectionPoint) != QLineF::BoundedIntersection) {
                    paint = true;
                }

                if (paint) {
                    painter->drawText(
                        QRectF(rotationCenter().x() + sPixelsPerMillimeter * millimeters - textWidth / 2, textY, textWidth, textHeight),
                        Qt::AlignVCenter, text);
                    if (millimeters != 0)
                        painter->drawText(
                        QRectF(rotationCenter().x() - sPixelsPerMillimeter * millimeters - textWidth / 2, textY, textWidth, textHeight),
                        Qt::AlignVCenter, text);
                }
            }
        }
    }
    painter->restore();
}

void UBGraphicsAristo::paintProtractorGraduations(QPainter* painter)
{
    /* defining useful constants */
    const int  tenDegreeGraduationLength = 15;
    const int fiveDegreeGraduationLength = 10;
    const int  oneDegreeGraduationLength = 5;

    painter->save();

    QFont font1 = painter->font();
#ifdef Q_WS_MAC
    font1.setPointSizeF(font1.pointSizeF() - 3);
#endif
    QFontMetricsF fm1(font1);

    //Font for internal arc
    QFont font2 = painter->font();
    font2.setPointSizeF(font1.pointSizeF()/1.5);
    QFontMetricsF fm2(font2);

    /* defining virtual arc diameter */
    qreal rad = radius();

    QPointF center = rotationCenter();

    /* browsing angles */
    for (int angle = 1; angle < mSpan; angle++)
    {
        int graduationLength = (0 == angle % 10) ? tenDegreeGraduationLength : ((0 == angle % 5) ? fiveDegreeGraduationLength : oneDegreeGraduationLength);
        
        qreal co = cos(((qreal)angle + mStartAngle) * PI/180);
        qreal si = sin(((qreal)angle + mStartAngle) * PI/180);

        /* inverse sinus according to the orientation, to draw graduations on the polygon */
        si = mOrientation == Bottom ? -si : si;

        /* drawing the graduation around the virtual arc */
        if (angle >= sArcAngleMargin && angle <= mSpan - sArcAngleMargin)
            painter->drawLine(QLineF(QPointF(center.x()+ rad/2*co, center.y() - rad/2*si),
                                    QPointF(center.x()+ (rad/2 + graduationLength)*co,
                                            center.y() - (rad/2 + graduationLength)*si)));


        QPointF intersectionPoint;
        QLineF referenceLine;
        if (angle < 90)
            referenceLine.setP1(B);
        else
            referenceLine.setP1(A);
        referenceLine.setP2(C);

        /* if angle is 10-multiple, drawing it's value, rotated to be easily red */
        if (0 == angle % 10) {
            QString grad = QString("%1").arg((int)(angle));
            QString grad2 = QString("%1").arg((int)mSpan - angle);

            painter->setFont(font2);

            painter->save();
            painter->translate(center.x() + (rad/2 + graduationLength*1.5)*co, center.y() - (rad/2 + graduationLength*1.5)*si);
            int degrees = mOrientation == Bottom ? angle : -angle;
            painter->rotate(-90 + degrees);
            painter->drawText(- fm2.width(grad)/2, - fm2.height()/2, fm2.width(grad), fm2.height(), Qt::AlignCenter, grad);
            painter->restore();

            painter->setFont(font1);

            
            /* drawing the graduation near tool's side */
            if (QLineF(QPointF(center.x()+ rad/2*co, center.y() - rad/2*si),
                                     QPointF(center.x()+ (rad/2 + graduationLength)*co,
                                             center.y() - (rad/2 + graduationLength)*si)).intersect(referenceLine, &intersectionPoint) == QLineF::UnboundedIntersection)

                painter->drawLine(QLineF(QPointF(center.x() + (rad/2 + graduationLength*1.5 + fm2.width(grad)/2)*co,
                                                 center.y() - (rad/2 + graduationLength*1.5 + fm2.height()/2)*si),
                                        intersectionPoint));

        }

        /* drawing the graduation near tool's side */
        else
            if (QLineF(QPointF(center.x()+ rad/2*co, center.y() - rad/2*si),
                                     QPointF(center.x()+ (rad/2 + graduationLength)*co,
                                             center.y() - (rad/2 + graduationLength)*si)).intersect(referenceLine, &intersectionPoint) == QLineF::UnboundedIntersection)

                painter->drawLine(QLineF(QPointF(intersectionPoint.x() - (graduationLength*1.5)*co,
                                                 intersectionPoint.y() + (graduationLength*1.5)*si),
                                        intersectionPoint));
    }

    painter->restore();
}

/* paintMarker() adjust marker button according to the current angle, draw the line allowing user to set precisely the angle, and draw the current angle's value. */
void UBGraphicsAristo::paintMarker(QPainter *painter)
{
    /* adjusting marker button */
    mMarkerSvgItem->resetTransform();
    mMarkerSvgItem->translate(-markerButtonRect().left(), -markerButtonRect().top());
    mMarkerSvgItem->rotate(mMarkerAngle);
    mMarkerSvgItem->translate(markerButtonRect().left(), markerButtonRect().top());

    
    qreal co = cos((mMarkerAngle) * PI/180);
    qreal si = sin((mMarkerAngle) * PI/180);

    /* Setting point composing the line (from point C) which intersects the line we want to draw. */
    QPointF referencePoint;
    if (mOrientation == Bottom) {
        if ((int)mMarkerAngle % 360 < 90)
            referencePoint = B;
        else
            referencePoint = A;
    }
    else if (mOrientation == Top) {
        if ((int)mMarkerAngle % 360 < 270 && (int)mMarkerAngle % 360 > 0)
            referencePoint = A;
        else
            referencePoint = B;
    }
    
    /* getting intersection point to draw the wanted line */
    QLineF intersectedLine(rotationCenter(), QPointF(rotationCenter().x()+co, rotationCenter().y()+si)); 
    QPointF intersectionPoint;
    if (intersectedLine.intersect(QLineF(referencePoint, C), &intersectionPoint))
        painter->drawLine(QLineF(intersectionPoint, rotationCenter()));

    /* drawing angle value */
    qreal rightAngle = mOrientation == Bottom ? mMarkerAngle : 360 - mMarkerAngle;


    QString angleText = QString("%1°").arg(rightAngle, 0, 'f', 1);

    QFont font1 = painter->font();
#ifdef Q_WS_MAC
    font1.setPointSizeF(font1.pointSizeF() - 3);
#endif
    QFontMetricsF fm1(font1);

    if (mOrientation == Bottom)
        painter->drawText(rotationCenter().x() - fm1.width(angleText)/2 - radius()/8, rotationCenter().y() + radius()/8 - fm1.height()/2, fm1.width(angleText), fm1.height(), Qt::AlignCenter, angleText);
    else
        painter->drawText(rotationCenter().x() - fm1.width(angleText)/2 - radius()/8, rotationCenter().y() - radius()/8 - fm1.height()/2, fm1.width(angleText), fm1.height(), Qt::AlignCenter, angleText);
}


void UBGraphicsAristo::rotateAroundCenter(qreal angle)
{
    qreal oldAngle = mRotatedAngle;
    mRotatedAngle = angle;
    QTransform transform;
    rotateAroundCenter(transform, rotationCenter());
    setTransform(transform, true);
    mRotatedAngle = oldAngle + angle; // We have to store absolute value for FLIP case
}

void UBGraphicsAristo::rotateAroundCenter(QTransform& transform, QPointF center)
{
    transform.translate(center.x(), center.y());
    transform.rotate(mRotatedAngle);
    transform.translate(- center.x(), - center.y());
}

void UBGraphicsAristo::resize(qreal factor)
{
    prepareGeometryChange();
    translate(rotationCenter().x(), rotationCenter().y());
    scale(factor, factor);
    translate(-rotationCenter().x(), -rotationCenter().y());
}


QPointF UBGraphicsAristo::rotationCenter() const
{
    return QPointF((A.x() + B.x()) / 2, (A.y() + B.y()) / 2);
}

QRectF UBGraphicsAristo::closeButtonRect() const
{
    qreal y = radius() / 4 + hFlipRect().height() + 3 + rotateRect().height() + 3;
    if (mOrientation == Top)
        y = -y;
    return QRectF(- mCloseSvgItem->boundingRect().width() / 2, y, mCloseSvgItem->boundingRect().width(), mCloseSvgItem->boundingRect().height());
}

QRectF UBGraphicsAristo::hFlipRect() const
{
    qreal y = radius() / 4;
    if (mOrientation == Top)
        y = -y;

     return QRectF(- mHFlipSvgItem->boundingRect().width() / 2, y, mHFlipSvgItem->boundingRect().width(), mHFlipSvgItem->boundingRect().height());
}

QRectF UBGraphicsAristo::markerButtonRect() const
{
    qreal y = - mMarkerSvgItem->boundingRect().height()/2;
    return QRectF (radius()/2 - mMarkerSvgItem->boundingRect().width(), y, mMarkerSvgItem->boundingRect().width(), mMarkerSvgItem->boundingRect().height());
}

QRectF  UBGraphicsAristo::resizeButtonRect() const
{
    return QRectF((B - rotationCenter()).x() - 100 - mResizeSvgItem->boundingRect().width()/2, - mResizeSvgItem->boundingRect().height()/2, mResizeSvgItem->boundingRect().width(), mResizeSvgItem->boundingRect().height());   
}

QRectF UBGraphicsAristo::rotateRect() const
{
    qreal y = radius() / 4 + hFlipRect().height() + 3;
    if (mOrientation == Top)
        y = -y;
    return QRectF(- mRotateSvgItem->boundingRect().width() / 2, y, mRotateSvgItem->boundingRect().width(), mRotateSvgItem->boundingRect().height());

}

QCursor UBGraphicsAristo::flipCursor() const
{
    return Qt::ArrowCursor;
}

QCursor UBGraphicsAristo::markerCursor() const
{
    return Qt::ArrowCursor;
}

QCursor UBGraphicsAristo::resizeCursor() const
{
    return Qt::ArrowCursor;
}


void UBGraphicsAristo::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    switch (toolFromPos(event->pos())) {
    case Rotate:
        mRotating = true;
        event->accept();
        break;
    case Resize:
        mResizing = true;
        event->accept();
        break;
    case MoveMarker:
        mMarking = true;
        event->accept();
        break;
    default:
        QGraphicsItem::mousePressEvent(event);
        break;
    }

    mShowButtons = false;
    mHFlipSvgItem->setVisible(false);
    mCloseSvgItem->setVisible(false);
    mRotateSvgItem->setVisible(mRotating);
    mResizeSvgItem->setVisible(mResizing);
    update();
}

void UBGraphicsAristo::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

    if (!mResizing && !mRotating && !mMarking)
    {
        QGraphicsItem::mouseMoveEvent(event);
    }
    else
    {
        QLineF currentLine(rotationCenter(), event->pos());
        QLineF lastLine(rotationCenter(), event->lastPos());

        if (mRotating) {
            
            rotateAroundCenter(currentLine.angleTo(lastLine));
        }
        else if (mResizing) {
            QPointF delta = event->pos() - event->lastPos();
            calculatePoints(QRectF(boundingRect().topLeft(), QSizeF(boundingRect().width() + delta.x(), boundingRect().height() + delta.x())));
        }
        else if(mMarking) {
            qreal angle = currentLine.angleTo(lastLine);

            mMarkerAngle += angle;
            mMarkerAngle -= (int)(mMarkerAngle/360)*360;

            if (mOrientation == Bottom) {
                if (mMarkerAngle >= 270)
                    mMarkerAngle = 0;
                else if (mMarkerAngle > 180)
                    mMarkerAngle = 180;
            }
            else if (mOrientation == Top) {
                if (mMarkerAngle < 90)
                    mMarkerAngle = 360;
                else if (mMarkerAngle < 180)
                    mMarkerAngle = 180;
            }
            update();
        }

        event->accept();
    }
}

void UBGraphicsAristo::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (mResizing || mRotating || mMarking)
    {
        if (mMarking)
            setPath(determinePath());
        mResizing = false;
        mRotating = false;
        mMarking = false;
        event->accept();
    }
    else
    {
        switch (toolFromPos(event->pos())) {
        case Close :
            hide();
            emit hidden();
            break;
        case HorizontalFlip:            
            /* setting new orientation */
            switch(mOrientation) {
            case Bottom:
                setOrientation(Top);
                break;
            case Top:
                setOrientation(Bottom);
                break;
            default:
                break;
            }
        default:
            QGraphicsItem::mouseReleaseEvent(event);
            break;
        }
    }

    mShowButtons = true;
    update();
    if (scene())
        scene()->setModified(true);
}

void UBGraphicsAristo::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

    if (currentTool == UBStylusTool::Selector)  {
        mShowButtons = true;
        mHFlipSvgItem->setVisible(true);
        mRotateSvgItem->setVisible(true);
        mResizeSvgItem->setVisible(true);
        mCloseSvgItem->setVisible(true);

        switch (toolFromPos(event->pos())) {
        case HorizontalFlip:
            setCursor(flipCursor());
            break;
        case Rotate:
            setCursor(rotateCursor());
            break;
        case Resize:
            setCursor(resizeCursor());
            break;
        case MoveMarker:
            setCursor(markerCursor());
            break;
        case Close:
            setCursor(closeCursor());
            break;
        default:
            setCursor(moveCursor());
            break;
        }

        event->accept();
        update();

    } else if (UBDrawingController::drawingController()->isDrawingTool())  {
            setCursor(drawRulerLineCursor());
            UBDrawingController::drawingController()->mActiveRuler = this;
            event->accept();
    }
}

void UBGraphicsAristo::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mShowButtons = false;
    setCursor(Qt::ArrowCursor);
    mHFlipSvgItem->setVisible(false);
    mRotateSvgItem->setVisible(false);
    mResizeSvgItem->setVisible(false);
    mCloseSvgItem->setVisible(false);
    UBDrawingController::drawingController()->mActiveRuler = NULL;
    event->accept();
    update();
}

void UBGraphicsAristo::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController ()->stylusTool ();

    if (currentTool == UBStylusTool::Selector)
    {
        mShowButtons = true;
        mHFlipSvgItem->setVisible(true);
        mRotateSvgItem->setVisible(true);
        mResizeSvgItem->setVisible(true);
        mCloseSvgItem->setVisible(true);

        switch (toolFromPos(event->pos())) {
        case HorizontalFlip:
            setCursor(flipCursor());
            break;
        case Rotate:
            setCursor(rotateCursor());
            break;
        case Resize:
            setCursor(resizeCursor());
            break;
        case MoveMarker:
            setCursor(markerCursor());
            break;
        case Close:
            setCursor(closeCursor());
            break;
        default:
            setCursor(moveCursor());
            break;
        }

        event->accept();
    }
    else if (UBDrawingController::drawingController()->isDrawingTool())
        event->accept();
}

/*
 * toolfromPos() returns the item type corresponding to the given position.
 * This method is used to reduce the amount of code in each event function and improve class' maintainability.
 * pos: event's position ; a rotation is done to counter elements rotation, like the marker button.
 */ 
UBGraphicsAristo::Tool UBGraphicsAristo::toolFromPos(QPointF pos)
{
    pos = pos - rotationCenter();

    qreal rotationAngle = mOrientation == Bottom ? - mMarkerAngle : Top ? 360 * (int)(mMarkerAngle / 360 + 1) - mMarkerAngle : 0;

    QTransform t;
    t.rotate(rotationAngle);
    QPointF p2 = t.map(pos);

    if (resizeButtonRect().contains(pos))
        return Resize;
    else if (closeButtonRect().contains(pos))
        return Close;
    else if (rotateRect().contains(pos))
        return Rotate;
    else if (markerButtonRect().contains(p2))
        return MoveMarker;
    else if (hFlipRect().contains(pos))
        return HorizontalFlip;
    else if (shape().contains(pos))
        return Move;
    else
        return None;
}


void UBGraphicsAristo::StartLine(const QPointF &scenePos, qreal width)
{
    QPointF itemPos = mapFromScene(scenePos);

    qreal y;

    y = rotationCenter().y();

    if (itemPos.x() < boundingRect().x() + sLeftEdgeMargin)
            itemPos.setX(boundingRect().x() + sLeftEdgeMargin);
    if (itemPos.x() > boundingRect().x() + boundingRect().width() - sLeftEdgeMargin)
            itemPos.setX(boundingRect().x() + boundingRect().width() - sLeftEdgeMargin);

    itemPos.setY(y);
    itemPos = mapToScene(itemPos);

    scene()->moveTo(itemPos);
    scene()->drawLineTo(itemPos, width, true);
}

void UBGraphicsAristo::DrawLine(const QPointF &scenePos, qreal width)
{
    QPointF itemPos = mapFromScene(scenePos);

    qreal y;

    y = rotationCenter().y();

    if (itemPos.x() < boundingRect().x() + sLeftEdgeMargin)
            itemPos.setX(boundingRect().x() + sLeftEdgeMargin);
    if (itemPos.x() > boundingRect().x() + boundingRect().width() - sLeftEdgeMargin)
            itemPos.setX(boundingRect().x() + boundingRect().width() - sLeftEdgeMargin);

    itemPos.setY(y);
    itemPos = mapToScene(itemPos);

    // We have to use "pointed" line for marker tool
    scene()->drawLineTo(itemPos, width,
            UBDrawingController::drawingController()->stylusTool() != UBStylusTool::Marker);
}

void UBGraphicsAristo::EndLine()
{
    /* NOOP */
}


UBGraphicsScene* UBGraphicsAristo::scene() const
{
    return static_cast<UBGraphicsScene*>(QGraphicsPathItem::scene());
}
