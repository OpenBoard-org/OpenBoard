/*
 * UBPageSizeUndoCommand.h
 *
 *  Created on: Jul 17, 2009
 *      Author: Patrick
 */

#ifndef UBPageSizeUndoCommand_H_
#define UBPageSizeUndoCommand_H_

#include <QtGui>

class UBGraphicsScene;


class UBPageSizeUndoCommand : public QUndoCommand
{
    public:
        UBPageSizeUndoCommand(UBGraphicsScene* pScene, const QSize& previousSize, const QSize& newSize);
        virtual ~UBPageSizeUndoCommand();

    protected:
        virtual void undo();
        virtual void redo();

    private:
        UBGraphicsScene* mScene;
        QSize mPreviousSize;
        QSize mNewSize;

        bool mFirstRedo;
};

#endif /* UBPageSizeUndoCommand_H_ */
