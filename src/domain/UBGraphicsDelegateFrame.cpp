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

#include "UBGraphicsDelegateFrame.h"

#include <QtGui>
#include <QtSvg>

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "domain/UBGraphicsItemDelegate.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsProxyWidget.h"

#include "gui/UBResources.h"

#include "core/memcheck.h"

UBGraphicsDelegateFrame::UBGraphicsDelegateFrame(UBGraphicsItemDelegate* pDelegate, QRectF pRect, qreal pFrameWidth, bool respectRatio)
    : QGraphicsRectItem(), QObject(pDelegate)
    , mCurrentTool(None)
    , mDelegate(pDelegate)
    , mVisible(true)
    , mFrameWidth(pFrameWidth)
    , mNominalFrameWidth(pFrameWidth)
    , mRespectRatio(respectRatio)
    , mAngle(0)
    , mAngleOffset(0)
    , mTotalScaleX(-1)
    , mTotalScaleY(-1)
    , mTranslateX(0)
    , mTranslateY(0)
    , mTotalTranslateX(0)
    , mTotalTranslateY(0)
    , mOperationMode(Scaling)
    , mMirrorX(false)
    , mMirrorY(false)
{
    mAngleTolerance = UBSettings::settings()->angleTolerance->get().toReal();

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setAcceptedMouseButtons(Qt::LeftButton);
    setRect(pRect.adjusted(mFrameWidth, mFrameWidth, mFrameWidth * -1, mFrameWidth * -1));

    setBrush(QBrush(UBSettings::paletteColor));
    setPen(Qt::NoPen);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mBottomRightResizeGripSvgItem = new QGraphicsSvgItem(":/images/resize.svg", this);
    mBottomResizeGripSvgItem = new QGraphicsSvgItem(":/images/resizeBottom.svg", this);
    mLeftResizeGripSvgItem = new QGraphicsSvgItem(":/images/resizeLeft.svg", this);
    mRightResizeGripSvgItem = new QGraphicsSvgItem(":/images/resizeRight.svg", this);
    mTopResizeGripSvgItem = new QGraphicsSvgItem(":/images/resizeTop.svg", this);

    mBottomRightResizeGrip = new QGraphicsRectItem(this);
    mBottomRightResizeGrip->setPen(Qt::NoPen);
    mBottomResizeGrip = new QGraphicsRectItem(this);
    mBottomResizeGrip->setPen(Qt::NoPen);
    mLeftResizeGrip = new QGraphicsRectItem(this);
    mLeftResizeGrip->setToolTip("left");
    mLeftResizeGrip->setPen(Qt::NoPen);
    mRightResizeGrip = new QGraphicsRectItem(this);
    mRightResizeGrip->setPen(Qt::NoPen);
    mRightResizeGrip->setToolTip("Right");
    mTopResizeGrip = new QGraphicsRectItem(this);
    mTopResizeGrip->setPen(Qt::NoPen);

    mRotateButton = new QGraphicsSvgItem(":/images/rotate.svg", this);
    mRotateButton->setCursor(UBResources::resources()->rotateCursor);
    mRotateButton->setVisible(mDelegate->canRotate());

    updateResizeCursors();

    setAntiScale(1.0);

    positionHandles();

    this->setAcceptHoverEvents(true);

    angleWidget = new UBAngleWidget();
}


UBGraphicsDelegateFrame::~UBGraphicsDelegateFrame()
{
delete angleWidget;
    // NOOP
}

void UBGraphicsDelegateFrame::setAntiScale(qreal pAntiScale)
{
    mFrameWidth = mNominalFrameWidth * pAntiScale;

    QTransform tr;
    tr.scale(pAntiScale, pAntiScale);

    mBottomRightResizeGripSvgItem->setTransform(tr);
    mBottomResizeGripSvgItem->setTransform(tr);
    mLeftResizeGripSvgItem->setTransform(tr);
    mRightResizeGripSvgItem->setTransform(tr);
    mTopResizeGripSvgItem->setTransform(tr);
    mRotateButton->setTransform(tr);
}


void UBGraphicsDelegateFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    path.addRoundedRect(rect(), mFrameWidth / 2, mFrameWidth / 2);

    if (rect().width() > 1 && rect().height() > 1)
    {
        QPainterPath extruded;
        extruded.addRect(rect().adjusted(mFrameWidth, mFrameWidth, (mFrameWidth * -1), (mFrameWidth * -1)));
        path = path.subtracted(extruded);
    }

    painter->fillPath(path, brush());
}


QPainterPath UBGraphicsDelegateFrame::shape() const
{
    QPainterPath path;

    //We do not use the rounded rect here because we want the bottom right corner
    //to be included in the frame (for resize grip handling : #702)
    path.addRect(rect());

    if (rect().width() > 0 && rect().height() > 0)
    {
        QPainterPath extruded;
        extruded.addRect(rect().adjusted(mFrameWidth, mFrameWidth, mFrameWidth * -1, mFrameWidth * -1));
        path = path.subtracted(extruded);
    }

    return path;
}


void UBGraphicsDelegateFrame::initializeTransform()
{
    QTransform itemTransform = delegated()->sceneTransform();
    QRectF itemRect = delegated()->boundingRect();
    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QPointF  bottomLeft = itemTransform.map(itemRect.bottomLeft());

    qreal horizontalFlip = (topLeft.x() > topRight.x()) ? -1 : 1;
    mMirrorX = horizontalFlip < 0 ;
    if(horizontalFlip < 0){
        // why this is because of the way of calculating the translations that checks which side is the most is the
        // nearest instead of checking which one is the left side.
        QPointF tmp = topLeft;
        topLeft = topRight;
        topRight = tmp;

        // because of the calculation of the height is done by lenght and not deltaY
        bottomLeft = itemTransform.map(itemRect.bottomRight());
    }

    qreal verticalFlip = (bottomLeft.y() < topLeft.y()) ? -1 : 1;
    // not sure that is usefull
    mMirrorY = verticalFlip < 0;
    if(verticalFlip < 0 && !mMirrorX){
        topLeft = itemTransform.map(itemRect.bottomLeft());
        topRight = itemTransform.map(itemRect.bottomRight());
        bottomLeft = itemTransform.map(itemRect.topLeft());
    }

    QLineF topLine(topLeft, topRight);
    QLineF leftLine(topLeft, bottomLeft);
    qreal width = topLine.length();
    qreal height = leftLine.length();

    mAngle = topLine.angle();

    // the fact the the length is used we loose the horizontalFlip information
    // a better way to do this is using DeltaX that preserve the direction information.
    mTotalScaleX = (width / itemRect.width()) * horizontalFlip;
    mTotalScaleY = height / itemRect.height() * verticalFlip;

    QTransform tr;
    QPointF center = delegated()->boundingRect().center();
    tr.translate(center.x() * mTotalScaleX, center.y() * mTotalScaleY);
    tr.rotate(-mAngle);
    tr.translate(-center.x() * mTotalScaleX, -center.y() * mTotalScaleY);
    tr.scale(mTotalScaleX, mTotalScaleY);

    mTotalTranslateX = delegated()->transform().dx() - tr.dx();
    mTotalTranslateY = delegated()->transform().dy() - tr.dy();
}


void UBGraphicsDelegateFrame::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mDelegate->startUndoStep();

    mStartingPoint = event->scenePos();

    initializeTransform();

    mScaleX = 1;
    mScaleY = 1;
    mTranslateX = 0;
    mTranslateY = 0;
    mAngleOffset = 0;

    mInitialTransform = buildTransform();

    mCurrentTool = toolFromPos(event->pos());

    event->accept();
}

