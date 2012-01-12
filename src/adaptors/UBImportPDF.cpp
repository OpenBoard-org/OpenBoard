/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBImportPDF.h"
#include "UBMetadataDcSubsetAdaptor.h"
#include "UBSvgSubsetAdaptor.h"

#include "document/UBDocumentProxy.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "domain/UBGraphicsPDFItem.h"

#include "pdf/PDFRenderer.h"

#include "core/memcheck.h"

UBImportPDF::UBImportPDF(QObject *parent)
    : UBImportAdaptor(parent)
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


bool UBImportPDF::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
    QString documentName = QFileInfo(pFile.fileName()).completeBaseName();

    QUuid uuid = QUuid::createUuid();

    QString filepath = UBPersistenceManager::persistenceManager()->addPdfFileToDocument(pDocument, pFile.fileName(), uuid);

    PDFRenderer *pdfRenderer = PDFRenderer::rendererForUuid(uuid, pDocument->persistencePath() + "/" + filepath, true); // renderer is automatically deleted when not used anymore

    if (!pdfRenderer->isValid())
    {
        UBApplication::showMessage(tr("PDF import failed."));
        return false;
    }
	pdfRenderer->setDPI(this->dpi);

    int documentPageCount = pDocument->pageCount();

    if (documentPageCount == 1 && UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocument, 0)->isEmpty())
    {
        documentPageCount = 0;
    }

    int pdfPageCount = pdfRenderer->pageCount();

    for(int pdfPageNumber = 1; pdfPageNumber <= pdfPageCount; pdfPageNumber++)
    {
        int pageIndex = documentPageCount + (pdfPageNumber - 1);
        UBApplication::showMessage(tr("Importing page %1 of %2").arg(pdfPageNumber).arg(pdfPageCount), true);

        UBGraphicsScene* scene = 0;

        if (pageIndex == 0)
        {
            scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(pDocument, pageIndex);
        }
        else
        {
            scene = UBPersistenceManager::persistenceManager()->createDocumentSceneAt(pDocument, pageIndex);
        }

        scene->setBackground(false, false);
        UBGraphicsPDFItem *pdfItem = new UBGraphicsPDFItem(pdfRenderer, pdfPageNumber); // deleted by the scene
        scene->addItem(pdfItem);

        pdfItem->setPos(-pdfItem->boundingRect().width() / 2, -pdfItem->boundingRect().height() / 2);

        scene->setAsBackgroundObject(pdfItem, false, false);

        scene->setNominalSize(pdfItem->boundingRect().width(), pdfItem->boundingRect().height());


        UBPersistenceManager::persistenceManager()->persistDocumentScene(pDocument, scene, pageIndex);
    }

    UBApplication::showMessage(tr("PDF import successful."));

    return true;
}
