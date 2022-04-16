/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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


#ifndef UBEXPORTDOCUMENTSETADAPTOR_H
#define UBEXPORTDOCUMENTSETADAPTOR_H

#include <QtCore>
#include "UBExportAdaptor.h"
#include "frameworks/UBFileSystemUtils.h"
#include "globals/UBGlobals.h"

#include "quazip.h"
#include "quazipfile.h"

class UBDocumentProxy;
class UBDocumentTreeModel;


class UBExportDocumentSetAdaptor : public UBExportAdaptor
{
    Q_OBJECT

    public:
        UBExportDocumentSetAdaptor(QObject *parent = 0);
        virtual ~UBExportDocumentSetAdaptor();

        virtual QString exportName();
        virtual QString exportExtention();

        virtual void persist(UBDocumentProxy* pDocument);
        bool persistData(const QModelIndex &pRootIndex, QString filename);
        bool addDocumentToZip(const QModelIndex &pIndex, UBDocumentTreeModel *model, QuaZip &zip);

        virtual bool associatedActionactionAvailableFor(const QModelIndex &selectedIndex);


};

#endif // UBEXPORTDOCUMENTSETADAPTOR_H
