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




#include "UBExportFullPDF.h"

#include <QtCore>
#include <QtSvg>
#include <QPrinter>

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

#include "UBExportPDF.h"

#include <Merger.h>
#include <Exception.h>
#include <Transformation.h>

#include "core/memcheck.h"


using namespace merge_lib;


UBExportFullPDF::UBExportFullPDF(QObject *parent)
    : UBExportAdaptor(parent)
{
    // need to calculate screen resolution
    float dpiCommon = UBApplication::displayManager->logicalDpi(ScreenRole::Control);
    mScaleFactor = 72.0f / dpiCommon; // 1pt = 1/72 inch

    mSimpleExporter = new UBExportPDF();
}


UBExportFullPDF::~UBExportFullPDF()
{
    // NOOP
}


void UBExportFullPDF::saveOverlayPdf(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const QString& filename)
{
    if (!pDocumentProxy || filename.length() == 0 || pDocumentProxy->pageCount() == 0)
        return;

    /*
        PDFMerger is supposed to be working only for PDFs using 1.0 to 1.4 standard, but I encountered no issue using 1.7 documents.
        Just tested with a few documents though so it could be a total mess to restore it without any possibility to disable it if an issue is encountered
        A new setting is introduced to handle that possibility.
        Also, calling simpleExporter here instead of the following code was done because I (wrongly) assumed that both codes was doing nearly the same thing, as I never
        tested to remove the part of pdf-merger throwing an exception if pdf version is > 1.4 and that calling the following lines without modifying the version check
        throws an exception and finally calls simpleExporter->persistsDocument
        calling simpleExporter directly also resulted in fixing two issues where pdf overlay could be badly scaled (not matching with scale of annotations)
    */
    bool usePDFMerger = UBSettings::settings()->pdfUsePDFMerger->get().toBool();
    if (usePDFMerger)
    {
        //PDF
        qDebug() << "exporting document to PDF Merger" << filename;
        QPrinter pdfPrinter;

        pdfPrinter.setOutputFormat(QPrinter::PdfFormat);
        pdfPrinter.setResolution(UBSettings::settings()->pdfResolution->get().toInt());
        pdfPrinter.setOutputFileName(filename);
        pdfPrinter.setFullPage(true);

        QPainter* pdfPainter = 0;

        for(int pageIndex = 0 ; pageIndex < pDocumentProxy->pageCount(); pageIndex++)
        {
            std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
            // set background according to PDF export settings
            bool isDark = scene->isDarkBackground();
            const auto sceneBackground = scene->background();

            bool exportDark = isDark && UBSettings::settings()->exportBackgroundColor->get().toBool();

            bool sceneHasPDFBackground = false;

            // set high res rendering
            scene->setRenderingQuality(UBItem::RenderingQualityHigh, UBItem::CacheNotAllowed);
            scene->setRenderingContext(UBGraphicsScene::PdfExport);

            // pageSize is the output PDF page size; it is set to equal the scene's boundary size; if the contents
            // of the scene overflow from the boundaries, they will be scaled down.
            QSizeF pageSize = scene->sceneSizeF() * mScaleFactor;   // points

            UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());

            if (pdfItem)
            {
                mHasPDFBackgrounds = true;
                sceneHasPDFBackground = true;
                pageSize = pdfItem->pageSize();     // original PDF document page size
            }
            else
            {
                sceneHasPDFBackground = false;
            }

            QPageSize size(pageSize, QPageSize::Point);
            pdfPrinter.setPageSize(size);

            if (!pdfPainter) pdfPainter = new QPainter(&pdfPrinter);

            if (pageIndex != 0) pdfPrinter.newPage();

            // do not draw background color and grid if scene has PDF background
            if (sceneHasPDFBackground)
            {
                scene->setDrawingMode(true);
                scene->setSceneBackground(false, nullptr);
            }
            else if (UBSettings::settings()->exportBackgroundGrid->get().toBool())
            {
                scene->setSceneBackground(exportDark, sceneBackground);
            }
            else
            {
                scene->setSceneBackground(exportDark, nullptr);
            }

            //render to PDF
            scene->render(pdfPainter, QRectF(), scene->normalizedSceneRect());

            //restore screen rendering quality
            scene->setRenderingContext(UBGraphicsScene::Screen);
            scene->setRenderingQuality(UBItem::RenderingQualityNormal, UBItem::CacheAllowed);

            //restore background state
            scene->setDrawingMode(false);
            scene->setSceneBackground(isDark, sceneBackground);
        }

        if (pdfPainter) delete pdfPainter;
    }
    else
    {
        mSimpleExporter->persistsDocument(pDocumentProxy, filename);
    }
}


void UBExportFullPDF::persist(std::shared_ptr<UBDocumentProxy> pDocumentProxy)
{
    persistLocally(pDocumentProxy, tr("Export as PDF File"));
}


