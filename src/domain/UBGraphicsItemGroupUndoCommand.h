/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBGRAPHICSITEMGROUPUNDOCOMMAND_H
#define UBGRAPHICSITEMGROUPUNDOCOMMAND_H

#include <QList>
#include "UBAbstractUndoCommand.h"

class UBGraphicsScene;
class UBGraphicsGroupContainerItem;

class UBGraphicsItemGroupUndoCommand : public UBAbstractUndoCommand
{
public:
    UBGraphicsItemGroupUndoCommand(UBGraphicsScene *pScene, UBGraphicsGroupContainerItem *pGroupCreated);
    virtual ~UBGraphicsItemGroupUndoCommand();

    virtual UndoType getType() { return undotype_GRAPHICSGROUPITEM; }

protected:
    virtual void undo();
    virtual void redo();

private:
    UBGraphicsScene *mScene;
    UBGraphicsGroupContainerItem *mGroup;
    QList<QGraphicsItem*> mItems;

    bool mFirstRedo;
};

#endif // UBGRAPHICSITEMGROUPUNDOCOMMAND_H
