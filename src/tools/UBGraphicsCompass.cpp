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

#include "tools/UBGraphicsCompass.h"
#include "domain/UBGraphicsScene.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "gui/UBResources.h"
#include "domain/UBGraphicsScene.h"

#include "board/UBBoardController.h" // TODO UB 4.x clean that dependency
#include "board/UBDrawingController.h" // TODO UB 4.x clean that dependency

#include "core/memcheck.h"

const QRect UBGraphicsCompass::sDefaultRect = QRect(0, -20, 300, 48);
const QColor UBGraphicsCompass::sLightBackgroundMiddleFillColor = QColor(0x72, 0x72, 0x72, sFillTransparency);
const QColor UBGraphicsCompass::sLightBackgroundEdgeFillColor = QColor(0xc3, 0xc3, 0xc3, sFillTransparency);
const QColor UBGraphicsCompass::sLightBackgroundDrawColor = QColor(0x33, 0x33, 0x33, sDrawTransparency);
const QColor UBGraphicsCompass::sDarkBackgroundMiddleFillColor = QColor(0xb5, 0xb5, 0xb5, sFillTransparency);
const QColor UBGraphicsCompass::sDarkBackgroundEdgeFillColor = QColor(0xdd, 0xdd, 0xdd, sFillTransparency);
const QColor UBGraphicsCompass::sDarkBackgroundDrawColor = QColor(0xff, 0xff, 0xff, sDrawTransparency);

const int UBGraphicsCompass::sMinRadius = UBGraphicsCompass::sNeedleLength + UBGraphicsCompass::sNeedleBaseLength
        + 32 + UBGraphicsCompass::sDefaultRect.height() + 32 + UBGraphicsCompass::sPencilBaseLength
        + UBGraphicsCompass::sPencilLength;

UBGraphicsCompass::UBGraphicsCompass()
    : QGraphicsRectItem()
    , mResizing(false)
    , mRotating(false)
    , mDrawing(false)
    , mShowButtons(false)
    , mSpanAngleInDegrees(0)
    , mDrewCircle(false)
    , mCloseSvgItem(0)
    , mResizeSvgItem(0)
    , mAntiScaleRatio(1.0)
    , mDrewCenterCross(false)
{
    setRect(sDefaultRect);
    //TODO claudio: remove code duplication
    QDesktopWidget* desktop = UBApplication::desktop();
    int dpiCommon = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
    mPixelsPerMillimeter = qRound(dpiCommon / 25.4f);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setAcceptsHoverEvents(true);

    mCloseSvgItem = new QGraphicsSvgItem(":/images/closeTool.svg", this);
    mCloseSvgItem->setVisible(false);
    mCloseSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mResizeSvgItem = new QGraphicsSvgItem(":/images/resizeCompass.svg", this);
    mResizeSvgItem->setVisible(false);
    mResizeSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    updateResizeCursor();
    updateDrawCursor();

    unsetCursor();

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::CppTool)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    connect(UBApplication::boardController, SIGNAL(penColorChanged()), this, SLOT(penColorChanged()));
    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), this, SLOT(lineWidthChanged()));
}

UBGraphicsCompass::~UBGraphicsCompass()
{
    // NOOP
}

UBItem* UBGraphicsCompass::deepCopy() const
{
   UBGraphicsCompass* copy = new UBGraphicsCompass();

    copyItemParameters(copy);

   // TODO UB 4.7 ... complete all members ?

   return copy;
}

void UBGraphicsCompass::copyItemParameters(UBItem *copy) const
{
    UBGraphicsCompass *cp = dynamic_cast<UBGraphicsCompass*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setRect(this->rect());
        cp->setTransform(this->transform());
    }
}

