/*
 * UBDocumentUndoCommand.h
 *
 *  Created on: Sep 19, 2008
 *      Author: luc
 */

#ifndef UBDOCUMENTUNDOCOMMAND_H_
#define UBDOCUMENTUNDOCOMMAND_H_

#include <QtGui>

class UBDocumentProxy;
class UBGraphicsScene;


class UBDocumentUndoCommand: public QUndoCommand
{
    public:
        UBDocumentUndoCommand(UBDocumentProxy* pDocument, const QList<UBGraphicsScene*>& pOldScenes,
                const QList<UBGraphicsScene*>& pNewScenes, const int& pActiveSceneIndex);
        virtual ~UBDocumentUndoCommand();

    protected:

        virtual void undo();
        virtual void redo();

    private:

        UBDocumentProxy* mDocument;
        QList<UBGraphicsScene*> mUndoScenes;
        QList<UBGraphicsScene*> mRedoScenes;
        int mActiveSceneIndex;
};

#endif /* UBDOCUMENTUNDOCOMMAND_H_ */
