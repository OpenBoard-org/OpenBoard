#ifndef UBDIAGONALHANDLE_H
#define UBDIAGONALHANDLE_H

#include "UBAbstractHandle.h"

class UBDiagonalHandle : public UBAbstractHandle
{
public:
    UBDiagonalHandle();

    UBDiagonalHandle(UBDiagonalHandle* const src);

    UBItem *deepCopy() const;
    void copyItemParameters(UBItem *copy) const;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // UBDIAGONALHANDLE_H
