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
            undotype_PAGESIZE              = 5,
            undotype_GRAPHICSGROUPITEM     = 6
        };

        virtual UndoType getType() { return undotype_UNKNOWN; }

    protected:
        virtual void undo();
        virtual void redo();

};

#endif /* UBABSTRACTUNDOCOMMAND_H_ */
