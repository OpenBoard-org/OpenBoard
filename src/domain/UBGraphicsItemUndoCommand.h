/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UBGRAPHICSITEMUNDOCOMMAND_H_
#define UBGRAPHICSITEMUNDOCOMMAND_H_

#include <QtGui>
#include "UBAbstractUndoCommand.h"
#include "UBGraphicsGroupContainerItem.h"


class UBGraphicsScene;


class UBGraphicsItemUndoCommand : public UBAbstractUndoCommand
{
    public:
        UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, const QSet<QGraphicsItem*>& pRemovedItems,
                                  const QSet<QGraphicsItem*>& pAddedItems, const QMultiMap<UBGraphicsGroupContainerItem*, QUuid> &groupsMap = QMultiMap<UBGraphicsGroupContainerItem*, QUuid>());

        UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, QGraphicsItem* pRemovedItem,
                        QGraphicsItem* pAddedItem);

        virtual ~UBGraphicsItemUndoCommand();

        QSet<QGraphicsItem*> GetAddedList() { return mAddedItems; }
        QSet<QGraphicsItem*> GetRemovedList() { return mRemovedItems; }

        virtual UndoType getType() { return undotype_GRAPHICITEM; }

    protected:
        virtual void undo();
        virtual void redo();

    private:
        UBGraphicsScene* mScene;
        QSet<QGraphicsItem*> mRemovedItems;
        QSet<QGraphicsItem*> mAddedItems;
        QMultiMap<UBGraphicsGroupContainerItem*, QUuid> mExcludedFromGroup;

        bool mFirstRedo;
};

#endif /* UBGRAPHICSITEMUNDOCOMMAND_H_ */
