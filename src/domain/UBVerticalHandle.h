#ifndef UBVERTICALHANDLE_H
#define UBVERTICALHANDLE_H

#include "UBAbstractHandle.h"

class UBVerticalHandle : public UBAbstractHandle
{
public:
    UBVerticalHandle();

    UBVerticalHandle(UBVerticalHandle* const src);

    UBItem *deepCopy() const;
    void copyItemParameters(UBItem *copy) const;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // UBVERTICALHANDLE_H
