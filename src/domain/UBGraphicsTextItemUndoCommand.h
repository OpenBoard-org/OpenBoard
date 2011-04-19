/*
 * UBGraphicsTextItemUndoCommand.h
 *
 *  Created on: July 7, 2009
 *      Author: Jerome
 */

#ifndef UBGRAPHICSTEXTITEMUNDOCOMMAND_H_
#define UBGRAPHICSTEXTITEMUNDOCOMMAND_H_

#include <QtGui>

#include "UBGraphicsTextItem.h"


class UBGraphicsTextItemUndoCommand: public QUndoCommand
{
    public:
        UBGraphicsTextItemUndoCommand(UBGraphicsTextItem *textItem);
        virtual ~UBGraphicsTextItemUndoCommand();

    protected:
        virtual void undo();
        virtual void redo();

    private:
        QPointer<UBGraphicsTextItem> mTextItem;
};

#endif /* UBGRAPHICSTEXTITEMUNDOCOMMAND_H_ */
