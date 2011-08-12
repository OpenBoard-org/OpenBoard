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

#include "XPDFRenderer.h"

#include <QtGui>

#include <frameworks/UBPlatformUtils.h>

#include <splash/SplashBitmap.h>
#include <xpdf/Object.h>
#include <xpdf/GlobalParams.h>
#include <xpdf/SplashOutputDev.h>
#include <xpdf/PDFDoc.h>


#include "core/memcheck.h"

QAtomicInt XPDFRenderer::sInstancesCount = 0;

XPDFRenderer::XPDFRenderer(const QString &filename)
    : mDocument(0)
{
    if (!globalParams)
    {
        // globalParams must be allocated once and never be deleted
        // note that this is *not* an instance variable of this XPDFRenderer class
        globalParams = new GlobalParams(0);
        globalParams->setupBaseFonts(QFile::encodeName(UBPlatformUtils::applicationResourcesDirectory() + "/" + "fonts").data());
        //globalParams->setPrintCommands(gTrue);
    }

    mDocument = new PDFDoc(new GString(filename.toUtf8().data()), 0, 0, 0); // the filename GString is deleted on PDFDoc desctruction
    sInstancesCount.ref();

}

XPDFRenderer::~XPDFRenderer()
{
    if (mDocument)
    {
        delete mDocument;
        sInstancesCount.deref();
    }

    if (sInstancesCount == 0 && globalParams)
    {
        delete globalParams;
        globalParams = 0;
    }
}

bool XPDFRenderer::isValid() const
{
    if (mDocument)
    {
        return mDocument->isOk();
    }
    else
    {
        return false;
    }
}

int XPDFRenderer::pageCount() const
{
    if (isValid())
        return mDocument->getNumPages();
    else
        return 0;
}

QString XPDFRenderer::title() const
{
    if (isValid())
    {
        Object pdfInfo;
        mDocument->getDocInfo(&pdfInfo);
        if (pdfInfo.isDict())
        {
            Object title;
            Dict *infoDict = pdfInfo.getDict();
            if (infoDict->lookup((char*)"Title", &title)->isString())
            {
                GString *gstring = title.getString();
                return QString(gstring->getCString());
            }
        }
    }

    return QString();
}


QSizeF XPDFRenderer::pageSizeF(int pageNumber) const
{
    qreal cropWidth = 0;
    qreal cropHeight = 0;

    if (isValid())
    {
        int rotate = mDocument->getPageRotate(pageNumber);

        cropWidth = mDocument->getPageCropWidth(pageNumber);
        cropHeight = mDocument->getPageCropHeight(pageNumber);

        if ((rotate == 90) || (rotate == 270))
        {
            cropWidth = mDocument->getPageCropHeight(pageNumber);
            cropHeight = mDocument->getPageCropWidth(pageNumber);
        }
    }

    return QSizeF(cropWidth, cropHeight);
}


int XPDFRenderer::pageRotation(int pageNumber) const
{
    if (mDocument)
        return  mDocument->getPageRotate(pageNumber);
    else
        return 0;
}


void XPDFRenderer::render(QPainter *p, int pageNumber, const QRectF &bounds)
{
    if (isValid())
    {
        SplashColor paperColor = {0xFF, 0xFF, 0xFF}; // white
        SplashOutputDev splash(splashModeRGB8, 1, gFalse, paperColor);
        splash.startDoc(mDocument->getXRef());
        int hResolution = 72;
        int vResolution = 72;
        int rotation = 0; // in degrees (get it from the worldTransform if we want to support rotation)
        GBool useMediaBox = gFalse;
        GBool crop = gTrue;
        GBool printing = gFalse;
        const qreal xScale = p->worldTransform().m11();
        const qreal yScale = p->worldTransform().m22();
        qreal sliceX = 0.;
        qreal sliceY = 0.;

        if (bounds.isNull())
        {
            mDocument->displayPage(&splash, pageNumber, hResolution * xScale, vResolution * yScale,
                                   rotation, useMediaBox, crop, printing);
        }
        else
        {
            sliceX = bounds.x() * xScale;
            sliceY = bounds.y() * yScale;
            qreal sliceW = bounds.width() * xScale;
            qreal sliceH = bounds.height() * yScale;

            mDocument->displayPageSlice(&splash, pageNumber, hResolution * xScale, vResolution * yScale,
                                        rotation, useMediaBox, crop, printing, sliceX, sliceY, sliceW, sliceH);
        }

        SplashBitmap *bitmap = splash.getBitmap();
        QImage pdfImage(bitmap->getDataPtr(), bitmap->getWidth(), bitmap->getHeight(), bitmap->getWidth() * 3, QImage::Format_RGB888);

        QTransform savedTransform = p->worldTransform();
        p->resetTransform();
        p->drawImage(QPointF(savedTransform.dx() + sliceX, savedTransform.dy() + sliceY), pdfImage);
        p->setWorldTransform(savedTransform);
    }
}