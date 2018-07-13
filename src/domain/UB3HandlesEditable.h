#ifndef UB3HANDLESEDITABLE_H
#define UB3HANDLESEDITABLE_H

#include "UBShapeEditable.h"

class UB3HEditablesGraphicsBasicShapeItem : public UBAbstractEditableGraphicsShapeItem
{
public:
    UB3HEditablesGraphicsBasicShapeItem(QGraphicsItem *parent = 0);

    UBAbstractHandle *horizontalHandle() const
    {
        return mHandles.at(0);
    }

    UBAbstractHandle *verticalHandle() const
    {
        return mHandles.at(1);
    }

    UBAbstractHandle *diagonalHandle() const
    {
        return mHandles.at(2);
    }

    QRectF adjustBoundingRect(QRectF rect) const;
};

#endif // UB3HANDLESEDITABLE_H
