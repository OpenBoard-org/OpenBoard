#include "UBAbstractEditableGraphicsPathItem.h"

#include "UBFreeHandle.h"

UBAbstractEditableGraphicsPathItem::UBAbstractEditableGraphicsPathItem(QGraphicsItem *parent):
    UBAbstractGraphicsPathItem(parent)
{
    mMultiClickState = 0;
    mHasMoved = false;
}

void UBAbstractEditableGraphicsPathItem::onActivateEditionMode()
{
    //NOOP
}

void UBAbstractEditableGraphicsPathItem::drawArrows()
{
    UBAbstractGraphicsPathItem::drawArrows();

    // Draw Arrows UNDER (z-order) Handles, if handles are shown.
    if (isInEditMode())
    {
        if (startArrowGraphicsItem()){
            startArrowGraphicsItem()->stackBefore(mHandles.first());
        }

        if (endArrowGraphicsItem()){
            endArrowGraphicsItem()->stackBefore(mHandles.last());
            endArrowGraphicsItem()->stackBefore(mHandles.first());
        }
    }
}

void UBAbstractEditableGraphicsPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mMultiClickState++;
    mHasMoved = false;

    UBAbstractGraphicsPathItem::mousePressEvent(event);
}

void UBAbstractEditableGraphicsPathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    prepareGeometryChange();

    if(!mHasMoved)
    {
        if (!Delegate()->isLocked())
        {
            if(mMultiClickState %2 == 1){
                onActivateEditionMode();

                /*Delegate()->showFrame(false);*/
                setFocus();
                showEditMode(true);
            }
            else
            {
                showEditMode(false);
                Delegate()->positionHandles();
                /*Delegate()->showFrame(true);*/
            }
        }
    }
    else
    {
        if(!isInEditMode()){
            mMultiClickState = 0;
        }else{
            mMultiClickState--;
        }
    }

    UBAbstractGraphicsPathItem::mouseReleaseEvent(event);

    mHasMoved = false;
}

QRectF UBAbstractEditableGraphicsPathItem::boundingRect() const
{
    QRectF rect = path().boundingRect();

    rect = UBAbstractGraphicsPathItem::adjustBoundingRect(rect);

    if(isInEditMode()){
        qreal r = mHandles.first()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UBAbstractEditableGraphicsPathItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!Delegate()->isLocked())
    {
        mHasMoved = true;

        if(!isInEditMode()){
            Delegate()->mouseMoveEvent(event);
            UBAbstractGraphicsPathItem::mouseMoveEvent(event);
        }
    }
}

void UBAbstractEditableGraphicsPathItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if(mMultiClickState %2 == 1){
        prepareGeometryChange();
        mMultiClickState = 0;
        showEditMode(false);
    }
}

void UBAbstractEditableGraphicsPathItem::focusHandle(UBAbstractHandle *handle)
{
    Q_UNUSED(handle)

    /*Delegate()->showFrame(false);*/
}

void UBAbstractEditableGraphicsPathItem::deactivateEditionMode()
{
    prepareGeometryChange();

    mMultiClickState = 0;
    showEditMode(false);
}

QPainterPath UBAbstractEditableGraphicsPathItem::shape() const
{
    QPainterPath path;
    if(mMultiClickState %2 == 1){
        path.addRect(boundingRect());
        return path;
    }else{
        return this->path();
    }
}
