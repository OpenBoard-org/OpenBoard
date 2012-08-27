/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBGRAPHICSITEMUNDOCOMMAND_H_
#define UBGRAPHICSITEMUNDOCOMMAND_H_

#include <QtGui>
#include "UBAbstractUndoCommand.h"


class UBGraphicsScene;


class UBGraphicsItemUndoCommand : public UBAbstractUndoCommand
{
    public:
        UBGraphicsItemUndoCommand(UBGraphicsScene* pScene, const QSet<QGraphicsItem*>& pRemovedItems,
                const QSet<QGraphicsItem*>& pAddedItems);

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

        bool mFirstRedo;
};

#endif /* UBGRAPHICSITEMUNDOCOMMAND_H_ */
