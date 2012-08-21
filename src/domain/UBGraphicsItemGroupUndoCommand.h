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
