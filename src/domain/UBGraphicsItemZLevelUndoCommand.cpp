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
    if(!mpScene)
        return;

    foreach(QGraphicsItem* item, mItems){
        if(mDest == UBZLayerController::down){
            mpScene->changeZLevelTo(item, UBZLayerController::up);
        }else if(mDest == UBZLayerController::up){
            mpScene->changeZLevelTo(item, UBZLayerController::down);
        }
        updateLazyScene();
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
