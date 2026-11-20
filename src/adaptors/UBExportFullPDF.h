/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors
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



#pragma once

#include <QtCore>
#include "UBExportAdaptor.h"
#include "UBExportPDF.h"

class UBDocumentProxy;

enum class ScalingMode
{
    ScaleContent, ClipContent, ExtendPage
};

class UBExportFullPDF : public UBExportAdaptor
{
    Q_OBJECT

public:
    UBExportFullPDF(ScalingMode scalingMode, QObject* parent = nullptr);
    virtual ~UBExportFullPDF();

    virtual QString exportName();
    virtual QString exportExtention();
    virtual void persist(std::shared_ptr<UBDocumentProxy> pDocument);
    virtual bool associatedActionactionAvailableFor(const QModelIndex &selectedIndex);

    virtual bool persistsDocument(std::shared_ptr<UBDocumentProxy> pDocument, const QString& filename);

protected:
    void saveOverlayPdf(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const QString& filename);

private:
    bool mHasPDFBackgrounds{false};
    ScalingMode mScalingMode{ScalingMode::ScaleContent};

    UBExportPDF* mSimpleExporter{nullptr};
};
