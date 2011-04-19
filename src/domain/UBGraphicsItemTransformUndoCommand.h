/*
 * UBGraphicsItemTransformUndoCommand.h
 *
 *  Created on: July 7, 2009
 *      Author: Jerome
 */

#ifndef UBGRAPHICSITEMTRANSFORMUNDOCOMMAND_H_
#define UBGRAPHICSITEMTRANSFORMUNDOCOMMAND_H_

#include <QtGui>

#include "UBResizableGraphicsItem.h"


class UBGraphicsItemTransformUndoCommand : public QUndoCommand
{
    public:
        UBGraphicsItemTransformUndoCommand(QGraphicsItem* pItem,
                                                const QPointF& prevPos,
                                                const QTransform& prevTransform,
                                                const qreal& prevZValue,
                                                const QSizeF& prevSize = QSizeF());
        virtual ~UBGraphicsItemTransformUndoCommand();

    protected:
        virtual void undo();
        virtual void redo();

    private:
        QGraphicsItem* mItem;
        QTransform mPreviousTransform;
        QTransform mCurrentTransform;
        QPointF mPreviousPosition;
        QPointF mCurrentPosition;
        QSizeF mPreviousSize;
        QSizeF mCurrentSize;

        qreal mPreviousZValue;
        qreal mCurrentZValue;

};

#endif /* UBGRAPHICSITEMTRANSFORMUNDOCOMMAND_H_ */
