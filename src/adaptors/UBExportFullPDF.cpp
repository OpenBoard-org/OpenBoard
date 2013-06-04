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



#include "UBExportFullPDF.h"

#include <QtCore>
#include <QtSvg>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBPersistenceManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPDFItem.h"

#include "document/UBDocumentProxy.h"

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
    //need to calculate screen resolution
    QDesktopWidget* desktop = UBApplication::desktop();
    int dpiCommon = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
    mScaleFactor = 72.0f / dpiCommon;
}


UBExportFullPDF::~UBExportFullPDF()
{
    // NOOP
}


void UBExportFullPDF::saveOverlayPdf(UBDocumentProxy* pDocumentProxy, const QString& filename)
{
    if (!pDocumentProxy || filename.length() == 0 || pDocumentProxy->pageCount() == 0)
        return;

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
        UBGraphicsScene* scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
        // set background to white, no grid for PDF output
        bool isDark = scene->isDarkBackground();
        bool isCrossed = scene->isCrossedBackground();
        scene->setBackground(false, false);

        // set high res rendering
        scene->setRenderingQuality(UBItem::RenderingQualityHigh);
        scene->setRenderingContext(UBGraphicsScene::PdfExport);

        QSize pageSize = scene->nominalSize();

        UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());

        if (pdfItem) mHasPDFBackgrounds = true;
        
        pdfPrinter.setPaperSize(QSizeF(pageSize.width()*mScaleFactor, pageSize.height()*mScaleFactor), QPrinter::Point);

        if (!pdfPainter) pdfPainter = new QPainter(&pdfPrinter);

        if (pageIndex != 0) pdfPrinter.newPage();

        //render to PDF
        scene->setDrawingMode(true);
        scene->render(pdfPainter, QRectF(), scene->normalizedSceneRect());

        //restore screen rendering quality
        scene->setRenderingContext(UBGraphicsScene::Screen);
        scene->setRenderingQuality(UBItem::RenderingQualityNormal);

        //restore background state
        scene->setDrawingMode(false);
        scene->setBackground(isDark, isCrossed);
    }

    if (pdfPainter) delete pdfPainter;
}


void UBExportFullPDF::persist(UBDocumentProxy* pDocumentProxy)
{
    if (!pDocumentProxy)
        return;

    QString filename = askForFileName(pDocumentProxy, tr("Export as PDF File"));

    if (filename.length() > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        if (mIsVerbose)
            UBApplication::showMessage(tr("Exporting document..."));

        persistsDocument(pDocumentProxy, filename);
        if (mIsVerbose)
            UBApplication::showMessage(tr("Export successful."));

        QApplication::restoreOverrideCursor();
    }
}


void UBExportFullPDF::persistsDocument(UBDocumentProxy* pDocumentProxy, const QString& filename)
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

            int existingPageCount = pDocumentProxy->pageCount();

            for(int pageIndex = 0 ; pageIndex < existingPageCount; pageIndex++)
            {
                UBGraphicsScene* scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
                UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());

                QSize pageSize = scene->nominalSize();
                
                if (pdfItem)
                {
                    QString pdfName = UBPersistenceManager::objectDirectory + "/" + pdfItem->fileUuid().toString() + ".pdf";
                    QString backgroundPath = pDocumentProxy->persistencePath() + "/" + pdfName;
                    QRectF annotationsRect = scene->itemsBoundingRect();

                    // Original datas
                    double xAnnotation = qRound(annotationsRect.x());
                    double yAnnotation = qRound(annotationsRect.y());
                    double xPdf = qRound(pdfItem->sceneBoundingRect().x());
                    double yPdf = qRound(pdfItem->sceneBoundingRect().y());
                    double hPdf = qRound(pdfItem->sceneBoundingRect().height());

                    // Exportation-transformed datas
                    double hScaleFactor = pageSize.width()/scene->itemsBoundingRect().width();
                    double vScaleFactor = pageSize.height()/scene->itemsBoundingRect().height();
                    double scaleFactor = qMin(hScaleFactor, vScaleFactor);

                    double xAnnotationsOffset = 0;
                    double yAnnotationsOffset = 0;
                    double hPdfTransformed = qRound(hPdf * scaleFactor);

                    // Here, we force the PDF page to be on the topleft corner of the page
                    double xPdfOffset = 0;
                    double yPdfOffset = (hPdf - hPdfTransformed) * mScaleFactor;

                    // Now we align the items
                    xPdfOffset += (xPdf - xAnnotation) * scaleFactor * mScaleFactor;
                    yPdfOffset -= (yPdf - yAnnotation) * scaleFactor * mScaleFactor;

                    TransformationDescription pdfTransform(xPdfOffset, yPdfOffset, scaleFactor, 0);
                    TransformationDescription annotationTransform(xAnnotationsOffset, yAnnotationsOffset, 1, 0);

                    MergePageDescription pageDescription(pageSize.width() * mScaleFactor,
                                                         pageSize.height() * mScaleFactor,
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
                    MergePageDescription pageDescription(pageSize.width() * mScaleFactor,
                             pageSize.height() * mScaleFactor,
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
        catch(Exception e)
        {
            qDebug() << "PdfMerger failed to merge documents to " << filename << " - Exception : " << e.what();

            // default to raster export
            UBExportPDF::persistsDocument(pDocumentProxy, filename);
        }

        if (!UBApplication::app()->isVerbose())
        {
            QFile::remove(overlayName);
        }
    }
}


QString UBExportFullPDF::exportExtention()
{
    return QString(".pdf");
}

QString UBExportFullPDF::exportName()
{
    return tr("Export to PDF");
}
