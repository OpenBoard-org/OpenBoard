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

#ifndef UBGRAPHICSTEXTITEMUNDOCOMMAND_H_
#define UBGRAPHICSTEXTITEMUNDOCOMMAND_H_

#include <QtGui>
#include "UBAbstractUndoCommand.h"

#include "UBGraphicsTextItem.h"


class UBGraphicsTextItemUndoCommand : public UBAbstractUndoCommand
{
    public:
        UBGraphicsTextItemUndoCommand(UBGraphicsTextItem *textItem);
        virtual ~UBGraphicsTextItemUndoCommand();

        virtual UndoType getType() { return undotype_GRAPHICTEXTITEM; };

    protected:
        virtual void undo();
        virtual void redo();

    private:
        QPointer<UBGraphicsTextItem> mTextItem;
};

#endif /* UBGRAPHICSTEXTITEMUNDOCOMMAND_H_ */
