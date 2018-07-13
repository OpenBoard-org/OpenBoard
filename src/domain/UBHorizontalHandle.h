#ifndef UBHORIZONTALHANDLE_H
#define UBHORIZONTALHANDLE_H

#include "UBAbstractHandle.h"

class UBHorizontalHandle : public UBAbstractHandle
{
public:
    UBHorizontalHandle();

    UBHorizontalHandle(UBHorizontalHandle* const src);

    UBItem *deepCopy() const;
    void copyItemParameters(UBItem *copy) const;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // UBHORIZONTALHANDLE_H
