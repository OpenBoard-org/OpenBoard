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

#ifndef UBREGULARSHAPESPALETTE_H
#define UBREGULARSHAPESPALETTE_H

#include <QButtonGroup>

#include "UBAbstractSubPalette.h"

class UBRegularShapesPalette : public UBAbstractSubPalette
{
    Q_OBJECT

    public:
        enum NVertices
        {
            Triangle = 3,
            Carre,
            Pentagone,
            Hexagone,
            Heptagone,
            Octogone
        };

        UBRegularShapesPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);
        UBRegularShapesPalette(Qt::Orientation orient, QWidget *parent = 0 );

        virtual ~UBRegularShapesPalette();
        virtual void triggerAction(QAction *action);

    private slots:
        void buttonClicked();
};

#endif // UBREGULARSHAPESPALETTE_H
