/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UBDOCUMENTUNDOCOMMAND_H_
#define UBDOCUMENTUNDOCOMMAND_H_

#include <QtGui>
#include "UBAbstractUndoCommand.h"

class UBDocumentProxy;
class UBGraphicsScene;

class UBDocumentUndoCommand: public UBAbstractUndoCommand
{
    public:
        UBDocumentUndoCommand(UBDocumentProxy* pDocument, const QList<UBGraphicsScene*>& pOldScenes,
                const QList<UBGraphicsScene*>& pNewScenes, const int& pActiveSceneIndex);
        virtual ~UBDocumentUndoCommand();

        virtual UndoType getType() { return undotype_DOCUMENT; };

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
