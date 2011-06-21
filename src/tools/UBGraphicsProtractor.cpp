
/*
 * UBGraphicsProtractor.cpp
 *
 *  Created on: April 23, 2009
 *      Author: Patrick Leclere
 */

#include "UBGraphicsProtractor.h"
#include "core/UBApplication.h"
#include "gui/UBResources.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"


const int UBGraphicsProtractor::sFillTransparency = 127;
const int UBGraphicsProtractor::sDrawTransparency = 192;
const QRectF                   UBGraphicsProtractor::sDefaultRect = QRectF(-175, -175, 350, 350);
const QColor                     UBGraphicsProtractor::sFillColor = QColor(0x72, 0x72, 0x72, sFillTransparency);
const QColor               UBGraphicsProtractor::sFillColorCenter = QColor(0xbe, 0xbe, 0xbe, sFillTransparency);
const QColor                     UBGraphicsProtractor::sDrawColor = QColor(32, 32, 32, sDrawTransparency);
const QColor       UBGraphicsProtractor::sDarkBackgroundFillColor = QColor(0xb5, 0xb5, 0xb5, sFillTransparency);
const QColor UBGraphicsProtractor::sDarkBackgroundFillColorCenter = QColor(0xde, 0xde, 0xde, sFillTransparency);
const QColor       UBGraphicsProtractor::sDarkBackgroundDrawColor = QColor(255, 255, 255, sDrawTransparency);

UBGraphicsProtractor::UBGraphicsProtractor()
    : QGraphicsEllipseItem(sDefaultRect)
    , mCurrentTool(None)
    , mShowButtons(false)
    , mCurrentAngle(0)
    , mSpan(180)
    , mStartAngle(0)
    , mScaleFactor(1)
    , mCloseSvgItem(0)
    , mResetSvgItem(0)
    , mResizeSvgItem(0)
    , mRotateSvgItem(0)
    , mMarkerSvgItem(0)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setAcceptsHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

	setStartAngle(0);
	setSpanAngle(180 * 16);

    mCloseSvgItem = new QGraphicsSvgItem(":/images/closeTool.svg", this);
    mCloseSvgItem->setVisible(false);
    mCloseSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    mCloseSvgItem->setPos(closeButtonBounds().topLeft());

    mResetSvgItem = new QGraphicsSvgItem(":/images/resetTool.svg", this);
    mResetSvgItem->setVisible(false);
    mResetSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    mResetSvgItem->setPos(resetButtonBounds().topLeft());

    mResizeSvgItem = new QGraphicsSvgItem(":/images/resizeTool.svg", this);
    mResizeSvgItem->setVisible(false);
    mResizeSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    mResizeSvgItem->setPos(resizeButtonBounds().topLeft());

    mRotateSvgItem = new QGraphicsSvgItem(":/images/rotateProtractor.svg", this);
    mRotateSvgItem->setVisible(false);
    mRotateSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    mRotateSvgItem->setPos(rotateButtonBounds().topLeft());

    mMarkerSvgItem = new QGraphicsSvgItem(":/images/angleMarker.svg", this);
    mMarkerSvgItem->setVisible(false);
    mMarkerSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Tool));
    mMarkerSvgItem->setPos(markerButtonBounds().topLeft());

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
    painter->drawPie(QRectF(rect().center().x() - radius(), rect().center().y() - radius(),
            2 * radius(), 2 * radius()), mStartAngle * 16, mSpan * 16);
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

    QRectF bounds = resizeButtonBounds().adjusted(centerX, centerY, centerX, centerY);
    bounds = bounds.united(closeButtonBounds().adjusted(centerX, centerY, centerX, centerY));
    bounds = bounds.united(resetButtonBounds().adjusted(centerX, centerY, centerX, centerY));

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
    QRectF markerBounds = markerButtonBounds();

    QPointF center = rect().center();
    qreal centerX = center.x();
    qreal centerY = center.y();

    buttonPath.addRect(resizeButtonBounds().adjusted(centerX, centerY, centerX, centerY));
    if(!resizeButtonBounds().contains(markerBounds))
    {
        buttonPath.addRect(markerBounds.adjusted(centerX - markerBounds.left() * 2 - markerBounds.width(), centerY
                , centerX - markerBounds.left() * 2 - markerBounds.width(), centerY));
        buttonPath.addRect(markerBounds.adjusted(centerX, centerY, centerX, centerY));
    }
    buttonPath.addRect(closeButtonBounds().adjusted(centerX, centerY, centerX, centerY));
    buttonPath.addRect(resetButtonBounds().adjusted(centerX, centerY, centerX, centerY));
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

    switch(mCurrentTool)
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
            if((int)mCurrentAngle % 360 > 270)
			    mCurrentAngle = 0;
            else if((int)mCurrentAngle % 360 >= 180)
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

    if(mCurrentTool != Move)
        event->accept();

}


void UBGraphicsProtractor::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    switch(mCurrentTool)
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

    mCurrentTool = None;
}


void UBGraphicsProtractor::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector)
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

    if (!mShowButtons)
    {
        mShowButtons = true;
        update();
    }

    if(currentTool == Move)
        setCursor(Qt::SizeAllCursor);
    else
        setCursor(Qt::ArrowCursor);

    event->accept();
}


