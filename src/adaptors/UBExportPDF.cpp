/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBExportPDF.h"

#include <QtCore>
#include <QtSvg>
#include <QPrinter>
#include <QPdfWriter>

#include "core/UBApplication.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBPersistenceManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPDFItem.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "pdf/GraphicsPDFItem.h"

#include "core/memcheck.h"

UBExportPDF::UBExportPDF(QObject *parent)
    : UBExportAdaptor(parent)
{
    // NOOP
}

UBExportPDF::~UBExportPDF()
{
    // NOOP
}

void UBExportPDF::persist(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    persistLocally(pDocumentProxy, tr("Export as PDF File"));
}

bool UBExportPDF::associatedActionactionAvailableFor(const QModelIndex &selectedIndex)
{
    const UBDocumentTreeModel *docModel = qobject_cast<const UBDocumentTreeModel*>(selectedIndex.model());
    if (!selectedIndex.isValid() || docModel->isCatalog(selectedIndex)) {
        return false;
    }

    return true;
}


bool UBExportPDF::persistsDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const QString& filename)
{
    QPdfWriter pdfWriter(filename);

    qDebug() << "exporting document to PDF" << filename;

    pdfWriter.setResolution(UBSettings::settings()->pdfResolution->get().toInt());
    pdfWriter.setPageMargins(QMarginsF());
    pdfWriter.setTitle(pDocumentProxy->name());
    pdfWriter.setCreator("OpenBoard PDF export");
    pdfWriter.setPdfVersion(QPagedPaintDevice::PdfVersion_1_4);

    // need to calculate screen resolution
    float dpiCommon = UBApplication::displayManager->logicalDpi(ScreenRole::Control);
    float scaleFactor = dpiCommon ? 72.0f / dpiCommon : 1.f;

    QPainter pdfPainter;
    bool painterNeedsBegin = true;

    int existingPageCount = pDocumentProxy->pageCount();

    for(int pageIndex = 0 ; pageIndex < existingPageCount; pageIndex++) {

        std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
        UBApplication::showMessage(tr("Exporting page %1 of %2").arg(pageIndex + 1).arg(existingPageCount));

        // set background to white, no crossing for PDF output
        bool isDark = scene->isDarkBackground();
        const auto pageBackground = scene->background();

        bool exportDark = isDark && UBSettings::settings()->exportBackgroundColor->get().toBool();

        if (UBSettings::settings()->exportBackgroundGrid->get().toBool())
        {
            scene->setSceneBackground(exportDark, pageBackground);
        }
        else
        {
            scene->setSceneBackground(exportDark, nullptr);
        }

        // pageSize is the output PDF page size; it is set to equal the scene's boundary size; if the contents
        // of the scene overflow from the boundaries, they will be scaled down.
        QSize pageSize = scene->sceneSize();

        // set high res rendering
        scene->setRenderingQuality(UBItem::RenderingQualityHigh, UBItem::CacheNotAllowed);
        scene->setRenderingContext(UBGraphicsScene::NonScreen);

        // Setting output page size
        QPageSize outputPageSize = QPageSize(QSizeF(pageSize.width()*scaleFactor, pageSize.height()*scaleFactor), QPageSize::Point);
        pdfWriter.setPageSize(outputPageSize);

        // Call begin only once
        if(painterNeedsBegin)
            painterNeedsBegin = !pdfPainter.begin(&pdfWriter);

        else if (pageIndex < existingPageCount)
            pdfWriter.newPage();

        // Render the scene
        scene->render(&pdfPainter, QRectF(), scene->normalizedSceneRect());

        // Restore screen rendering quality
        scene->setRenderingContext(UBGraphicsScene::Screen);
        scene->setRenderingQuality(UBItem::RenderingQualityNormal, UBItem::CacheAllowed);

        // Restore background state
        scene->setSceneBackground(isDark, pageBackground);
    }

    if(!painterNeedsBegin)
        pdfPainter.end();

    return true;
}

QString UBExportPDF::exportExtention()
{
    return QString(".pdf");
}

QString UBExportPDF::exportName()
{
    return tr("Export to PDF");
}