void UBGraphicsCompass::paint(QPainter *painter, const QStyleOptionGraphicsItem *styleOption, QWidget *widget)
{
    Q_UNUSED(styleOption);
    Q_UNUSED(widget);

    mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
    QTransform antiScaleTransform;
    antiScaleTransform.scale(mAntiScaleRatio, mAntiScaleRatio);

    mCloseSvgItem->setTransform(antiScaleTransform);
    mCloseSvgItem->setPos(
        closeButtonRect().center().x() - mCloseSvgItem->boundingRect().width() * mAntiScaleRatio / 2,
        closeButtonRect().center().y() - mCloseSvgItem->boundingRect().height() * mAntiScaleRatio / 2);

    mResizeSvgItem->setTransform(antiScaleTransform);
    mResizeSvgItem->setPos(
        resizeButtonRect().center().x() - mResizeSvgItem->boundingRect().width() * mAntiScaleRatio / 2,
        resizeButtonRect().center().y() - mResizeSvgItem->boundingRect().height() * mAntiScaleRatio / 2);

    painter->setPen(drawColor());
    painter->drawRoundedRect(hingeRect(), 4, 4);
    painter->fillPath(hingeShape(), middleFillColor());

    painter->fillPath(needleShape(), middleFillColor());
    painter->drawPath(needleShape());
    painter->fillPath(needleBaseShape(), middleFillColor());
    painter->drawPath(needleBaseShape());

    QLinearGradient needleArmLinearGradient(
        QPointF(rect().left() + sNeedleLength + sNeedleBaseLength, rect().center().y()),
        QPointF(hingeRect().left(), rect().center().y()));
    needleArmLinearGradient.setColorAt(0, edgeFillColor());
    needleArmLinearGradient.setColorAt(1, middleFillColor());
    painter->fillPath(needleArmShape(), needleArmLinearGradient);
    painter->drawPath(needleArmShape());

    QRectF hingeGripRect(rect().center().x() - 16, rect().center().y() - 16, 32, 32);
    painter->drawEllipse(hingeGripRect);
    if (mShowButtons)
        paintAngleDisplay(painter);

    QLinearGradient pencilArmLinearGradient(
        QPointF(hingeRect().right(), rect().center().y()),
        QPointF(rect().right() - sPencilLength - sPencilBaseLength, rect().center().y()));
    pencilArmLinearGradient.setColorAt(0, middleFillColor());
    pencilArmLinearGradient.setColorAt(1, edgeFillColor());
    painter->fillPath(pencilArmShape(), pencilArmLinearGradient);
    painter->drawPath(pencilArmShape());

    if (scene()->isDarkBackground())
        painter->fillPath(pencilShape(), UBApplication::boardController->penColorOnDarkBackground());
    else
        painter->fillPath(pencilShape(), UBApplication::boardController->penColorOnLightBackground());

    painter->fillPath(pencilBaseShape(), middleFillColor());
    painter->drawPath(pencilBaseShape());

    if (mResizing || mRotating || mDrawing || (mShowButtons && rect().width() > sDisplayRadiusOnPencilArmMinLength))
        paintRadiusDisplay(painter);
}


QVariant UBGraphicsCompass::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneChange)
    {
        mCloseSvgItem->setParentItem(this);
        mResizeSvgItem->setParentItem(this);
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void UBGraphicsCompass::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector &&
        UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Play)
        return;

    if (resizeButtonRect().contains(event->pos()))
    {
        mResizing = true;
        mCloseSvgItem->setVisible(false);
        event->accept();
    }
    else if (hingeRect().contains(event->pos()))
    {
        mRotating = true;
        mResizeSvgItem->setVisible(false);
        mCloseSvgItem->setVisible(false);
        event->accept();
    }
    else
    {
        mDrawing = event->pos().x() > rect().right() - sPencilLength - sPencilBaseLength;
        if (mDrawing)
        {
            mSpanAngleInDegrees = 0;
            mSceneArcStartPoint = mapToScene(pencilPosition());
            scene()->initStroke();
            scene()->moveTo(mSceneArcStartPoint);
        }

        mResizeSvgItem->setVisible(false);
        mCloseSvgItem->setVisible(false);

        QGraphicsRectItem::mousePressEvent(event);
    }
}