bool UBGraphicsDelegateFrame::canResizeBottomRight(qreal width, qreal height, qreal scaleFactor)
{
    bool res = false;

    if(!mMirrorX && !mMirrorX && ((width * scaleFactor) > 2*mFrameWidth && (height * scaleFactor) > 2*mFrameWidth)){
        res = true;
    }else if(mMirrorX && !mMirrorY && (-width * scaleFactor) > 2*mFrameWidth && (height*scaleFactor) > 2*mFrameWidth){
        res = true;
    }else if(!mMirrorX && mMirrorY && (width * scaleFactor) > 2*mFrameWidth && (-height*scaleFactor) > 2*mFrameWidth){
        res = true;
    }else if(mMirrorX && mMirrorY && (-width * scaleFactor) > 2*mFrameWidth && (-height*scaleFactor) > 2*mFrameWidth){
        res = true;
    }

    return res;
}

void UBGraphicsDelegateFrame::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QLineF move(mStartingPoint, event->scenePos());
    qreal moveX = move.length() * cos((move.angle() - mAngle) * PI / 180);
    qreal moveY = -move.length() * sin((move.angle() - mAngle) * PI / 180);
    qreal width = delegated()->boundingRect().width() * mTotalScaleX;
    qreal height = delegated()->boundingRect().height() * mTotalScaleY;

    if(mOperationMode == Scaling)
    {
        mTranslateX = moveX;
        // Perform the resize
        if (resizingBottomRight())
        {
            // -----------------------------------------------------
            // ! We want to keep the aspect ratio with this resize !
            // -----------------------------------------------------
            qreal scaleX;
            qreal scaleY;

            if(!mMirrorX){
                scaleX = (width + moveX) / width;
            }else{
                scaleX = (width - moveX) / width;
            }

            if(!mMirrorY){
                scaleY = (height + moveY) / height;
            }else{
                scaleY = (height - moveY) / height;
            }

            qreal scaleFactor = (scaleX + scaleY) / 2;

            // Do not allow resizing of image size under frame size
            if (canResizeBottomRight(width, height, scaleFactor))
            {
                if (mRespectRatio)
                {
                    mScaleX = scaleFactor;
                    mScaleY = scaleFactor;
                }
                else
                {
                    mScaleX = scaleX;
                    mScaleY = scaleY;
                }
            }
        }else if (resizingLeft() || resizingRight())
        {
            if(width != 0){
                qreal scaleX = 0.0;
                if(resizingLeft()){
                    scaleX = (width - moveX) / width;
                }else if(resizingRight()){
                    scaleX = (width + moveX) / width;
                }
                if(mDelegate->isFlippable() && qAbs(scaleX) != 0){
                    if((qAbs(width * scaleX)) < 2*mFrameWidth){
                        bool negative = (scaleX < 0)?true:false;
                        if(negative){
                            if(mMirrorX)
                                scaleX = 2*mFrameWidth/width;
                            else
                                scaleX = -2*mFrameWidth/width;
                        }else{
                            scaleX = -1;
                        }
                    }
                    mScaleX = scaleX;
                }else if (scaleX > 1 || (width * scaleX) > 2 * mFrameWidth){
                    mScaleX = scaleX;
                    if(resizingLeft()){
                        mTranslateX = moveX;
                    }
                }
            }
        }else if(resizingTop() || resizingBottom()){
            if(height != 0){
                qreal scaleY = 0.0;
                if(resizingTop()){
                    scaleY = (height - moveY) / height;
                }else if(resizingBottom()){
                    scaleY = (height + moveY) / height;
                }

                if(mDelegate->isFlippable() && qAbs(scaleY) != 0){
                    if((qAbs(height * scaleY)) < 2*mFrameWidth){
                        bool negative = (scaleY < 0)?true:false;
                        if(negative){
                            if(mMirrorY)
                                scaleY = 2*mFrameWidth/width;
                            else
                                scaleY = -2*mFrameWidth/width;
                        }else{
                            scaleY = -1;
                        }
                    }
                    mScaleY = scaleY;
                }else if (scaleY > 1 || (height * scaleY) > 2 * mFrameWidth)
                {
                    mScaleY = scaleY;
                    if(resizingTop()){
                        mTranslateY = moveY;
                    }
                }
            }
        }
    }
    else if (mOperationMode == Resizing)
    {
        mTranslateX = moveX;
        UBResizableGraphicsItem* resizableItem = dynamic_cast<UBResizableGraphicsItem*>(delegated());

        if (resizableItem)
        {
            QLineF mousePosDelta(delegated()->mapFromScene(event->lastScenePos())
                    , delegated()->mapFromScene(event->scenePos()));
            QSizeF incVector(0, 0);

            if (resizingBottomRight())
            {
                incVector = QSizeF(mousePosDelta.dx(), mousePosDelta.dy());
            }
            else if (resizingRight())
            {
                incVector = QSizeF(mousePosDelta.dx(), 0);
            }
            else if (resizingBottom())
            {
                incVector = QSizeF(0, mousePosDelta.dy());
            }
            else if (resizingLeft())
            {
                incVector = QSizeF(- mousePosDelta.dx(), 0);
            }
            else if (resizingTop())
            {
                incVector = QSizeF(0, - mousePosDelta.dy());
            }

            QSizeF newSize = resizableItem->size() + incVector;

            if (!(mDelegate->getToolBarItem()->isVisibleOnBoard()
                && (newSize.width() < mDelegate->getToolBarItem()->minWidth() / mDelegate->antiScaleRatio()
                || newSize.height() < mDelegate->getToolBarItem()->minWidth() / mDelegate->antiScaleRatio() * 3/4)))
                    resizableItem->resize(newSize);
        }
    }

    if (rotating())
    {
        mTranslateX = 0;
        mTranslateY = 0;

        QLineF startLine(sceneBoundingRect().center(), event->lastScenePos());
        QLineF currentLine(sceneBoundingRect().center(), event->scenePos());
        mAngle += startLine.angleTo(currentLine);

        if ((int)mAngle % 45 >= 45 - mAngleTolerance || (int)mAngle % 45 <= mAngleTolerance)
        {
            mAngle = qRound(mAngle / 45) * 45;
            mAngleOffset += startLine.angleTo(currentLine);
            if ((int)mAngleOffset % 360 > mAngleTolerance && (int)mAngleOffset % 360 < 360 - mAngleTolerance)
            {
                mAngle += mAngleOffset;
                mAngleOffset = 0;
            }
        }
        else if ((int)mAngle % 30 >= 30 - mAngleTolerance || (int)mAngle % 30 <= mAngleTolerance)
        {
            mAngle = qRound(mAngle / 30) * 30;
            mAngleOffset += startLine.angleTo(currentLine);
            if ((int)mAngleOffset % 360 > mAngleTolerance && (int)mAngleOffset % 360 < 360 - mAngleTolerance)
            {
                mAngle += mAngleOffset;
                mAngleOffset = 0;
            }
        }

        if (!angleWidget->isVisible())
            angleWidget->show();

        angleWidget->setText(QString::number((int)mAngle % 360));
        angleWidget->update();

    }
    else if (moving())
    {
        mTranslateX = move.dx();
        mTranslateY = move.dy();
    }

    QTransform tr = buildTransform();

    //TODO UB 4.x: Could find a better solution ?
    if (resizingRight() || resizingBottom() || resizingBottomRight())
    {
        QPointF ref;
        if(!mMirrorX && !mMirrorY){
            ref = delegated()->boundingRect().topLeft();
        }else if(mMirrorX && !mMirrorY){
            ref = delegated()->boundingRect().topLeft();
        }else if(!mMirrorX && mMirrorY){
            ref = delegated()->boundingRect().topLeft();
        }else if(mMirrorX && mMirrorY){
            ref = delegated()->boundingRect().topRight();
        }

        // Map the item topleft point to the current mouse move transform
        QPointF topLeft = tr.map(ref);

        // Map the item topleft point to the mouse press transform
        QPointF fixedPoint = mInitialTransform.map(ref);

        // Update the translation coordinates
        mTranslateX += fixedPoint.x() - topLeft.x();
        mTranslateY += fixedPoint.y() - topLeft.y();

        // Update the transform
        tr = buildTransform();
    }
    else if (resizingTop() || resizingLeft())
    {
        if (mOperationMode == Scaling)
        {
            QPointF bottomRight = tr.map(delegated()->boundingRect().bottomRight());
            QPointF fixedPoint = mInitialTransform.map(delegated()->boundingRect().bottomRight());
            mTranslateX += fixedPoint.x() - bottomRight.x();
            mTranslateY += fixedPoint.y() - bottomRight.y();
        }
        else
        {
            QLineF vector;
            if (resizingLeft())
            {
                QPointF topRight1 = mInitialTransform.map(QPointF(delegated()->boundingRect().width() - moveX, 0));
                QPointF topRight2 = mInitialTransform.map(QPointF(delegated()->boundingRect().width(), 0));
                vector.setPoints(topRight1, topRight2);
            }
            else
            {
                QPointF bottomLeft1 = mInitialTransform.map(QPointF(0, delegated()->boundingRect().height() - moveY));
                QPointF bottomLeft2 = mInitialTransform.map(QPointF(0, delegated()->boundingRect().height()));
                vector.setPoints(bottomLeft1, bottomLeft2);
            }
            mTranslateX = vector.dx();
            mTranslateY = vector.dy();
        }
        tr = buildTransform();
    }

    delegated()->setTransform(tr);
    event->accept();
}


