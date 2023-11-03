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


#include <QVBoxLayout>


#include "tools/UBGraphicsCompass.h"
#include "domain/UBGraphicsScene.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "gui/UBResources.h"
#include "domain/UBGraphicsScene.h"
#include "core/UBSettings.h"

#include "board/UBBoardController.h" // TODO UB 4.x clean that dependency
#include "board/UBDrawingController.h" // TODO UB 4.x clean that dependency
#include "board/UBBoardView.h" // TODO UB 4.x clean that dependency

#include "core/memcheck.h"

const QRect UBGraphicsCompass::sDefaultRect = QRect(0, -20, 300, 36);
const QColor UBGraphicsCompass::sLightBackgroundMiddleFillColor = QColor(0x72, 0x72, 0x72, sFillTransparency);
const QColor UBGraphicsCompass::sLightBackgroundEdgeFillColor = QColor(0xc3, 0xc3, 0xc3, sFillTransparency);
const QColor UBGraphicsCompass::sLightBackgroundDrawColor = QColor(0x33, 0x33, 0x33, sDrawTransparency);
const QColor UBGraphicsCompass::sDarkBackgroundMiddleFillColor = QColor(0xb5, 0xb5, 0xb5, sFillTransparency);
const QColor UBGraphicsCompass::sDarkBackgroundEdgeFillColor = QColor(0xdd, 0xdd, 0xdd, sFillTransparency);
const QColor UBGraphicsCompass::sDarkBackgroundDrawColor = QColor(0xff, 0xff, 0xff, sDrawTransparency);

