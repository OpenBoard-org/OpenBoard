#ifndef UB1HANDLEEDITABLE_H
#define UB1HANDLEEDITABLE_H

#include "UBShapeEditable.h"

class UB1HEditableGraphicsBasicShapeItem : public UBAbstractEditableGraphicsShapeItem
{
public:
    UB1HEditableGraphicsBasicShapeItem(QGraphicsItem *parent = 0);

    UBAbstractHandle *getHandle() const
    {
        return mHandles.first();
    }

    QRectF adjustBoundingRect(QRectF rect) const;
};

#endif // UB1HANDLEEDITABLE_H
