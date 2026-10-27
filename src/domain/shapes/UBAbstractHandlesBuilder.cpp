/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBAbstractHandlesBuilder.h"

#include "UBDiagonalHandle.h"
#include "UBHorizontalHandle.h"
#include "UBVerticalHandle.h"

void UB1HandleBuilder::buildHandles(QVector<UBAbstractHandle*>& handles)
{
    // before clean the vector
    if (handles.size() > 0)
    {
        qDeleteAll(handles);
        handles.clear();
    }

    UBDiagonalHandle* dh = new UBDiagonalHandle();
    UBDiagonalHandle* sh = new UBDiagonalHandle();

    sh->setId(Stretch);

    dh->hide();
    sh->hide();

    handles.push_back(dh);
    handles.push_back(sh);
}

void UB3HandlesBuilder::buildHandles(QVector<UBAbstractHandle*>& handles)
{
    // before clean the vector
    if (handles.size() > 0)
    {
        qDeleteAll(handles);
        handles.clear();
    }

    UBHorizontalHandle* hh = new UBHorizontalHandle();
    UBVerticalHandle* vh = new UBVerticalHandle();
    UBDiagonalHandle* dh = new UBDiagonalHandle();
    UBDiagonalHandle* sh = new UBDiagonalHandle();

    sh->setId(Stretch);

    hh->hide();
    vh->hide();
    dh->hide();
    sh->hide();

    handles.push_back(hh);
    handles.push_back(vh);
    handles.push_back(dh);
    handles.push_back(sh);
}