bool UBExportFullPDF::persistsDocument(std::shared_ptr<UBDocumentProxy> pDocumentProxy, const QString& filename)
{
    QFile file(filename);
    if (file.exists()) file.remove();

    QString overlayName = filename;
    overlayName.replace(".pdf", "_overlay.pdf");

    QFile previousOverlay(overlayName);
    if (previousOverlay.exists())
        previousOverlay.remove();

    mHasPDFBackgrounds = false;

    saveOverlayPdf(pDocumentProxy, overlayName);

    if (!mHasPDFBackgrounds)
    {
        QFile f(overlayName);
        f.rename(filename);
    }
    else
    {
        Merger merger;
        try
        {
            merger.addOverlayDocument(QFile::encodeName(overlayName).constData());

            MergeDescription mergeInfo;

            // factor between scene coordinates and PDF coordinates
            double dpiScale = 72. / pDocumentProxy->pageDpi();

            int existingPageCount = pDocumentProxy->pageCount();

            for(int pageIndex = 0 ; pageIndex < existingPageCount; pageIndex++)
            {
                std::shared_ptr<UBGraphicsScene> scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
                UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());

                if (pdfItem)
                {
                    QString pdfName = UBPersistenceManager::objectDirectory + "/" + pdfItem->fileUuid().toString() + ".pdf";
                    QString backgroundPath = pDocumentProxy->persistencePath() + "/" + pdfName;

                    // Original data in scene coordinates, annotationsRect always contains pdfSceneRect
                    QRectF pdfSceneRect = pdfItem->sceneBoundingRect();
                    QRectF annotationsRect = scene->normalizedSceneRect();

                    double xAnnotation = annotationsRect.x();
                    double yAnnotation = annotationsRect.y();
                    double xPdf = pdfSceneRect.x();
                    double yPdf = pdfSceneRect.y();
                    double hPdf = pdfSceneRect.height();

                    // Exportation-transformed data, scaleFactor always <= 1
                    double hScaleFactor = pdfSceneRect.width() / annotationsRect.width();
                    double vScaleFactor = pdfSceneRect.height() / annotationsRect.height();
                    double scaleFactor = qMin(hScaleFactor, vScaleFactor);

                    double xAnnotationsOffset = 0;
                    double yAnnotationsOffset = 0;
                    double hPdfTransformed = hPdf * scaleFactor;

                    // Compute scaling of PDF on the scene
                    // If the PDF was scaled when added to the scene (e.g if it was loaded from a document with a different DPI
                    // than the current one), it should also be scaled here.
                    QSizeF pageSize = pdfItem->pageSize();
                    double pdfScale = pdfSceneRect.width() / pageSize.width() * dpiScale;

                    // Offsets are calculated in the PDF coordinate system.
                    // It has its origin at the lower left corner and is measured in points of 1/72 inch.
                    // Here, we force the PDF page to be on the topleft corner of the page
                    double xPdfOffset = 0;
                    double yPdfOffset = (hPdf - hPdfTransformed) * dpiScale / pdfScale;

                    // Now we align the items
                    xPdfOffset += (xPdf - xAnnotation) * scaleFactor * dpiScale / pdfScale;
                    yPdfOffset -= (yPdf - yAnnotation) * scaleFactor * dpiScale / pdfScale;

                    TransformationDescription pdfTransform(xPdfOffset, yPdfOffset, scaleFactor, 0);
                    TransformationDescription annotationTransform(xAnnotationsOffset, yAnnotationsOffset, 1, 0);

                    MergePageDescription pageDescription(pageSize.width(),
                                                         pageSize.height(),
                                                         pdfItem->pageNumber(),
                                                         QFile::encodeName(backgroundPath).constData(),
                                                         pdfTransform,
                                                         pageIndex + 1,
                                                         annotationTransform,
                                                         false, false);

                    mergeInfo.push_back(pageDescription);

                    merger.addBaseDocument(QFile::encodeName(backgroundPath).constData());
                }
                else
                {
                    QSizeF pageSize = scene->nominalSize() * mScaleFactor;

                    MergePageDescription pageDescription(pageSize.width(),
                             pageSize.height(),
                             0,
                             "",
                             TransformationDescription(),
                             pageIndex + 1,
                             TransformationDescription(),
                             false, true);

                    mergeInfo.push_back(pageDescription);
                }
            }

            merger.merge(QFile::encodeName(overlayName).constData(), mergeInfo);

            merger.saveMergedDocumentsAs(QFile::encodeName(filename).constData());

        }
        catch(const std::exception& e)
        {
            qWarning() << "An exception occured during PDF merging of document " << filename << " :" << e.what();

            QMessageBox errorBox;
            errorBox.setIcon(QMessageBox::Warning);
            errorBox.setText(tr("The original PDF imported in OpenBoard seems not valid and could not be merged with your annotations. Please repair it and then reimport it in OpenBoard. The current export will be done with detailed (heavy) images of the pages of the original PDF instead, to avoid complete export failure."));
            QApplication::restoreOverrideCursor();
            errorBox.exec();
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            // default to raster export

            mSimpleExporter->persistsDocument(pDocumentProxy, filename);
        }
        catch (...)
        {
            qWarning() << "An exception occured during PDF merging of document " << filename;

            QMessageBox errorBox;
            errorBox.setIcon(QMessageBox::Warning);
            errorBox.setText(tr("The original PDF imported in OpenBoard seems not valid and could not be merged with your annotations. Please repair it and then reimport it in OpenBoard. The current export will be done with detailed (heavy) images of the pages of the original PDF instead, to avoid complete export failure."));
            QApplication::restoreOverrideCursor();
            errorBox.exec();
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            // default to raster export
            mSimpleExporter->persistsDocument(pDocumentProxy, filename);
        }

        if (!UBApplication::app()->isVerbose())
        {
            QFile::remove(overlayName);
        }
    }

    return true;
}

bool UBExportFullPDF::associatedActionactionAvailableFor(const QModelIndex &selectedIndex)
{
    const UBDocumentTreeModel *docModel = qobject_cast<const UBDocumentTreeModel*>(selectedIndex.model());
    if (!selectedIndex.isValid() || docModel->isCatalog(selectedIndex)) {
        return false;
    }

    return true;
}


QString UBExportFullPDF::exportExtention()
{
    return QString(".pdf");
}

QString UBExportFullPDF::exportName()
{
    return tr("Export to PDF");
}