void UBGraphicsCompass::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector &&
        UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Play)
        return;

    if (!mResizing && !mRotating && !mDrawing)
    {
        QGraphicsRectItem::mouseMoveEvent(event);
        mDrewCenterCross = false;
    }
    else
    {
        if (mResizing)
        {
            QPointF delta = event->pos() - event->lastPos();
            if (rect().width() + delta.x() < sMinRadius)
                delta.setX(sMinRadius - rect().width());
            setRect(QRectF(rect().topLeft(), QSizeF(rect().width() + delta.x(), rect().height())));
        }
        else
        {
            QLineF currentLine(needlePosition(), event->pos());
            QLineF lastLine(needlePosition(), event->lastPos());
            qreal deltaAngle = currentLine.angleTo(lastLine);
            if (deltaAngle > 180)
                deltaAngle -= 360;
            else if (deltaAngle < -180)
                deltaAngle += 360;
            rotateAroundNeedle(deltaAngle);

            if (mDrawing)
            {
                mSpanAngleInDegrees += deltaAngle;
                if (mSpanAngleInDegrees >= 1080)
                    mSpanAngleInDegrees -= 360;
                else if (mSpanAngleInDegrees < -1080)
                    mSpanAngleInDegrees += 360;
                drawArc();
            }
        }
        event->accept();
    }
}

void UBGraphicsCompass::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector &&
        UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Play)
        return;

    if (mResizing)
    {
        mResizing = false;
        event->accept();
    }
    else if (mRotating)
    {
        mRotating = false;
        updateResizeCursor();
        updateDrawCursor();
        event->accept();
    }
    else if (mDrawing)
    {
        mDrawing = false;
        updateResizeCursor();
        updateDrawCursor();
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

void UBGraphicsCompass::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector &&
        UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Play)
        return;

    mOuterCursor = cursor();
    mShowButtons = shape().contains(event->pos());

    mCloseSvgItem->setParentItem(this);
    mResizeSvgItem->setParentItem(this);

    mCloseSvgItem->setVisible(mShowButtons);
    if (mShowButtons)
    {
        if (hingeRect().contains(event->pos()))
            setCursor(rotateCursor());
        else if (event->pos().x() > rect().right() - sPencilLength - sPencilBaseLength)
            setCursor(drawCursor());
        else if (resizeButtonRect().contains(event->pos()))
            setCursor(resizeCursor());
        else if (closeButtonRect().contains(event->pos()))
            setCursor(closeCursor());
        else
            setCursor(moveCursor());
    }
    event->accept();
    update();
}

void UBGraphicsCompass::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector &&
        UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Play)
        return;

    mShowButtons = false;
    mCloseSvgItem->setVisible(mShowButtons);
    mResizeSvgItem->setVisible(mShowButtons);
    unsetCursor();
    event->accept();
    update();
}

void UBGraphicsCompass::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector &&
        UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Play)
        return;

    mShowButtons = shape().contains(event->pos());
    mCloseSvgItem->setVisible(mShowButtons);
    mResizeSvgItem->setVisible(mShowButtons);
    if (mShowButtons)
    {
        if (hingeRect().contains(event->pos()))
            setCursor(rotateCursor());
        else if (event->pos().x() > rect().right() - sPencilLength - sPencilBaseLength)
            setCursor(drawCursor());
        else if (resizeButtonRect().contains(event->pos()))
            setCursor(resizeCursor());
        else if (closeButtonRect().contains(event->pos()))
            setCursor(closeCursor());
        else
            setCursor(moveCursor());
    }
    else
    {
        setCursor(mOuterCursor);
    }
    event->accept();
    update();
}

void UBGraphicsCompass::paintAngleDisplay(QPainter *painter)
{
    qreal angle = angleInDegrees();

    qreal angleValue = mDrawing ? - mSpanAngleInDegrees : angle;
    QString angleText = QString("%1").arg(angleValue, 0, 'f', 1) + "°";

    painter->save();
    painter->setFont(font());
        QFontMetricsF fm(painter->font());
    painter->translate(hingeRect().center());
    painter->rotate(angle);
        painter->drawText(
        QRectF(
            - fm.width(angleText) / 2,
            - fm.height() / 2,
            fm.width(angleText),
            fm.height()),
        Qt::AlignTop,
        angleText);
    painter->restore();
}

