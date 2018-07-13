#include "UBShapeEditable.h"

UBAbstractEditableGraphicsShapeItem::UBAbstractEditableGraphicsShapeItem(QGraphicsItem *parent):
    UBAbstractGraphicsItem(parent)
{
    mMultiClickState = 0;
    mHasMoved = false;
}

void UBAbstractEditableGraphicsShapeItem::onActivateEditionMode()
{
    //NOOP
}

void UBAbstractEditableGraphicsShapeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mMultiClickState++;
    mHasMoved = false;

    UBAbstractGraphicsItem::mousePressEvent(event);
}

void UBAbstractEditableGraphicsShapeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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

    UBAbstractGraphicsItem::mouseReleaseEvent(event);

    mHasMoved = false;
}

void UBAbstractEditableGraphicsShapeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!Delegate()->isLocked())
    {
        mHasMoved = true;

        if(!isInEditMode()){            
            UBAbstractGraphicsItem::mouseMoveEvent(event);
            Delegate()->mouseMoveEvent(event);
        }
    }
}

void UBAbstractEditableGraphicsShapeItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if(mMultiClickState %2 == 1){
        prepareGeometryChange();
        mMultiClickState = 0;
        showEditMode(false);
    }
}

void UBAbstractEditableGraphicsShapeItem::deactivateEditionMode()
{
    prepareGeometryChange();

    mMultiClickState = 0;
    showEditMode(false);
}
