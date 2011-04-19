/*
 * UBGraphicsItemUndoCommand.h
 *
 *  Created on: Sep 16, 2008
 *      Author: luc
 */

#ifndef UBGRAPHICSITEMUNDOCOMMAND_H_
#define UBGRAPHICSITEMUNDOCOMMAND_H_

#include <QtGui>

class UBGraphicsScene;


class UBGraphicsItemUndoCommand : public QUndoCommand
{
    public:
        UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, const QSet<QGraphicsItem*>& pRemovedItems,
                const QSet<QGraphicsItem*>& pAddedItems);

        UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, QGraphicsItem* pRemovedItem,
                        QGraphicsItem* pAddedItem);

        virtual ~UBGraphicsItemUndoCommand();

    protected:
        virtual void undo();
        virtual void redo();

    private:
        UBGraphicsScene* mScene;
        QSet<QGraphicsItem*> mRemovedItems;
        QSet<QGraphicsItem*> mAddedItems;

        bool mFirstRedo;
};

#endif /* UBGRAPHICSITEMUNDOCOMMAND_H_ */
