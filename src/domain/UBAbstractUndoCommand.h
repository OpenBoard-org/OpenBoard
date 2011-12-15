/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef UBABSTRACTUNDOCOMMAND_H_
#define UBABSTRACTUNDOCOMMAND_H_

#include <QtGui>

class UBAbstractUndoCommand : public QUndoCommand
{
    public:

        UBAbstractUndoCommand();
        ~UBAbstractUndoCommand();

        enum UndoType
        {
            undotype_UNKNOWN               = 0,
            undotype_DOCUMENT              = 1,
            undotype_GRAPHICITEMTRANSFORM  = 2,
            undotype_GRAPHICITEM           = 3,
            undotype_GRAPHICTEXTITEM       = 4,
            undotype_PAGESIZE              = 5
        };

        virtual UndoType getType() { return undotype_UNKNOWN; }

    protected:
        virtual void undo();
        virtual void redo();

};

#endif /* UBABSTRACTUNDOCOMMAND_H_ */
