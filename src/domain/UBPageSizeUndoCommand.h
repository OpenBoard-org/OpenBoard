/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBPageSizeUndoCommand_H_
#define UBPageSizeUndoCommand_H_

#include <QtGui>
#include "UBAbstractUndoCommand.h"

class UBGraphicsScene;


class UBPageSizeUndoCommand : public UBAbstractUndoCommand
{
    public:
        UBPageSizeUndoCommand(UBGraphicsScene* pScene, const QSize& previousSize, const QSize& newSize);
        virtual ~UBPageSizeUndoCommand();

        virtual UndoType getType() { return undotype_PAGESIZE; };

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
