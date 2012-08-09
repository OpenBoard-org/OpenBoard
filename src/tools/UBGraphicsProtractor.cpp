/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include "UBGraphicsProtractor.h"
#include "core/UBApplication.h"
#include "gui/UBResources.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "UBAbstractDrawRuler.h"

#include "core/memcheck.h"


const QRectF UBGraphicsProtractor::sDefaultRect = QRectF(-175, -175, 350, 350);

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

    scale(1.5, 1.5);
}


void UBGraphicsProtractor::paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget)
{
    painter->save();

    Q_UNUSED(styleOption);
    Q_UNUSED(widget);

    painter->setFont(QFont("Arial"));
    painter->setPen(drawColor());
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
        prepareGeometryChange();
        translate(rect().center().x(), rect().center().y());
        scale(scaleFactor, scaleFactor);
        translate(-rect().center().x(), -rect().center().y());
        mScaleFactor *= scaleFactor;
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
        emit hidden();
        break;

    case MoveMarker :
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
    qreal antiSc = antiScale();

    if (buttonSizeReference().width() * antiSc <=  buttonSizeReference().width() * 15)
        return QRectF(-buttonSizeReference().width() * 7, -buttonSizeReference().height() * antiSc / 2,
                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
    else
        return QRectF(-buttonSizeReference().width() * antiSc / 2, -buttonSizeReference().height() * antiSc / 2,
                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
}


QRectF UBGraphicsProtractor::closeButtonRect () const
{
    qreal antiSc = antiScale();

    if (buttonSizeReference().width() * antiSc <=  buttonSizeReference().width() * 2)
        return QRectF(-buttonSizeReference().width() * 9, -buttonSizeReference().height() * antiSc / 2,
                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
    else if (buttonSizeReference().width() * antiSc <=  buttonSizeReference().width() * 15)
        return QRectF(-buttonSizeReference().width() * 7 -buttonSizeReference().width() * antiSc, -buttonSizeReference().height() * antiSc / 2,
                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
    else
        return QRectF(-buttonSizeReference().width() * antiSc / 2, -buttonSizeReference().height() * antiSc / 2,
                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
}


QRectF UBGraphicsProtractor::resizeButtonRect () const
{
    qreal antiSc = antiScale();

    if (buttonSizeReference().width() * antiSc <=  buttonSizeReference().width() * 15)
        return QRectF(buttonSizeReference().width() * 8, -buttonSizeReference().height() * antiSc / 2,
                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
    else
    {
        mResizeSvgItem->setZValue(zValue()+10);
        return QRectF(-buttonSizeReference().width() * antiSc / 2, -buttonSizeReference().height() * antiSc / 2,
                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
    }
}


void UBGraphicsProtractor::paintGraduations(QPainter *painter)
{
    painter->save();

    const int  tenDegreeGraduationLength = 15;
    const int fiveDegreeGraduationLength = 10;
    const int  oneDegreeGraduationLength = 5;

    QFont font1 = painter->font();
#ifdef Q_WS_MAC
    font1.setPointSizeF(font1.pointSizeF() - 3);
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
            painter->drawLine(QLineF(QPointF(center.x(), center.y()), QPointF(center.x() + co*tenDegreeGraduationLength, center.y() - si*tenDegreeGraduationLength)));

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

            painter->drawText(QRectF(center.x() + (rad - graduationLength*1.5)*co  - fm1.width(grad)/2,
                                     center.y() - (rad - graduationLength*1.5)*si - fm1.height()/2,
                                     fm1.width(grad), fm1.height()), Qt::AlignTop, grad);

            //internal arc
            painter->setFont(font2);
            painter->drawText(QRectF(center.x() + (rad/2 + graduationLength*1.5)*co  - fm2.width(grad2)/2,
                                     center.y() - (rad/2 + graduationLength*1.5)*si - fm2.height()/2,
                                     fm2.width(grad2), fm2.height()), Qt::AlignTop, grad2);
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
        qreal antiSc = antiScale();

        qreal scale = buttonSizeReference().width() / mCloseSvgItem->boundingRect().width();
        mCloseSvgItem->setPos(closeButtonRect().topLeft() + rect().center());
        mCloseSvgItem->resetTransform();
        mCloseSvgItem->translate(-closeButtonRect().left(),-closeButtonRect().top());
        mCloseSvgItem->rotate(-mStartAngle);
        mCloseSvgItem->translate(closeButtonRect().left(), closeButtonRect().top());
        mCloseSvgItem->scale(scale * antiSc, scale * antiSc);//this do not impact the bounding box of thr svg item...

        mResetSvgItem->setPos(resetButtonRect().topLeft() + rect().center());
        mResetSvgItem->resetTransform();
        mResetSvgItem->translate(-resetButtonRect().left(), -resetButtonRect().top());
        mResetSvgItem->rotate(-mStartAngle);
        mResetSvgItem->translate(resetButtonRect().left(), resetButtonRect().top());
        mResetSvgItem->scale(scale * antiSc, scale * antiSc);//this do not impact the bounding box of thr svg item...

        mResizeSvgItem->setPos(resizeButtonRect().topLeft() + rect().center());
        mResizeSvgItem->resetTransform();
        mResizeSvgItem->translate(-resizeButtonRect().left(), -resizeButtonRect().top());
        mResizeSvgItem->rotate(-mStartAngle);
        mResizeSvgItem->translate(resizeButtonRect().left(), resizeButtonRect().top());
        mResizeSvgItem->scale(scale * antiSc, scale * antiSc);//this do not impact the bounding box of thr svg item...

        mRotateSvgItem->setPos(rotateButtonRect().topLeft() + rect().center());
        mRotateSvgItem->resetTransform();
        mRotateSvgItem->translate(-rotateButtonRect().left(), -rotateButtonRect().top());
        mRotateSvgItem->rotate(-mStartAngle);
        mRotateSvgItem->translate(rotateButtonRect().left(), rotateButtonRect().top());
        mRotateSvgItem->scale(scale, scale);//this do not impact the bounding box of thr svg item...
    }

    qreal scale = markerSizeReference().width()/mMarkerSvgItem->boundingRect().width();
    mMarkerSvgItem->setPos(markerButtonRect().topLeft() + rect().center());
    mMarkerSvgItem->resetTransform();
    mMarkerSvgItem->translate(-markerButtonRect().left(), -markerButtonRect().top());
    mMarkerSvgItem->rotate(- mStartAngle - mCurrentAngle);
    mMarkerSvgItem->translate(markerButtonRect().left(), markerButtonRect().top());
    mMarkerSvgItem->scale(scale, scale);//this do not impact the bounding box of thr svg item...

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
        painter->drawText(QRectF(rect().center().x() + (rad*2.5/10)*co  - fm2.width(ang)/2,
                                 rect().center().y() - (rad*2.5/10)*si - fm2.height()/2,
                                 fm2.width(ang), fm2.height()), Qt::AlignTop, ang);
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

    if (resizeButtonRect().contains(p1))
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
    Q_UNUSED(angle);
}

QPointF UBGraphicsProtractor::rotationCenter() const
{
    return QPointF(rect().x(), rect().y());
}
