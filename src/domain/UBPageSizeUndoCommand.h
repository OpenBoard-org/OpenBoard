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