qreal UBGraphicsProtractor::antiScale() const
{
    return 1 / (mScaleFactor * UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
}


QRectF UBGraphicsProtractor::resetButtonBounds () const
{
    qreal antiSc = antiScale();

    if(buttonSizeReference().width() * antiSc <=  buttonSizeReference().width() * 15)
       return QRectF(-buttonSizeReference().width() * 7, -buttonSizeReference().height() * antiSc / 2,
                                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
    else
       return QRectF(-buttonSizeReference().width() * antiSc / 2, -buttonSizeReference().height() * antiSc / 2,
                                      buttonSizeReference().width() * antiSc, buttonSizeReference().height() * antiSc);
}


QRectF UBGraphicsProtractor::closeButtonBounds () const
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


QRectF UBGraphicsProtractor::resizeButtonBounds () const
{
    qreal antiSc = antiScale();

    if(buttonSizeReference().width() * antiSc <=  buttonSizeReference().width() * 15)
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

    for(int angle = 1; angle < mSpan; angle++)
    {
        int graduationLength = (0 == angle % 10) ? tenDegreeGraduationLength : ((0 == angle % 5) ? fiveDegreeGraduationLength : oneDegreeGraduationLength);
        qreal co = cos(((qreal)angle + mStartAngle) * PI/180);
        qreal si = sin(((qreal)angle + mStartAngle) * PI/180);
        if(0 == angle % 90)
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

    if(mShowButtons)
    {
        qreal antiSc = antiScale();

        qreal scale = buttonSizeReference().width() / mCloseSvgItem->boundingRect().width();
        mCloseSvgItem->setPos(closeButtonBounds().topLeft() + rect().center());
        mCloseSvgItem->resetTransform();
        mCloseSvgItem->translate(-closeButtonBounds().left(),-closeButtonBounds().top());
        mCloseSvgItem->rotate(-mStartAngle);
        mCloseSvgItem->translate(closeButtonBounds().left(), closeButtonBounds().top());
        mCloseSvgItem->scale(scale * antiSc, scale * antiSc);//this do not impact the bounding box of thr svg item...

        mResetSvgItem->setPos(resetButtonBounds().topLeft() + rect().center());
        mResetSvgItem->resetTransform();
        mResetSvgItem->translate(-resetButtonBounds().left(), -resetButtonBounds().top());
        mResetSvgItem->rotate(-mStartAngle);
        mResetSvgItem->translate(resetButtonBounds().left(), resetButtonBounds().top());
        mResetSvgItem->scale(scale * antiSc, scale * antiSc);//this do not impact the bounding box of thr svg item...

        mResizeSvgItem->setPos(resizeButtonBounds().topLeft() + rect().center());
        mResizeSvgItem->resetTransform();
        mResizeSvgItem->translate(-resizeButtonBounds().left(), -resizeButtonBounds().top());
        mResizeSvgItem->rotate(-mStartAngle);
        mResizeSvgItem->translate(resizeButtonBounds().left(), resizeButtonBounds().top());
        mResizeSvgItem->scale(scale * antiSc, scale * antiSc);//this do not impact the bounding box of thr svg item...

        mRotateSvgItem->setPos(rotateButtonBounds().topLeft() + rect().center());
        mRotateSvgItem->resetTransform();
        mRotateSvgItem->translate(-rotateButtonBounds().left(), -rotateButtonBounds().top());
        mRotateSvgItem->rotate(-mStartAngle);
        mRotateSvgItem->translate(rotateButtonBounds().left(), rotateButtonBounds().top());
        mRotateSvgItem->scale(scale, scale);//this do not impact the bounding box of thr svg item...
    }

    qreal scale = markerSizeReference().width()/mMarkerSvgItem->boundingRect().width();
    mMarkerSvgItem->setPos(markerButtonBounds().topLeft() + rect().center());
    mMarkerSvgItem->resetTransform();
    mMarkerSvgItem->translate(-markerButtonBounds().left(), -markerButtonBounds().top());
    mMarkerSvgItem->rotate(- mStartAngle - mCurrentAngle);
    mMarkerSvgItem->translate(markerButtonBounds().left(), markerButtonBounds().top());
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

    if(angle != 0)
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

    if(resizeButtonBounds().contains(p1))
        return Resize;
    else if(closeButtonBounds().contains(p1))
        return Close;
    else if(resetButtonBounds().contains(p1))
        return Reset;
    else if(rotateButtonBounds().contains(p1))
        return Rotate;
    else if(markerButtonBounds().contains(p2))
        return MoveMarker;
    else if(line.length() <= radius())
        return Move;
    else
        return None;
}


UBGraphicsScene* UBGraphicsProtractor::scene() const
{
    return static_cast<UBGraphicsScene*>(QGraphicsEllipseItem::scene());
}

QColor UBGraphicsProtractor::drawColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundDrawColor : sDrawColor;
}

QBrush UBGraphicsProtractor::fillBrush() const
{
    QColor fillColor = scene()->isDarkBackground() ? sDarkBackgroundFillColor : sFillColor;
    QColor fillColorCenter = scene()->isDarkBackground() ? sDarkBackgroundFillColorCenter : sFillColorCenter;
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

    copy->setPos(this->pos());
    copy->setRect(this->rect());
    copy->setZValue(this->zValue());
    copy->setTransform(this->transform());

    copy->mCurrentAngle = this->mCurrentAngle;
    copy->mSpan = this->mSpan;
    copy->mStartAngle = this->mStartAngle;
    copy->mScaleFactor = this->mScaleFactor;

    // TODO UB 4.7 ... complete all members ?

    return copy;
}