const int UBGraphicsCompass::sMinRadius = UBGraphicsCompass::sNeedleLength + UBGraphicsCompass::sNeedleBaseLength
        + 24 + UBGraphicsCompass::sDefaultRect.height() + 24 + UBGraphicsCompass::sPencilBaseLength
        + UBGraphicsCompass::sPencilLength + 48 + 48;

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
    , mSettingsSvgItem(0)
    , mRihtAngleSvgItem(0)
    , mAngleRotateSvgItem(0)
    , mAntiScaleRatio(1.0)
    , mDrewCenterCross(false)
    , mSettingsMenu(0)
{
    setRect(sDefaultRect);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setAcceptHoverEvents(true);

    mCloseSvgItem = new QGraphicsSvgItem(":/images/closeTool.svg", this);
    mCloseSvgItem->setVisible(false);
    mCloseSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mResizeSvgItem = new QGraphicsSvgItem(":/images/resizeCompass.svg", this);
    mResizeSvgItem->setVisible(false);
    mResizeSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mSettingsSvgItem = new QGraphicsSvgItem(":/images/settingsCompass.svg", this);
    mSettingsSvgItem->setVisible(false);
    mSettingsSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mRihtAngleSvgItem = new QGraphicsSvgItem(":/images/rightAngleCompass.svg", this);
    mRihtAngleSvgItem->setVisible(false);
    mRihtAngleSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mAngleRotateSvgItem = new QGraphicsSvgItem(":/images/angleRotateCompass.svg", this);
    mAngleRotateSvgItem->setVisible(false);
    mAngleRotateSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mAngleRotateDialog =  new UBCompassAngleRotateDialog(this);    
    mAngleRotateDialog->setWindowFlags(Qt::Tool);

    updateResizeCursor();
    updateDrawCursor();

    unsetCursor();

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::CppTool)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    connect(UBApplication::boardController, SIGNAL(penColorChanged()), this, SLOT(penColorChanged()));
    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), this, SLOT(lineWidthChanged()));
    connect(UBSettings::settings(), SIGNAL(pageBackgroundChanged()), this, SLOT(setMenuActions()));

    mNormalizePos = false;
    mNormalizeSize  = false;
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

    painter->setBrush(edgeFillColor());


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

    mSettingsSvgItem->setTransform(antiScaleTransform);
    mSettingsSvgItem->setPos(
                settingsButtonRect().center().x() - mSettingsSvgItem->boundingRect().width() * mAntiScaleRatio / 2,
                settingsButtonRect().center().y() - mSettingsSvgItem->boundingRect().height() * mAntiScaleRatio / 2);

    mRihtAngleSvgItem->setTransform(antiScaleTransform);
    mRihtAngleSvgItem->setPos(
                rightAngleButtonRect().center().x() - mRihtAngleSvgItem->boundingRect().width() * mAntiScaleRatio / 2,
                rightAngleButtonRect().center().y() - mRihtAngleSvgItem->boundingRect().height() * mAntiScaleRatio /2);

    mAngleRotateSvgItem->setTransform(antiScaleTransform);
    mAngleRotateSvgItem->setPos(
                angleRotateButtonRect().center().x() -  mAngleRotateSvgItem->boundingRect().width() * mAntiScaleRatio /2,
                angleRotateButtonRect().center().y() -  mAngleRotateSvgItem->boundingRect().height() * mAntiScaleRatio /2);

    painter->setPen(drawColor());
    painter->drawRoundedRect(hingeRect(), sCornerRadius, sCornerRadius);
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
        mSettingsSvgItem->setParent(this);
        mRihtAngleSvgItem->setParent(this);
        mAngleRotateSvgItem->setParent(this);
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void UBGraphicsCompass::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Selector &&
        UBDrawingController::drawingController ()->stylusTool() != UBStylusTool::Play)
        return;

    bool setting = false;
    bool rightAngle = false;
    bool angleRotate = false;
    bool closing = false;

    if (resizeButtonRect().contains(event->pos()))
    {
        mResizing = true;
        mRotating = false;
        event->accept();
        qDebug() << "resizing";
    }
    else if (hingeRect().contains(event->pos()))
    {
        mRotating = true;
        mResizing = false;
        event->accept();
        qDebug() << "hinge";
    }
    else if (settingsButtonRect().contains(event->pos()))
    {
        setting = true;
    }
    else if (rightAngleButtonRect().contains(event->pos()))
    {
        rightAngle = true;
    }
    else if (angleRotateButtonRect().contains((event->pos())))
    {
        mBeforeRotatePosition = QLineF(needlePosition(), pencilPosition());
        angleRotate = true;
    }
    else if (!closeButtonRect().contains(event->pos()))
    {
        qDebug() << "the rest";

        mDrawing = event->pos().x() > rect().right() - sPencilLength - sPencilBaseLength;
        if (mDrawing)
        {
            qDebug() << "drawing";
            mSpanAngleInDegrees = 0;
            mSceneArcStartPoint = mapToScene(pencilPosition());
            scene()->initStroke();
            scene()->moveTo(mSceneArcStartPoint);
        }
        QGraphicsRectItem::mousePressEvent(event);
    }
    else
        closing = true;

    mResizeSvgItem->setVisible(mShowButtons && mResizing);
    mCloseSvgItem->setVisible(mShowButtons && closing);
    mSettingsSvgItem->setVisible(mShowButtons && setting);
    mRihtAngleSvgItem->setVisible(mShowButtons && rightAngle);
    mAngleRotateSvgItem->setVisible(mShowButtons && angleRotate);
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


    if (!mResizing && !mRotating && !mDrawing && mNormalizePos && (UBSettings::settings()->pageBackground()==UBPageBackground::crossed))
    {
        normalizePos();
        event->accept();
    }
    if (mResizing)
    {
        if (mNormalizeSize && (UBSettings::settings()->pageBackground()==UBPageBackground::crossed))
            normalizeSize();
        event->accept();
    }
    else if (mRotating)
    {
        updateResizeCursor();
        updateDrawCursor();
        event->accept();
    }
    else if (mDrawing)
    {
        updateResizeCursor();
        updateDrawCursor();
        mDrewCenterCross = false;
        event->accept();
    }
    else if (closeButtonRect().contains(event->pos()))
    {
        hide();
        event->accept();
    }
    else if (settingsButtonRect().contains(event->pos()))
    {
        showSettings();
        event->accept();
    }
    else if (rightAngleButtonRect().contains(event->pos()))
    {
        qreal rotateAngle = angleInDegrees();
        if (angleInDegrees() < 270)
        {
            int neededAngle = 90;
            while (angleInDegrees() >= neededAngle)
                neededAngle += 90;
            rotateAngle -= neededAngle;
        }
        rotateAroundNeedle(rotateAngle);
        event->accept();
    }
    else if (angleRotateButtonRect().contains(event->pos()))
    {
        mAngleRotateDialog->clearData();
        mAngleRotateDialog->show();
        event->accept();
    }
    else
    {
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

    mRotating = false;
    mResizing = false;
    mDrawing = false;

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
    mSettingsSvgItem->setParent(this);
    mRihtAngleSvgItem->setParent(this);
    mAngleRotateSvgItem->setParent(this);


    mCloseSvgItem->setVisible(mShowButtons);
    if (mShowButtons)
    {
        if (hingeRect().contains(event->pos()))
            setCursor(rotateCursor());
        else if (event->pos().x() > rect().right() - sPencilLength - sPencilBaseLength)
            setCursor(drawCursor());
        else if (resizeButtonRect().contains(event->pos()))
            setCursor(resizeCursor());
        else if (closeButtonRect().contains(event->pos())
                 || settingsButtonRect().contains(event->pos())
                 || rightAngleButtonRect().contains(event->pos())
                 || angleRotateButtonRect().contains(event->pos()))
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
    mSettingsSvgItem->setVisible(mShowButtons);
    mRihtAngleSvgItem->setVisible(mShowButtons);
    mAngleRotateSvgItem->setVisible(mShowButtons);
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
    mSettingsSvgItem->setVisible(mShowButtons);
    mRihtAngleSvgItem->setVisible(mShowButtons);
    mAngleRotateSvgItem->setVisible(mShowButtons);
    if (mShowButtons)
    {
        if (hingeRect().contains(event->pos()))
            setCursor(rotateCursor());
        else if (event->pos().x() > rect().right() - sPencilLength - sPencilBaseLength)
            setCursor(drawCursor());
        else if (resizeButtonRect().contains(event->pos()))
            setCursor(resizeCursor());
        else if (closeButtonRect().contains(event->pos())
                 || settingsButtonRect().contains(event->pos())
                 || rightAngleButtonRect().contains(event->pos())
                 || angleRotateButtonRect().contains(event->pos()))
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
            - fm.horizontalAdvance(angleText) / 2,
            - fm.height() / 2,
            fm.horizontalAdvance(angleText),
            fm.height()),
        Qt::AlignTop,
        angleText);
    painter->restore();
}