QTransform UBGraphicsDelegateFrame::buildTransform()
{
    QTransform tr;
    QPointF center = delegated()->boundingRect().center();

    // Translate
    tr.translate(mTotalTranslateX + mTranslateX, mTotalTranslateY + mTranslateY);

    // Set angle
    tr.translate(center.x() * mTotalScaleX * mScaleX, center.y() * mTotalScaleY * mScaleY);
    tr.rotate(-mAngle);
    tr.translate(-center.x() * mTotalScaleX * mScaleX, -center.y() * mTotalScaleY * mScaleY);

    // Scale
    tr.scale(mTotalScaleX * mScaleX, mTotalScaleY * mScaleY);
    return tr;
}


void UBGraphicsDelegateFrame::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (angleWidget->isVisible())
        angleWidget->hide();

    updateResizeCursors();

    mDelegate->commitUndoStep();
    mTotalScaleX *= mScaleX;
    mTotalScaleY *= mScaleY;
    mTotalTranslateX += mTranslateX;
    mTotalTranslateY += mTranslateY;
    event->accept();

    mCurrentTool = None;
    QGraphicsRectItem::mouseReleaseEvent(event);

    // Show the buttons
    if(isResizing()){
        mResizing = false;
    }
    mDelegate->setButtonsVisible(true);
}


