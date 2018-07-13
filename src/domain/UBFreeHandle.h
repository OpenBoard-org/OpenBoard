#ifndef UBFREEHANDLE_H
#define UBFREEHANDLE_H

#include "UBAbstractHandle.h"

class UBFreeHandle : public UBAbstractHandle
{
public:
    UBFreeHandle();

    UBFreeHandle(UBFreeHandle* const src);

    UBItem *deepCopy() const;
    void copyItemParameters(UBItem *copy) const;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // UBFREEHANDLE_H
