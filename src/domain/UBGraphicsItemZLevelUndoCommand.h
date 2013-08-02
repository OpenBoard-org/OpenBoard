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
