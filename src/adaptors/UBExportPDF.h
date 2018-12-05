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




#ifndef UBEXPORTPDF_H_
#define UBEXPORTPDF_H_

#include <QtCore>
#include "UBExportAdaptor.h"

class UBDocumentProxy;

class UBExportPDF : public UBExportAdaptor
{
    Q_OBJECT;

    public:
        UBExportPDF(QObject *parent = 0);
        virtual ~UBExportPDF();

        virtual QString exportName();
        virtual QString exportExtention();
        virtual void persist(UBDocumentProxy* pDocument);
        virtual bool associatedActionactionAvailableFor(const QModelIndex &selectedIndex);

        virtual bool persistsDocument(UBDocumentProxy* pDocument, const QString& filename);
};

#endif /* UBEXPORTPDF_H_ */
