/*
 * UBExportFullPDF.cpp
 *
 *  Created on: Aug 21, 2009
 *      Author: Patrick
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

using namespace merge_lib;


UBExportFullPDF::UBExportFullPDF(QObject *parent)
    : UBExportAdaptor(parent)
{
    // NOOP
}


UBExportFullPDF::~UBExportFullPDF()
{
    // NOOP
}


void UBExportFullPDF::saveOverlayPdf(UBDocumentProxy* pDocumentProxy, QString filename)
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

    const qreal margin = UBSettings::settings()->pdfMargin->get().toDouble() * pdfPrinter.resolution() / 25.4;
    mMargin = margin;

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

        UBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<UBGraphicsPDFItem*>(scene->backgroundObject());

        if (pdfItem)
        {
            QSizeF sceneItemsBound = scene->itemsBoundingRect().size();
            qreal ratio = (qreal)pdfPrinter.resolution() / 72.0;
            QSizeF scaled = sceneItemsBound * ratio;

            pdfPrinter.setPaperSize(scaled, QPrinter::DevicePixel);

            if (pageIndex != 0)
                 pdfPrinter.newPage();

            if (!pdfPainter)
                pdfPainter = new QPainter(&pdfPrinter);

            //render to PDF
            scene->render(pdfPainter, QRectF(0, 0, sceneItemsBound.width() * ratio
                    , sceneItemsBound.height() * ratio), scene->itemsBoundingRect());

            mHasPDFBackgrounds = true;
        }
        else
        {
            if (UBSettings::settings()->pdfPageFormat->get().toString() == "Letter")
                pdfPrinter.setPageSize(QPrinter::Letter);
            else
                pdfPrinter.setPageSize(QPrinter::A4);

            QSize docSize = pDocumentProxy->defaultDocumentSize();
            if(docSize.width() > docSize.height())
            {
                pdfPrinter.setOrientation(QPrinter::Landscape);
            }

            if (pageIndex != 0)
                 pdfPrinter.newPage();

            mDefaultPageRect = pdfPrinter.paperRect();
            QRectF paperRect = mDefaultPageRect.adjusted(margin, margin, -margin, -margin);
            QRectF normalized = scene->normalizedSceneRect(paperRect.width() / paperRect.height());

            if (!pdfPainter)
                pdfPainter = new QPainter(&pdfPrinter);

            //render to PDF
            scene->render(pdfPainter, paperRect, normalized);
        }

        //restore screen rendering quality
        scene->setRenderingContext(UBGraphicsScene::Screen);
        scene->setRenderingQuality(UBItem::RenderingQualityNormal);

        //restore background state
        scene->setBackground(isDark, isCrossed);
    }

    if (pdfPainter)
        delete pdfPainter;
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


void UBExportFullPDF::persistsDocument(UBDocumentProxy* pDocumentProxy, QString filename)
{

    QFile file(filename);
    if (file.exists())
        file.remove();

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

                if (pdfItem)
                {
                    QString pdfName = UBPersistenceManager::objectDirectory + "/" + pdfItem->fileUuid().toString() + ".pdf";
                    QString backgroundPath = pDocumentProxy->persistencePath() + "/" + pdfName;

                    QPointF boudingRectBottomLeft = scene->itemsBoundingRect().bottomLeft();
                    QPointF pdfItemBottomLeft = pdfItem->sceneBoundingRect().bottomLeft();
                    QPointF offset = pdfItemBottomLeft - boudingRectBottomLeft;

                    qDebug() << "scene->itemsBoundingRect()" << scene->itemsBoundingRect();
                    qDebug() << "pdfItem->boundingRect()" << pdfItem->boundingRect();
                    qDebug() << "pdfItem->sceneBoundingRect()" << pdfItem->sceneBoundingRect();
                    qDebug() << offset;

                    TransformationDescription baseTrans(offset.x(), offset.y() * -1, 1, 0);
                    //TransformationDescription baseTrans(0, 0, 1, 0);
                    TransformationDescription overlayTrans(0, 0, 1, 0);

                    MergePageDescription pageDescription(scene->itemsBoundingRect().width(),
                                                         scene->itemsBoundingRect().height(),
                                                         pdfItem->pageNumber(),
                                                         QFile::encodeName(backgroundPath).constData(),
                                                         baseTrans,
                                                         pageIndex + 1,
                                                         overlayTrans,
                                                         false, false);

                    mergeInfo.push_back(pageDescription);

                    merger.addBaseDocument(QFile::encodeName(backgroundPath).constData());
                }
                else
                {
                    QRectF paperRect = mDefaultPageRect.adjusted(mMargin, mMargin, -mMargin, -mMargin);
                    QRectF normalized = scene->normalizedSceneRect(paperRect.width() / paperRect.height());


                    MergePageDescription pageDescription(normalized.width(),
                             normalized.height(),
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
