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




#include "UBImportPDF.h"

#include "document/UBDocumentProxy.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "domain/UBGraphicsPDFItem.h"

#include "pdf/PDFRenderer.h"

#include "core/memcheck.h"

UBImportPDF::UBImportPDF(QObject *parent)
    : UBPageBasedImportAdaptor(parent)
{
    QDesktopWidget* desktop = UBApplication::desktop();
    this->dpi = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
}


UBImportPDF::~UBImportPDF()
{
    // NOOP
}


QStringList UBImportPDF::supportedExtentions()
{
    return QStringList("pdf");
}


QString UBImportPDF::importFileFilter()
{
    return tr("Portable Document Format (*.pdf)");
}


QList<UBGraphicsItem*> UBImportPDF::import(const QUuid& uuid, const QString& filePath)
{
    QList<UBGraphicsItem*> result;

    PDFRenderer *pdfRenderer = PDFRenderer::rendererForUuid(uuid, filePath, true); // renderer is automatically deleted when not used anymore

    if (!pdfRenderer->isValid())
    {
        UBApplication::showMessage(tr("PDF import failed."));
        return result;
    }
    pdfRenderer->setDPI(this->dpi);

    int pdfPageCount = pdfRenderer->pageCount();

    for(int pdfPageNumber = 1; pdfPageNumber <= pdfPageCount; pdfPageNumber++)
    {
        UBApplication::showMessage(tr("Importing page %1 of %2").arg(pdfPageNumber).arg(pdfPageCount), true);
        result << new UBGraphicsPDFItem(pdfRenderer, pdfPageNumber); // deleted by the scene
    }
    return result;
}

void UBImportPDF::placeImportedItemToScene(UBGraphicsScene* scene, UBGraphicsItem* item)
{
    UBGraphicsPDFItem *pdfItem = (UBGraphicsPDFItem*)item;

    pdfItem->setPos(-pdfItem->boundingRect().width() / 2, -pdfItem->boundingRect().height() / 2);

    scene->setAsBackgroundObject(pdfItem, false, false);

    scene->setNominalSize(pdfItem->boundingRect().width(), pdfItem->boundingRect().height());
}

const QString& UBImportPDF::folderToCopy()
{
    return UBPersistenceManager::objectDirectory;
}