void UBGraphicsDelegateFrame::updateResizeCursors()
{
    QPixmap pix(":/images/cursors/resize.png");
    QTransform tr;

    tr.rotate(-mAngle);
    QCursor resizeCursor  = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
    mLeftResizeGrip->setCursor(resizeCursor);
    mRightResizeGrip->setCursor(resizeCursor);

    tr.rotate(-90);
    resizeCursor  = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
    mBottomResizeGrip->setCursor(resizeCursor);
    mTopResizeGrip->setCursor(resizeCursor);

    tr.rotate(-45);
    resizeCursor  = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
    mBottomRightResizeGrip->setCursor(resizeCursor);
}


void UBGraphicsDelegateFrame::setVisible(bool visible)
{
    mVisible = visible;
    if (mVisible)
       setBrush(QBrush(UBSettings::paletteColor));
    else
       setBrush(Qt::NoBrush);
}


void UBGraphicsDelegateFrame::positionHandles()
{
    QRectF itemRect = delegated()->boundingRect();
    
    if (mDelegate->getToolBarItem()->isVisibleOnBoard()
        && mDelegate->getToolBarItem()->isShifting())
        itemRect.setHeight(itemRect.height() + mDelegate->getToolBarItem()->rect().height() * mDelegate->antiScaleRatio() * 1.1);

    QTransform itemTransform = delegated()->sceneTransform();
    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QPointF bottomLeft = itemTransform.map(itemRect.bottomLeft());
    QPointF bottomRight = itemTransform.map(itemRect.bottomRight());
    QPointF center = itemTransform.map(itemRect.center());
    int rotateHeight = QLineF(topLeft, bottomLeft).length();

    // Handle the mirroring
    if(topLeft.x() > topRight.x()){
        QPointF topTmp = topRight;
        QPointF bottomTmp = bottomRight;
        topRight = topLeft;
        topLeft = topTmp;
        bottomRight = bottomLeft;
        bottomLeft = bottomTmp;
    }

    if(bottomLeft.y() > topLeft.y()){
        QPointF leftTmp = bottomLeft;
        QPointF rightTmp = bottomRight;
        bottomLeft = topLeft;
        topLeft = leftTmp;
        bottomRight = topRight;
        topRight = rightTmp;
    }

    QLineF topLine(topLeft, topRight);
    qreal angle = topLine.angle();
    qreal width = topLine.length();

    QLineF leftLine(topLeft, bottomLeft);
    qreal height = leftLine.length();

    int h = rotating()?rotateHeight:height;

    if (mVisible)
    {
        setRect(center.x() - mFrameWidth - width / 2, center.y() - mFrameWidth - h / 2, width + 2 * mFrameWidth, h + 2 * mFrameWidth);
    }
    else
    {
        setRect(center.x() - width / 2, center.y() - h / 2, width, h);
    }

    resetTransform();
    translate(center.x(), center.y());
    rotate(-angle);
    translate(-center.x(), -center.y());

    mBottomRightResizeGripSvgItem->setParentItem(this);
    mBottomResizeGripSvgItem->setParentItem(this);
    mLeftResizeGripSvgItem->setParentItem(this);
    mRightResizeGripSvgItem->setParentItem(this);
    mTopResizeGripSvgItem->setParentItem(this);
    mRotateButton->setParentItem(this);

    mBottomRightResizeGrip->setParentItem(this);
    mBottomResizeGrip->setParentItem(this);
    mLeftResizeGrip->setParentItem(this);
    mRightResizeGrip->setParentItem(this);
    mTopResizeGrip->setParentItem(this);

    QRectF brRect = mBottomRightResizeGripSvgItem->mapRectToParent(mBottomRightResizeGripSvgItem->boundingRect());
    QRectF bRect = mBottomResizeGripSvgItem->mapRectToParent(mBottomResizeGripSvgItem->boundingRect());
    QRectF lRect = mLeftResizeGripSvgItem->mapRectToParent(mLeftResizeGripSvgItem->boundingRect());
    QRectF rRect = mRightResizeGripSvgItem->mapRectToParent(mRightResizeGripSvgItem->boundingRect());
    QRectF trRect = mTopResizeGripSvgItem->mapRectToParent(mTopResizeGripSvgItem->boundingRect());

    mBottomRightResizeGripSvgItem->setPos(rect().right() - brRect.width(), rect().bottom() - brRect.height());
    mBottomResizeGripSvgItem->setPos(rect().center().x() - bRect.width() / 2, rect().bottom() - bRect.height());

    mLeftResizeGripSvgItem->setPos(rect().left(), rect().center().y() - lRect.height() / 2);
    mRightResizeGripSvgItem->setPos(rect().right() - rRect.width(), rect().center().y() - rRect.height() / 2);

    mTopResizeGripSvgItem->setPos(rect().center().x() - trRect.width() / 2, rect().y());
    mRotateButton->setPos(rect().right() - mFrameWidth - 5, rect().top() + 5);

    mBottomRightResizeGrip->setRect(bottomRightResizeGripRect());
    mBottomResizeGrip->setRect(bottomResizeGripRect());
    mLeftResizeGrip->setRect(leftResizeGripRect());
    mRightResizeGrip->setRect(rightResizeGripRect());
    mTopResizeGrip->setRect(topResizeGripRect());

    QVariant vLocked = delegated()->data(UBGraphicsItemData::ItemLocked);
    bool isLocked = (vLocked.isValid() && vLocked.toBool());

    mBottomRightResizeGripSvgItem->setVisible(!isLocked);
    mBottomResizeGripSvgItem->setVisible(!isLocked);
    mLeftResizeGripSvgItem->setVisible(!isLocked);
    mRightResizeGripSvgItem->setVisible(!isLocked);
    mTopResizeGripSvgItem->setVisible(!isLocked);
    mRotateButton->setVisible(mDelegate->canRotate() && !isLocked);

    mBottomRightResizeGrip->setVisible(!isLocked);
    mBottomResizeGrip->setVisible(!isLocked);
    mLeftResizeGrip->setVisible(!isLocked);
    mRightResizeGrip->setVisible(!isLocked);
    mTopResizeGrip->setVisible(!isLocked);

    if (isLocked)
    {
        QColor baseColor = UBSettings::paletteColor;
        baseColor.setAlphaF(baseColor.alphaF() / 3);
        setBrush(QBrush(baseColor));
    }
    else
    {
        setBrush(QBrush(UBSettings::paletteColor));
    }

    //make frame interact like delegated item when selected. Maybe should be deleted if selection logic will change
    setZValue(delegated()->zValue());
}