void UBGraphicsCompass::paintRadiusDisplay(QPainter *painter)
{
    qreal radiusInCentimeters = rect().width() / (mPixelsPerMillimeter * 10);
    QString format = rect().width() >= sDisplayRadiusUnitMinLength ? "%1 cm" : "%1";
    QString radiusText = QString(format).arg(radiusInCentimeters, 0, 'f', 1);

    bool onPencilArm = rect().width() > sDisplayRadiusOnPencilArmMinLength;

    painter->save();
    painter->setFont(font());
    QFontMetricsF fm(painter->font());
    QPointF textCenter;

    if (onPencilArm)
        textCenter = QPointF(rect().right() - sPencilBaseLength - sPencilLength - fm.width(radiusText) / 2 - 24 - 8, rect().center().y());
    else
        textCenter = QPointF((rect().left() + sNeedleLength + sNeedleBaseLength + hingeRect().left()) / 2, rect().center().y());

    painter->translate(textCenter);
    qreal angle = angleInDegrees();
    if (angle > 180)
        angle -= 360;
    else if (angle < -180)
        angle += 360;
    if (angle <= -90 || angle > 90)
        painter->rotate(180);
    painter->drawText(
        QRectF(
            - fm.width(radiusText) / 2,
            - rect().height() / 2,
            fm.width(radiusText),
            rect().height()),
        Qt::AlignVCenter,
        radiusText);
    painter->restore();
}

QCursor UBGraphicsCompass::moveCursor() const
{
    return Qt::SizeAllCursor;
}

QCursor UBGraphicsCompass::resizeCursor() const
{
    return mResizeCursor;
}

QCursor UBGraphicsCompass::rotateCursor() const
{
    return UBResources::resources()->rotateCursor;
}

QCursor UBGraphicsCompass::closeCursor() const
{
    return Qt::ArrowCursor;
}

QCursor UBGraphicsCompass::drawCursor() const
{
    return mDrawCursor;
}

QRectF UBGraphicsCompass::hingeRect() const
{
    QRectF rotationRect(rect().width() / 2 - rect().height() / 2, 0, rect().height(), rect().height());
    rotationRect.translate(rect().topLeft());
    return rotationRect;
}

QRectF UBGraphicsCompass::closeButtonRect() const
{
    QPixmap closePixmap(":/images/closeTool.svg");

    QSizeF closeRectSize(
        closePixmap.width() * mAntiScaleRatio,
        closePixmap.height() * mAntiScaleRatio);

    QPointF closeRectTopLeft(
        sNeedleLength + sNeedleBaseLength + 4,
        (rect().height() - closeRectSize.height()) / 2);

    QRectF closeRect(closeRectTopLeft, closeRectSize);
    closeRect.translate(rect().topLeft());

    return closeRect;
}

QRectF UBGraphicsCompass::resizeButtonRect() const
{
    QPixmap resizePixmap(":/images/resizeCompass.svg");

    QSizeF resizeRectSize(
        resizePixmap.width() * mAntiScaleRatio,
        resizePixmap.height() * mAntiScaleRatio);

    QPointF resizeRectTopLeft(
        rect().width() - sPencilLength - sPencilBaseLength - resizeRectSize.width() - 4,
        (rect().height() - resizeRectSize.height()) / 2);

    QRectF resizeRect(resizeRectTopLeft, resizeRectSize);
    resizeRect.translate(rect().topLeft());

    return resizeRect;
}

void UBGraphicsCompass::rotateAroundNeedle(qreal angle)
{
    QTransform transform;
    transform.translate(needlePosition().x(), needlePosition().y());
    transform.rotate(angle);
    transform.translate(- needlePosition().x(), - needlePosition().y());
    setTransform(transform, true);
}

void UBGraphicsCompass::drawArc()
{
    if (!mDrewCenterCross)
    {
        paintCenterCross();
        mDrewCenterCross = true;
        scene()->moveTo(mSceneArcStartPoint);
    }
    QPointF sceneNeedlePosition = mapToScene(needlePosition());
    qreal arcSpanAngle = mSpanAngleInDegrees;
    if (arcSpanAngle > 360)
        arcSpanAngle = 360;
    else if (arcSpanAngle < -360)
        arcSpanAngle = -360;
    if (!mDrewCircle || (-360 != arcSpanAngle && 360 != arcSpanAngle))
    {
        mDrewCircle = (-360 == arcSpanAngle || 360 == arcSpanAngle);
        scene()->drawArcTo(sceneNeedlePosition, arcSpanAngle);
    }
}

void UBGraphicsCompass::updateResizeCursor()
{
    QPixmap pix(":/images/cursors/resize.png");
    qreal angle = angleInDegrees();

    QTransform tr;
    tr.rotate(- angle);
    mResizeCursor = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
}

