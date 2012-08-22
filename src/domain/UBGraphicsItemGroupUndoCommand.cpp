#include "UBGraphicsItemGroupUndoCommand.h"

#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsScene.h"
#include "core/memcheck.h"


UBGraphicsItemGroupUndoCommand::UBGraphicsItemGroupUndoCommand(UBGraphicsScene *pScene, UBGraphicsGroupContainerItem *pGroupCreated) :
    mScene (pScene), mGroup(pGroupCreated), mFirstRedo(true)

{
    if (pGroupCreated->childItems().count()) {
        foreach (QGraphicsItem *item, pGroupCreated->childItems()) {
            mItems << item;
        }
    }
}

UBGraphicsItemGroupUndoCommand::~UBGraphicsItemGroupUndoCommand()
{
}

void UBGraphicsItemGroupUndoCommand::undo()
{
    mGroup->destroy();
    foreach(QGraphicsItem *item, mItems) {
        item->setSelected(true);
    }
}

void UBGraphicsItemGroupUndoCommand::redo()
{
    if (mFirstRedo) {
        //Work around. TODO determine why does Qt call the redo function on pushing to undo
        mFirstRedo = false;
        return;
    }

    foreach (QGraphicsItem *item, mItems) {
        if (item->type() == UBGraphicsGroupContainerItem::Type) {
            QList<QGraphicsItem*> childItems = item->childItems();
            UBGraphicsGroupContainerItem *currentGroup = dynamic_cast<UBGraphicsGroupContainerItem*>(item);
            if (currentGroup) {
                currentGroup->destroy();
            }
            foreach (QGraphicsItem *chItem, childItems) {
                mGroup->addToGroup(chItem);
            }
        } else {
            mGroup->addToGroup(item);
        }
    }

    mScene->addItem(mGroup);
    mGroup->setVisible(true);
    mGroup->setFocus();
    mGroup->setSelected(true);
}