QGraphicsItem* UBGraphicsDelegateFrame::delegated()
{
    return mDelegate->delegated();
}

UBGraphicsDelegateFrame::FrameTool UBGraphicsDelegateFrame::toolFromPos(QPointF pos)
{
        if(mDelegate->isLocked())
                return None;
    else if (bottomRightResizeGripRect().contains(pos))
        return ResizeBottomRight;
    else if (bottomResizeGripRect().contains(pos)){
            if(mMirrorY){
                return ResizeTop;
            }else{
                return ResizeBottom;
            }
        }
    else if (leftResizeGripRect().contains(pos)){
            if(mMirrorX){
                return ResizeRight;
            }else{
                return ResizeLeft;
            }
            return ResizeLeft;
        }
    else if (rightResizeGripRect().contains(pos)){
            if(mMirrorX){
                return ResizeLeft;
            }else{
                return ResizeRight;
            }
        }
    else if (topResizeGripRect().contains(pos)){
            if(mMirrorY){
                return ResizeBottom;
            }else{
                return ResizeTop;
            }
        }
    else if (rotateButtonBounds().contains(pos) && mDelegate && mDelegate->canRotate())
        return Rotate;
    else
        return Move;
}


QRectF UBGraphicsDelegateFrame::bottomRightResizeGripRect() const
{
    return QRectF(rect().right() - mFrameWidth, rect().bottom() - mFrameWidth, mFrameWidth, mFrameWidth);
}


