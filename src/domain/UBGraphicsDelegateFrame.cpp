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

qreal const UBGraphicsDelegateFrame::mAngleTolerance = 6;

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
{
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
    mLeftResizeGrip->setPen(Qt::NoPen);
    mRightResizeGrip = new QGraphicsRectItem(this);
    mRightResizeGrip->setPen(Qt::NoPen);
    mTopResizeGrip = new QGraphicsRectItem(this);
    mTopResizeGrip->setPen(Qt::NoPen);

    mRotateButton = new QGraphicsSvgItem(":/images/rotate.svg", this);
    mRotateButton->setCursor(UBResources::resources()->rotateCursor);
    mRotateButton->setVisible(mDelegate->canRotate());

    updateResizeCursors();

    setAntiScale(1.0);

    positionHandles();
}


UBGraphicsDelegateFrame::~UBGraphicsDelegateFrame()
{
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

    QLineF topLine(topLeft, topRight);
    QLineF leftLine(topLeft, bottomLeft);
    qreal width = topLine.length();
    qreal height = leftLine.length();

    mAngle = topLine.angle();
    mTotalScaleX = width / itemRect.width();
    mTotalScaleY = height / itemRect.height();

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


void UBGraphicsDelegateFrame::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QLineF move(mStartingPoint, event->scenePos());
    qreal moveX = move.length() * cos((move.angle() - mAngle) * PI / 180);
    qreal moveY = -move.length() * sin((move.angle() - mAngle) * PI / 180);
    qreal width = delegated()->boundingRect().width() * mTotalScaleX;
    qreal height = delegated()->boundingRect().height() * mTotalScaleY;

    if(mOperationMode == Scaling)
    {
        if (resizingBottomRight())
        {
            qreal scaleX = (width + moveX) / width;
            qreal scaleY = (height + moveY) / height;
            qreal scaleFactor = (scaleX + scaleY) / 2;

            // Do not allow resizing of image size under frame size
            if (scaleFactor > 1
                || ((width * scaleFactor) > 2 * mFrameWidth
                && (height * scaleFactor) > 2 * mFrameWidth))
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
        }
        else if (resizingLeft())
        {
            qreal scaleX = (width - moveX) / width;
            if (scaleX > 1 || (width * scaleX) > 2 * mFrameWidth)
            {
                mScaleX = scaleX;
                mTranslateX = moveX;
            }
        }
        else if (resizingRight())
        {
            qreal scaleX = (width + moveX) / width;
            if (scaleX > 1 || (width * scaleX) > 2 * mFrameWidth)
            {
                mScaleX = scaleX;
            }
        }
        else if (resizingTop())
        {
            qreal scaleY = (height - moveY) / height;
            if (scaleY > 1 || (height * scaleY) > 2 * mFrameWidth)
            {
                mScaleY = scaleY;
                mTranslateY = moveY;
            }
        }
        else if (resizingBottom())
        {
            qreal scaleY = (height + moveY) / height;
            if (scaleY > 1 || (height * scaleY) > 2 * mFrameWidth)
            {
                mScaleY = scaleY;
            }
        }
    }
    else if (mOperationMode == Resizing)
    {
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

            if (newSize.width() < 50 /*0*/ || newSize.height() < /*0*/ 50)
                return;

            resizableItem->resize(newSize);
        }
    }

    if (rotating())
    {
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
        QPointF topLeft = tr.map(delegated()->boundingRect().topLeft());
        QPointF fixedPoint = mInitialTransform.map(delegated()->boundingRect().topLeft());
        mTranslateX += fixedPoint.x() - topLeft.x();
        mTranslateY += fixedPoint.y() - topLeft.y();
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
    tr.translate(mTotalTranslateX + mTranslateX, mTotalTranslateY + mTranslateY);
    tr.translate(center.x() * mTotalScaleX * mScaleX, center.y() * mTotalScaleY * mScaleY);
    tr.rotate(-mAngle);
    tr.translate(-center.x() * mTotalScaleX * mScaleX, -center.y() * mTotalScaleY * mScaleY);
    tr.scale(mTotalScaleX * mScaleX, mTotalScaleY * mScaleY);
    return tr;
}


void UBGraphicsDelegateFrame::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    updateResizeCursors();

    mDelegate->commitUndoStep();
    mTotalScaleX *= mScaleX;
    mTotalScaleY *= mScaleY;
    mTotalTranslateX += mTranslateX;
    mTotalTranslateY += mTranslateY;
    event->accept();

    mCurrentTool = None;
    QGraphicsRectItem::mouseReleaseEvent(event);
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
    QTransform itemTransform = delegated()->sceneTransform();
    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QPointF bottomLeft = itemTransform.map(itemRect.bottomLeft());
    QPointF center = itemTransform.map(itemRect.center());

    QLineF topLine(topLeft, topRight);
    qreal angle = topLine.angle();
    qreal width = topLine.length();
    QLineF leftLine(topLeft, bottomLeft);
    qreal height = leftLine.length();

    if (mVisible)
    {
        setRect(center.x() - mFrameWidth - width / 2, center.y() - mFrameWidth - height / 2,
                width + 2 * mFrameWidth, height + 2 * mFrameWidth);
    }
    else
    {
        setRect(center.x() - width / 2, center.y() - height / 2, width, height);
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
    mBottomRightResizeGripSvgItem->setPos(rect().right() - brRect.width()
            , rect().bottom() - brRect.height());

    QRectF bRect = mBottomResizeGripSvgItem->mapRectToParent(mBottomResizeGripSvgItem->boundingRect());

    mBottomResizeGripSvgItem->setPos(rect().center().x() - bRect.width() / 2
            , rect().bottom() - bRect.height());

    QRectF lRect = mLeftResizeGripSvgItem->mapRectToParent(mLeftResizeGripSvgItem->boundingRect());
    mLeftResizeGripSvgItem->setPos(rect().left()
            , rect().center().y() - lRect.height() / 2);

    QRectF rRect = mRightResizeGripSvgItem->mapRectToParent(mRightResizeGripSvgItem->boundingRect());
    mRightResizeGripSvgItem->setPos(rect().right() - rRect.width()
            , rect().center().y() - rRect.height() / 2);

    QRectF trRect = mTopResizeGripSvgItem->mapRectToParent(mTopResizeGripSvgItem->boundingRect());
    mTopResizeGripSvgItem->setPos(rect().center().x() - trRect.width() / 2
            , rect().y());

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
    else if (bottomResizeGripRect().contains(pos))
        return ResizeBottom;
    else if (leftResizeGripRect().contains(pos))
        return ResizeLeft;
    else if (rightResizeGripRect().contains(pos))
        return ResizeRight;
    else if (topResizeGripRect().contains(pos))
        return ResizeTop;
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