void UBGraphicsCompass::updateDrawCursor()
{
    QPixmap pix(":/images/cursors/drawCompass.png");
    qreal angle = angleInDegrees();

    QTransform tr;
    tr.rotate(- angle);
    mDrawCursor = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
}

void UBGraphicsCompass::paintCenterCross()
{
    QPointF needleCrossCenter = mapToScene(needlePosition());
    scene()->moveTo(QPointF(needleCrossCenter.x() - 5, needleCrossCenter.y()));
    scene()->drawLineTo(QPointF(needleCrossCenter.x() + 5, needleCrossCenter.y()), 1,
        UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Line);
    scene()->moveTo(QPointF(needleCrossCenter.x(), needleCrossCenter.y() - 5));
    scene()->drawLineTo(QPointF(needleCrossCenter.x(), needleCrossCenter.y() + 5), 1,
        UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Line);
}

QPointF UBGraphicsCompass::needlePosition() const
{
    return QPointF(rect().x(), rect().y() + rect().height() / 2);
}

QPointF UBGraphicsCompass::pencilPosition() const
{
    return QPointF(rect().right(), rect().center().y());
}

QPainterPath UBGraphicsCompass::shape() const
{
    QPainterPath path = needleShape();
    path = path.united(needleBaseShape());
    path = path.united(needleArmShape());
    path.addRect(hingeRect());
    path = path.united(pencilArmShape());
    path = path.united(pencilBaseShape());
    path = path.united(pencilShape());
    return path;
}

QPainterPath UBGraphicsCompass::needleShape() const
{
    QPainterPath path;
    path.moveTo(rect().left(), rect().center().y());
    path.lineTo(rect().left() + sNeedleLength, rect().center().y() - 2);
    path.lineTo(rect().left() + sNeedleLength, rect().center().y() + 2);
    path.closeSubpath();
    return path;
}

QPainterPath UBGraphicsCompass::needleBaseShape() const
{
    QPainterPath path;
    path.moveTo(rect().left() + sNeedleLength, rect().center().y() - 4);
    path.arcTo(
        rect().left() + sNeedleLength,
        rect().center().y() - 8,
        8, 8,
        180, -90);
    path.lineTo(rect().left() + sNeedleLength + sNeedleBaseLength, rect().center().y() - 8);
    path.lineTo(rect().left() + sNeedleLength + sNeedleBaseLength, rect().center().y() + 8);
    path.lineTo(rect().left() + sNeedleLength + 4, rect().center().y() + 8);
    path.arcTo(
        rect().left() + sNeedleLength,
        rect().center().y(),
        8, 8,
        -90, -90);
    path.closeSubpath();

    return path;
}

QPainterPath UBGraphicsCompass::needleArmShape() const
{
    QPainterPath path;
    path.moveTo(rect().left() + sNeedleLength + sNeedleBaseLength, rect().center().y() - 8);
    path.arcTo(
        rect().left() + sNeedleLength + sNeedleBaseLength,
        rect().center().y() - 12,
        8, 8,
        180, -90);
    path.lineTo(hingeRect().left(), rect().center().y() - 16);
    path.lineTo(hingeRect().left(), rect().center().y() + 16);
    path.lineTo(rect().left() + sNeedleLength + sNeedleBaseLength + 4, rect().center().y() + 12);
    path.arcTo(
        rect().left() + sNeedleLength + sNeedleBaseLength,
        rect().center().y() + 4,
        8, 8,
        -90, -90);
    path.closeSubpath();
    return path;
}

QPainterPath UBGraphicsCompass::hingeShape() const
{
    QPainterPath path;
    path.moveTo(hingeRect().left() + 4, hingeRect().top());
    path.lineTo(hingeRect().right() - 4, hingeRect().top());
    path.arcTo(
        hingeRect().right() - 8,
        hingeRect().top(),
        8, 8,
        90, -90);
    path.lineTo(hingeRect().right(), hingeRect().bottom() - 4);
    path.arcTo(
        hingeRect().right() - 8,
        hingeRect().bottom() - 8,
        8, 8,
        0, -90);
    path.lineTo(hingeRect().left() + 4, hingeRect().bottom());
    path.arcTo(
        hingeRect().left(),
        hingeRect().bottom() - 8,
        8, 8,
        -90, -90);
    path.lineTo(hingeRect().left(), hingeRect().top() + 4);
    path.arcTo(
        hingeRect().left(),
        hingeRect().top(),
        8, 8,
        -180, -90);
    path.closeSubpath();
    return path;
}

