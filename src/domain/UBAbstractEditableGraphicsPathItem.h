#ifndef UBABSTRACTEDITABLEGRAPHICSPATHITEM_H
#define UBABSTRACTEDITABLEGRAPHICSPATHITEM_H

#include "UBEditable.h"
#include "UBAbstractGraphicsPathItem.h"

class UBAbstractEditableGraphicsPathItem : public UBAbstractEditable, public UBAbstractGraphicsPathItem
{
public:
    UBAbstractEditableGraphicsPathItem(QGraphicsItem *parent = 0);

    virtual QRectF boundingRect() const;

    virtual QPainterPath shape() const;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void focusHandle(UBAbstractHandle *handle);
    virtual void deactivateEditionMode();
    virtual void onActivateEditionMode();

    virtual void drawArrows();


    int mMultiClickState;

    bool mHasMoved;
};

#endif // UBABSTRACTEDITABLEGRAPHICSPATHITEM_H
