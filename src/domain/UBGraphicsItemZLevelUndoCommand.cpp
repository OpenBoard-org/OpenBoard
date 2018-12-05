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


#include "UBGraphicsItemZLevelUndoCommand.h"

UBGraphicsItemZLevelUndoCommand::UBGraphicsItemZLevelUndoCommand(UBGraphicsScene *_scene, const QList<QGraphicsItem*>& _items, qreal _previousZLevel, UBZLayerController::moveDestination dest):UBUndoCommand(){
    Q_ASSERT(_scene != NULL);
    mpScene = _scene;
    mItems = _items;
    mPreviousZLevel = _previousZLevel;
    mDest = dest;
    mHack = false;
}

UBGraphicsItemZLevelUndoCommand::UBGraphicsItemZLevelUndoCommand(UBGraphicsScene *_scene, QGraphicsItem* _item, qreal _previousZLevel, UBZLayerController::moveDestination dest):UBUndoCommand(){
    Q_ASSERT(_scene != NULL);
    mpScene = _scene;
    if(NULL != _item)
        mItems.append(_item);

    mPreviousZLevel = _previousZLevel;
    mDest = dest;
    mHack = false;
}

UBGraphicsItemZLevelUndoCommand::~UBGraphicsItemZLevelUndoCommand(){

}

void UBGraphicsItemZLevelUndoCommand::undo(){
    if(!mpScene || mItems.empty())
        return;

    // Getting the difference between the initial z-value and the actual one
    qreal zDiff = qAbs(mItems.at(mItems.size()-1)->data(UBGraphicsItemData::ItemOwnZValue).toReal() - mPreviousZLevel);

    if(mDest == UBZLayerController::down || mDest == UBZLayerController::bottom){
        // Move up
        QList<QGraphicsItem*>::iterator downIt = mItems.end();
        for(; downIt >= mItems.begin(); downIt--){
            for(int i=0; i<zDiff; i++)
                mpScene->changeZLevelTo(*downIt, UBZLayerController::up);
        }

    }else if(mDest == UBZLayerController::up || mDest == UBZLayerController::top){
        // Move down
        foreach(QGraphicsItem* item, mItems){
            for(int i=0; i<zDiff; i++)
                mpScene->changeZLevelTo(item, UBZLayerController::down);
        }
    }

}

void UBGraphicsItemZLevelUndoCommand::redo(){
    if(!mHack){
        // Ugly! But pushing a new command to QUndoStack calls redo by itself.
        mHack = true;
    }else{
        if(!mpScene)
            return;

        foreach(QGraphicsItem* item, mItems){
            mpScene->changeZLevelTo(item, mDest);
            updateLazyScene();
        }
    }
}

void UBGraphicsItemZLevelUndoCommand::updateLazyScene(){
    mpScene->update(mpScene->sceneRect());
    mpScene->updateSelectionFrame();
}