QPainterPath UBGraphicsCompass::pencilShape() const
{
    int penWidthIndex = UBSettings::settings()->penWidthIndex();
    int logicalCompassPencilWidth = penWidthIndex > 1 ? 8 : (penWidthIndex > 0 ? 4 : 2);
    QPainterPath path;
    path.moveTo(rect().right() - sPencilLength, rect().center().y() - logicalCompassPencilWidth / 2);
    path.lineTo(rect().right() - logicalCompassPencilWidth / 2, rect().center().y() - logicalCompassPencilWidth / 2);
    QRectF tipRect(rect().right() - logicalCompassPencilWidth, rect().center().y() - logicalCompassPencilWidth / 2, logicalCompassPencilWidth, logicalCompassPencilWidth);
    path.arcTo(tipRect, 90, -180);
    path.lineTo(rect().right() - sPencilLength, rect().center().y() + logicalCompassPencilWidth / 2);
    path.closeSubpath();
    return path;
}

QPainterPath UBGraphicsCompass::pencilBaseShape() const
{
    QPainterPath path;
    path.moveTo(rect().right() - sPencilLength - sPencilBaseLength, rect().center().y() - 8);
    path.lineTo(rect().right() - sPencilLength - 4, rect().center().y() - 8);
    path.arcTo(
        rect().right() - sPencilLength - 8, rect().center().y() - 8,
        8, 8,
        90, -90);
    path.lineTo(rect().right() - sPencilLength, rect().center().y() + 4);
    path.arcTo(
        rect().right() - sPencilLength - 8, rect().center().y(),
        8, 8,
        0, -90);
    path.lineTo(rect().right() - sPencilLength - sPencilBaseLength, rect().center().y() + 8);
    path.closeSubpath();

    return path;
}

QPainterPath UBGraphicsCompass::pencilArmShape() const
{
    QPainterPath path;
    path.moveTo(hingeRect().right(), rect().center().y() - 16);
    path.lineTo(rect().right() - sPencilLength - sPencilBaseLength - 4, rect().center().y() - 12);
    path.arcTo(
        rect().right() - sPencilLength - sPencilBaseLength - 8, rect().center().y() - 12,
        8, 8,
        90, -90);
    path.lineTo(rect().right() - sPencilLength - sPencilBaseLength, rect().center().y() + 8);
    path.arcTo(
        rect().right() - sPencilLength - sPencilBaseLength - 8, rect().center().y() + 4,
        8, 8,
        0, -90);
    path.lineTo(hingeRect().right(), rect().center().y() + 16);
    path.closeSubpath();
    return path;
}

UBGraphicsScene* UBGraphicsCompass::scene() const
{
    return static_cast<UBGraphicsScene*>(QGraphicsRectItem::scene());
}

QColor UBGraphicsCompass::drawColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundDrawColor : sLightBackgroundDrawColor;
}

QColor UBGraphicsCompass::middleFillColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundMiddleFillColor : sLightBackgroundMiddleFillColor;
}

QColor UBGraphicsCompass::edgeFillColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundEdgeFillColor : sLightBackgroundEdgeFillColor;
}

QFont UBGraphicsCompass::font() const
{
    QFont font("Arial");
    font.setPixelSize(16);
    font.setBold(true);
    return font;
}

qreal UBGraphicsCompass::angleInDegrees() const
{
    QRectF itemRect = boundingRect();
    QTransform itemTransform = sceneTransform();
    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QLineF topLine(topLeft, topRight);
    return topLine.angle();
}

void UBGraphicsCompass::penColorChanged()
{
    QRect pencilRect(rect().right() - sPencilLength, rect().top(), sPencilLength, rect().height());
    update(pencilRect);
}

void UBGraphicsCompass::lineWidthChanged()
{
    QRect pencilRect(rect().right() - sPencilLength, rect().top(), sPencilLength, rect().height());
    update(pencilRect);
}