QRectF UBGraphicsDelegateFrame::bottomResizeGripRect() const
{
    return QRectF(rect().center().x() - mFrameWidth / 2, rect().bottom() - mFrameWidth, mFrameWidth, mFrameWidth);
}


QRectF UBGraphicsDelegateFrame::leftResizeGripRect() const
{
    return QRectF(rect().left(), rect().center().y() - mFrameWidth / 2, mFrameWidth, mFrameWidth);
}


QRectF UBGraphicsDelegateFrame::rightResizeGripRect() const
{
    return QRectF(rect().right() - mFrameWidth, rect().center().y() - mFrameWidth / 2, mFrameWidth, mFrameWidth);
}


QRectF UBGraphicsDelegateFrame::topResizeGripRect() const
{
    return QRectF(rect().center().x() - mFrameWidth / 2, rect().top(), mFrameWidth, mFrameWidth);
}


QRectF UBGraphicsDelegateFrame::rotateButtonBounds() const
{
    return QRectF(rect().right()- mFrameWidth, rect().top(), mFrameWidth, mFrameWidth);
}

void UBGraphicsDelegateFrame::refreshGeometry()
{
    // Here we want to have the left on the left, the right on the right, the top on the top and the bottom on the bottom!
    QRectF itemRect = delegated()->boundingRect();
    QTransform itemTransform = delegated()->sceneTransform();
    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QPointF bottomLeft = itemTransform.map(itemRect.bottomLeft());

    QLineF topLine(topLeft, topRight);
    qreal width = topLine.length();
    QLineF leftLine(topLeft, bottomLeft);
    qreal height = leftLine.length();
    setRect(topRight.x() - mFrameWidth, topLeft.y() - mFrameWidth, width + 2*mFrameWidth, height + 2*mFrameWidth);
}
