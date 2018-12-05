/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef UBGRAPHICSITEMZLEVELUNDOCOMMAND_H
#define UBGRAPHICSITEMZLEVELUNDOCOMMAND_H

#include <QGraphicsItem>

#include "UBUndoCommand.h"
#include "UBGraphicsScene.h"

class UBGraphicsItemZLevelUndoCommand : public UBUndoCommand{
public:
    UBGraphicsItemZLevelUndoCommand(UBGraphicsScene* _scene, const QList<QGraphicsItem*>& _items, qreal _previousZLevel, UBZLayerController::moveDestination dest);
    UBGraphicsItemZLevelUndoCommand(UBGraphicsScene* _scene, QGraphicsItem* _item, qreal _previousZLevel, UBZLayerController::moveDestination dest);
    ~UBGraphicsItemZLevelUndoCommand();

    virtual int getType() const { return UBUndoType::undotype_GRAPHICITEMZVALUE; }

protected:
    virtual void undo();
    virtual void redo();

private:
    void updateLazyScene();

    qreal mPreviousZLevel;
    QList<QGraphicsItem*> mItems;
    UBGraphicsScene* mpScene;
    UBZLayerController::moveDestination mDest;
    bool mHack;
};


#endif // UBGRAPHICSITEMZLEVELUNDOCOMMAND_H