void UBGraphicsCompass::paintRadiusDisplay(QPainter *painter)
{
    double pixelsPerCentimeter = UBApplication::boardController->activeScene()->backgroundGridSize();

    qreal radiusInCentimeters = rect().width() / pixelsPerCentimeter;
    QString format = rect().width() >= sDisplayRadiusUnitMinLength ? "%1 cm" : "%1";
    QString radiusText = QString(format).arg(radiusInCentimeters, 0, 'f', 1);

    bool onPencilArm = rect().width() > sDisplayRadiusOnPencilArmMinLength;

    painter->save();
    painter->setFont(font());
    QFontMetricsF fm(painter->font());
    QPointF textCenter;

    if (onPencilArm)
        textCenter = QPointF(rect().right() - sPencilBaseLength - sPencilLength - fm.horizontalAdvance(radiusText) / 2 - 24 - 8, rect().center().y());
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
            - fm.horizontalAdvance(radiusText) / 2,
            - rect().height() / 2,
            fm.horizontalAdvance(radiusText),
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

QRectF UBGraphicsCompass::settingsButtonRect() const
{
    QPixmap settingsPixmap(":/images/settingsCompass.svg");

    QSizeF settingsRectSize(
        settingsPixmap.width() * mAntiScaleRatio,
        settingsPixmap.height() * mAntiScaleRatio);

    QPointF settingsRectTopLeft(
        rect().width()/2 - hingeRect().width()/2 - settingsRectSize.width() - 2,
        (rect().height() - settingsRectSize.height()) / 2);

    QRectF settingsRect(settingsRectTopLeft, settingsRectSize);
    settingsRect.translate(rect().topLeft());

    return settingsRect;
}

QRectF UBGraphicsCompass::rightAngleButtonRect() const
{
    QPixmap rightAnglePixmap(":/images/rightAngleCompass.svg");

    QSizeF rightAngleRectSize(
        rightAnglePixmap.width() * mAntiScaleRatio,
        rightAnglePixmap.height() * mAntiScaleRatio);

    QPointF rightAngleRectTopLeft(
        rect().width()/2 - hingeRect().width()/2 - rightAngleRectSize.width() - settingsButtonRect().width() - 6,
        (rect().height() - rightAngleRectSize.height()) / 2);

    QRectF rightAngleRect(rightAngleRectTopLeft, rightAngleRectSize);
    rightAngleRect.translate(rect().topLeft());

    return rightAngleRect;
}

QRectF UBGraphicsCompass::angleRotateButtonRect() const
{
    QPixmap angleRotatePixmap(":/images/angleRotateCompass.svg");

    QSizeF angleRotateRectSize(
        angleRotatePixmap.width() * mAntiScaleRatio,
        angleRotatePixmap.height() * mAntiScaleRatio);

    QPointF angleRotateRectTopLeft(
        rect().width()/2 + hingeRect().width()/2 + angleRotateRectSize.width()/2,
        (rect().height() - angleRotateRectSize.height()) / 2);

    QRectF angleRotateRect(angleRotateRectTopLeft, angleRotateRectSize);
    angleRotateRect.translate(rect().topLeft());

    return angleRotateRect;
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

void UBGraphicsCompass::showSettings()
{
    if (!mSettingsMenu)
    {
        mSettingsMenu = new QMenu(UBApplication::boardController->controlView());
        decorateSettingsMenu(mSettingsMenu);
    }

    UBBoardView* cv = UBApplication::boardController->controlView();
    QRect pinPos = cv->mapFromScene(mSettingsSvgItem->sceneBoundingRect()).boundingRect();

    mSettingsMenu->exec(cv->mapToGlobal(pinPos.bottomRight()));
}

void UBGraphicsCompass::decorateSettingsMenu(QMenu* menu)
{
    mNormalizePosAction = menu->addAction(tr("Normalize position"), this, SLOT(setNormalizePos(bool)));
    mNormalizePosAction->setCheckable(true);
    mNormalizePosAction->setChecked(mNormalizePos);

    mNormalizeSizeAction = menu->addAction(tr("Normalize size"), this, SLOT(setNormalizeSize(bool)));
    mNormalizeSizeAction->setCheckable(true);
    mNormalizeSizeAction->setChecked(mNormalizeSize);
    setMenuActions();
}

void UBGraphicsCompass::angleRotateDialogResult()
{
    if (mAngleRotateDialog->isAbsolutely())
        rotateAroundNeedle(angleInDegrees());
    scene()->setPreviousPoint(mapToScene(pencilPosition()));

    qreal rotateAngle = -mAngleRotateDialog->Angle();
    qreal drawingAngle = rotateAngle;
    if (!mAngleRotateDialog->isAbsolutely())
        drawingAngle -= angleInDegrees();

    mSceneArcStartPoint = mapToScene(pencilPosition());
    rotateAroundNeedle(rotateAngle);

    if (mAngleRotateDialog->isDrawing())
    {
        mSpanAngleInDegrees = rotateAngle;
        if (mSpanAngleInDegrees >= 1080)
            mSpanAngleInDegrees -= 360;
        else if (mSpanAngleInDegrees < -1080)
            mSpanAngleInDegrees += 360;
        drawArc();
    }
    updateResizeCursor();
    updateDrawCursor();
    scene()->inputDeviceRelease();
    scene()->moveTo(mSceneArcStartPoint);
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
    path.lineTo(rect().left() + sNeedleLength, rect().center().y() - sNeedleWidth/2);
    path.lineTo(rect().left() + sNeedleLength, rect().center().y() + sNeedleWidth/2);
    path.closeSubpath();
    return path;
}

QPainterPath UBGraphicsCompass::needleBaseShape() const
{
    int smallHalfSide = sNeedleBaseWidth/2 - sCornerRadius;

    QPainterPath path;
    path.moveTo(rect().left() + sNeedleLength, rect().center().y() - smallHalfSide);
    path.arcTo(
        rect().left() + sNeedleLength,
        rect().center().y() - smallHalfSide - sCornerRadius,
        sCornerRadius*2, sCornerRadius*2,
        180, -90);
    path.lineTo(rect().left() + sNeedleLength + sNeedleBaseLength, rect().center().y() - sNeedleBaseWidth/2);
    path.lineTo(rect().left() + sNeedleLength + sNeedleBaseLength, rect().center().y() + sNeedleBaseWidth/2);
    path.lineTo(rect().left() + sNeedleLength + sCornerRadius, rect().center().y() + smallHalfSide + sCornerRadius);
    path.arcTo(
        rect().left() + sNeedleLength,
        rect().center().y() + smallHalfSide - sCornerRadius,
        sCornerRadius*2, sCornerRadius*2,
        -90, -90);
    path.closeSubpath();

    return path;
}

QPainterPath UBGraphicsCompass::needleArmShape() const
{
    int smallHalfSide = sNeedleArmLeftWidth/2 - sCornerRadius;

    QPainterPath path;
    path.moveTo(rect().left() + sNeedleLength + sNeedleBaseLength, rect().center().y() - smallHalfSide);
    path.arcTo(
        rect().left() + sNeedleLength + sNeedleBaseLength,
        rect().center().y() - sNeedleArmLeftWidth/2,
        sCornerRadius*2, sCornerRadius*2,
        180, -90);
    path.lineTo(hingeRect().left(), rect().center().y() - sNeedleArmRigthWidth/2);
    path.lineTo(hingeRect().left(), rect().center().y() + sNeedleArmRigthWidth/2);
    path.lineTo(rect().left() + sNeedleLength + sNeedleBaseLength + sCornerRadius, rect().center().y() + sNeedleArmLeftWidth/2);
    path.arcTo(
        rect().left() + sNeedleLength + sNeedleBaseLength,
        rect().center().y() + smallHalfSide - sCornerRadius,
        sCornerRadius*2, sCornerRadius*2,
        -90, -90);
    path.closeSubpath();
    return path;
}

QPainterPath UBGraphicsCompass::hingeShape() const
{
    QPainterPath path;
    path.moveTo(hingeRect().left() + sCornerRadius, hingeRect().top());
    path.lineTo(hingeRect().right() - sCornerRadius, hingeRect().top());
    path.arcTo(
        hingeRect().right() - sCornerRadius*2,
        hingeRect().top(),
        sCornerRadius*2, sCornerRadius*2,
        90, -90);
    path.lineTo(hingeRect().right(), hingeRect().bottom() - sCornerRadius);
    path.arcTo(
        hingeRect().right() - sCornerRadius*2,
        hingeRect().bottom() - sCornerRadius*2,
        sCornerRadius*2, sCornerRadius*2,
        0, -90);
    path.lineTo(hingeRect().left() + sCornerRadius, hingeRect().bottom());
    path.arcTo(
        hingeRect().left(),
        hingeRect().bottom() - sCornerRadius*2,
        sCornerRadius*2, sCornerRadius*2,
        -90, -90);
    path.lineTo(hingeRect().left(), hingeRect().top() + sCornerRadius);
    path.arcTo(
        hingeRect().left(),
        hingeRect().top(),
        sCornerRadius*2, sCornerRadius*2,
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
    path.moveTo(rect().right() - sPencilLength - sPencilBaseLength, rect().center().y() - sPencilBaseWidth/2);
    path.lineTo(rect().right() - sPencilLength - sCornerRadius, rect().center().y() - sPencilBaseWidth/2);
    path.arcTo(
        rect().right() - sPencilLength - sCornerRadius*2, rect().center().y() - sPencilBaseWidth/2,
        sCornerRadius*2, sCornerRadius*2,
        90, -90);
    path.lineTo(rect().right() - sPencilLength, rect().center().y() + sPencilBaseWidth/2 - sCornerRadius);
    path.arcTo(
        rect().right() - sPencilLength - sCornerRadius*2, rect().center().y() + sPencilBaseWidth/2 - sCornerRadius*2,
        sCornerRadius*2, sCornerRadius*2,
        0, -90);
    path.lineTo(rect().right() - sPencilLength - sPencilBaseLength, rect().center().y() + sPencilBaseWidth/2);
    path.closeSubpath();

    return path;
}

QPainterPath UBGraphicsCompass::pencilArmShape() const
{
    QPainterPath path;
    path.moveTo(hingeRect().right(), rect().center().y() - sPencilArmLeftWidth/2);
    path.lineTo(rect().right() - sPencilLength - sPencilBaseLength - sCornerRadius, rect().center().y() - sPencilArmRightWidth/2);
    path.arcTo(
        rect().right() - sPencilLength - sPencilBaseLength - sCornerRadius*2, rect().center().y() - sPencilArmRightWidth/2,
        sCornerRadius*2, sCornerRadius*2,
        90, -90);
    path.lineTo(rect().right() - sPencilLength - sPencilBaseLength, rect().center().y() + sPencilArmRightWidth/2 - sCornerRadius);
    path.arcTo(
        rect().right() - sPencilLength - sPencilBaseLength - sCornerRadius*2, rect().center().y() + sPencilArmRightWidth/2 - sCornerRadius*2,
        sCornerRadius*2, sCornerRadius*2,
        0, -90);
    path.lineTo(hingeRect().right(), rect().center().y() + sPencilArmLeftWidth/2);
    path.closeSubpath();
    return path;
}

std::shared_ptr<UBGraphicsScene> UBGraphicsCompass::scene() const
{
    auto scenePtr = dynamic_cast<UBGraphicsScene*>(QGraphicsRectItem::scene());
    return scenePtr ? scenePtr->shared_from_this() : nullptr;
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

void UBGraphicsCompass::setNormalizePos(bool isNormalize)
{
    mNormalizePos = isNormalize;
    if (isNormalize)
        normalizePos();
}
void UBGraphicsCompass::setNormalizeSize(bool isNormalize)
{
    mNormalizeSize = isNormalize;
    if (isNormalize)
        normalizeSize();
}

void UBGraphicsCompass::setMenuActions()
{
    mNormalizePosAction->setEnabled(UBSettings::settings()->pageBackground()==UBPageBackground::crossed);
    mNormalizeSizeAction->setEnabled(UBSettings::settings()->pageBackground()==UBPageBackground::crossed);
}

void UBGraphicsCompass::normalizePos()
{
    QPointF newPos = nearPointFromGrid(mapToScene(needlePosition()));
    qreal curAngle = angleInDegrees();
    if (curAngle!=0)
        rotateAroundNeedle(curAngle);
    setRect(newPos.x(), newPos.y() - rect().height() / 2, rect().width(), rect().height());
    setPos(0,0);
    if (curAngle !=0)
        rotateAroundNeedle(-curAngle);
}

void UBGraphicsCompass::normalizeSize()
{
    int gridSize = UBSettings::settings()->crossSize;
    int divGrid = rect().width()/gridSize;
    if ((rect().toRect().width()%gridSize)>(gridSize/2))
        ++divGrid;
    setRect(QRectF(rect().topLeft(), QSizeF(gridSize*divGrid,rect().height())));
}

QPointF UBGraphicsCompass::nearPointFromGrid(QPointF point)
{
    QPointF result = point;
    QList<QPointF> gridPoints;
    int sceneHeight = UBApplication::boardController->activeScene()->sceneSize().height(),
        sceneWidth = UBApplication::boardController->activeScene()->sceneSize().width(),
        gridSize = UBSettings::settings()->crossSize;
    for (int i = 0; i >= sceneWidth/-2; i -= gridSize)
    {
        for (int j = 0; j >= sceneHeight/-2; j -=gridSize)
            gridPoints.push_back(QPointF(i,j));
        for (int j = 0; j <= sceneHeight/2; j +=gridSize)
            gridPoints.push_back(QPointF(i,j));
    }
    for (int i = 0; i <= sceneWidth/2; i += gridSize)
    {
        for (int j = 0; j >= sceneHeight/-2; j -=gridSize)
            gridPoints.push_back(QPointF(i,j));
        for (int j = 0; j <= sceneHeight/2; j +=gridSize)
            gridPoints.push_back(QPointF(i,j));
    }
    if (gridPoints.count() > 0)
    {
        result = gridPoints[0];
        QLineF checkLine = QLineF(point, result);
        float length = qSqrt(qPow(checkLine.dx(),2) + qPow(checkLine.dy(),2));
        for (int i = 1; i < gridPoints.count(); ++i)
        {
            checkLine = QLineF(point, gridPoints[i]);
            float checkLength = qSqrt(qPow(checkLine.dx(),2) + qPow(checkLine.dy(),2));
            if (checkLength<length)
            {
                length = checkLength;
                result = gridPoints[i];
            }
        }
    }
    return result;
}


UBCompassAngleRotateDialog::UBCompassAngleRotateDialog(UBGraphicsCompass* compass,QWidget *parent) :
    QDialog(parent)
{
    mCompass = compass;
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    window()->setWindowTitle(tr("Compass angle rotation"));
    window()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    mAngleSpinBox = new QDoubleSpinBox(this);
    mAngleSpinBox->setMaximum(360);
    mAngleSpinBox->setMinimum(-360);
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    QLabel* angleLabel = new QLabel(this);
    angleLabel->setText(tr("Angle:"));
    angleLabel->setAlignment(Qt::AlignVCenter);
    angleLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    hLayout->addWidget(angleLabel);
    hLayout->addWidget(mAngleSpinBox);
    hLayout->addSpacerItem(new QSpacerItem(26,20, QSizePolicy::Expanding, QSizePolicy::Preferred));
    QWidget* angleFrame = new QWidget(this);
    angleFrame->setLayout(hLayout);
    angleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(angleFrame);

    mDrawingCheckBox = new QCheckBox(this);
    mDrawingCheckBox->setText(tr("With drawing"));
    mDrawingCheckBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QWidget* drawingCheckBoxWidget = new QWidget(this);
    drawingCheckBoxWidget->setLayout(new QHBoxLayout(this));
    drawingCheckBoxWidget->layout()->addWidget(mDrawingCheckBox);
    mainLayout->addWidget(drawingCheckBoxWidget);

    mainLayout->addSpacerItem(new QSpacerItem(26,20, QSizePolicy::Preferred, QSizePolicy::Expanding));

    QWidget* buttonsWidget = new QWidget(this);
    QPushButton* absolutButton = new QPushButton(this);
    absolutButton->setText(tr("Rotate absolutely"));
    absolutButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QHBoxLayout* hLayout2 = new QHBoxLayout(this);
    hLayout2->addWidget(absolutButton);
    hLayout2->addSpacerItem(new QSpacerItem(26,20, QSizePolicy::Expanding, QSizePolicy::Preferred));
    QPushButton* relativeButton = new QPushButton(this);
    relativeButton->setText(tr("Rotate relative"));
    relativeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    hLayout2->addWidget(relativeButton);
    buttonsWidget->setLayout(hLayout2);
    mainLayout->addWidget(buttonsWidget);
    setLayout(mainLayout);

    connect(absolutButton, SIGNAL(clicked()), this, SLOT(RotateAbsolutely()));
    connect(relativeButton, SIGNAL(clicked()), this, SLOT(RotateRelative()));
}

UBCompassAngleRotateDialog::~UBCompassAngleRotateDialog()
{
    delete mDrawingCheckBox;
    delete mAngleSpinBox;
}

void UBCompassAngleRotateDialog::RotateAbsolutely()
{
    mIsAbsolutely = true;
    mCompass->angleRotateDialogResult();
    emit accept();
}

void UBCompassAngleRotateDialog::RotateRelative()
{
    mIsAbsolutely = false;
    mCompass->angleRotateDialogResult();
    emit accept();
}

bool UBCompassAngleRotateDialog::isDrawing()
{
    return mDrawingCheckBox->isChecked();
}

bool UBCompassAngleRotateDialog::isAbsolutely()
{
    return mIsAbsolutely;
}

qreal UBCompassAngleRotateDialog::Angle()
{
    return mAngleSpinBox->value();
}

void UBCompassAngleRotateDialog::clearData()
{
    mDrawingCheckBox->setChecked(false);
    mAngleSpinBox->setValue(0);
}
