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




#include "XPDFRenderer.h"

#include <QtGui>

#include <frameworks/UBPlatformUtils.h>
#ifndef USE_XPDF
    #include <poppler/cpp/poppler-version.h>
#endif

#include "core/memcheck.h"
#include "core/UBSettings.h"


QAtomicInt XPDFRenderer::sInstancesCount = 0;

XPDFRenderer::XPDFRenderer(const QString &filename, bool importingFile) :
    mpSplashBitmapHistorical(nullptr), mSplashHistorical(nullptr), mDocument(nullptr)
{
    switch (UBSettings::settings()->pdfZoomBehavior->get().toUInt()) {
        case 0: // Render each time (historical initial implementation).
        break;
        case 1: // Render a single image, degradated quality when zoomed big.
        default:
            m_pdfZoomCache.push_back(XPDFRendererZoomFactor::mode1_zoomFactor);
        break;
        case 2: // Render three images, optimal quality all the time.
            m_pdfZoomCache.push_back(XPDFRendererZoomFactor::mode2_zoomFactorStage1);
            m_pdfZoomCache.push_back(XPDFRendererZoomFactor::mode2_zoomFactorStage2);
            m_pdfZoomCache.push_back(XPDFRendererZoomFactor::mode2_zoomFactorStage3);
        break;
    }

    Q_UNUSED(importingFile);
    if (!globalParams)
    {
        // globalParams must be allocated once and never be deleted
        // note that this is *not* an instance variable of this XPDFRenderer class
#if POPPLER_VERSION_MAJOR > 0 || POPPLER_VERSION_MINOR >= 83
        globalParams = std::make_unique<GlobalParams>();
#else
        globalParams = new GlobalParams(0);
#endif
        globalParams->setupBaseFonts(QFile::encodeName(UBPlatformUtils::applicationResourcesDirectory() + "/" + "fonts").data());
    }
#ifdef USE_XPDF
    mDocument = new PDFDoc(new GString(filename.toLocal8Bit()), 0, 0, 0); // the filename GString is deleted on PDFDoc desctruction
#else
    mDocument = new PDFDoc(new GooString(filename.toLocal8Bit()), 0, 0, 0); // the filename GString is deleted on PDFDoc desctruction
#endif
    sInstancesCount.ref();
}

XPDFRenderer::~XPDFRenderer()
{
    for(int i = 0; i < m_pdfZoomCache.size(); i++)
    {
        PdfZoomCacheData &cacheData = m_pdfZoomCache[i];
        if(cacheData.splash != nullptr){
            cacheData.cachedImage = QImage(); // The 'cachedImage' uses a buffer from 'splash'.
            delete cacheData.splash;
            cacheData.splash = nullptr;
        }
    }

    if(mSplashHistorical)
        delete mSplashHistorical;

    if (mDocument)
    {
        delete mDocument;
        sInstancesCount.deref();
    }

    if (sInstancesCount.loadAcquire() == 0 && globalParams)
    {
#if POPPLER_VERSION_MAJOR > 0 || POPPLER_VERSION_MINOR >= 83
        globalParams.reset();
#else
        delete globalParams;
        globalParams = 0;
#endif
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
#if POPPLER_VERSION_MAJOR > 0 || POPPLER_VERSION_MINOR >= 55
        Object pdfInfo = mDocument->getDocInfo();
#else
        Object pdfInfo;
        mDocument->getDocInfo(&pdfInfo);
#endif
        if (pdfInfo.isDict())
        {
            Dict *infoDict = pdfInfo.getDict();
#if POPPLER_VERSION_MAJOR > 0 || POPPLER_VERSION_MINOR >= 55
            Object title = infoDict->lookup((char*)"Title");
#else
            Object title;
            infoDict->lookup((char*)"Title", &title);
#endif
            if (title.isString())
            {
#if POPPLER_VERSION_MAJOR > 0 || POPPLER_VERSION_MINOR >= 72
                return QString(title.getString()->c_str());
#else
                return QString(title.getString()->getCString());
#endif
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

        cropWidth = mDocument->getPageCropWidth(pageNumber) * this->dpiForRendering / 72.0;
        cropHeight = mDocument->getPageCropHeight(pageNumber) * this->dpiForRendering / 72.0;

        if (rotate == 90 || rotate == 270)
        {
            //switching width and height
            qreal tmpVar = cropWidth;
            cropWidth = cropHeight;
            cropHeight = tmpVar;
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


QImage* XPDFRenderer::createPDFImageHistorical(int pageNumber, qreal xscale, qreal yscale, const QRectF &bounds)
{
    if (isValid())
    {
        SplashColor paperColor = {0xFF, 0xFF, 0xFF}; // white
        if(mSplashHistorical)
            delete mSplashHistorical;
        mSplashHistorical = new SplashOutputDev(splashModeRGB8, 1, false, paperColor);
#ifdef USE_XPDF
        mSplashHistorical->startDoc(mDocument->getXRef());
#else
        mSplashHistorical->startDoc(mDocument);
#endif
        int rotation = 0; // in degrees (get it from the worldTransform if we want to support rotation)
        bool useMediaBox = false;
        bool crop = true;
        bool printing = false;
        mSliceX = 0.;
        mSliceY = 0.;

        if (bounds.isNull())
        {
            mDocument->displayPage(mSplashHistorical, pageNumber, this->dpiForRendering * xscale, this->dpiForRendering *yscale,
                                   rotation, useMediaBox, crop, printing);
        }
        else
        {
            mSliceX = bounds.x() * xscale;
            mSliceY = bounds.y() * yscale;
            qreal sliceW = bounds.width() * xscale;
            qreal sliceH = bounds.height() * yscale;

            mDocument->displayPageSlice(mSplashHistorical, pageNumber, this->dpiForRendering * xscale, this->dpiForRendering * yscale,
                rotation, useMediaBox, crop, printing, mSliceX, mSliceY, sliceW, sliceH);
        }

        mpSplashBitmapHistorical = mSplashHistorical->getBitmap();
    }
    return new QImage(mpSplashBitmapHistorical->getDataPtr(), mpSplashBitmapHistorical->getWidth(), mpSplashBitmapHistorical->getHeight(), mpSplashBitmapHistorical->getWidth() * 3, QImage::Format_RGB888);
}

void XPDFRenderer::render(QPainter *p, int pageNumber, const QRectF &bounds)
{
    Q_UNUSED(bounds);
    if (isValid())
    {
        if (m_pdfZoomCache.size() > 0)
        {
            qreal xscale = p->worldTransform().m11();
            qreal yscale = p->worldTransform().m22();
            Q_ASSERT(qFuzzyCompare(xscale, yscale)); // Zoom equal in all axes expected.
            Q_ASSERT(xscale > 0.0); // Potential Div0 later if this assert fail.

            int zoomIndex = 0;
            bool foundIndex = false;
            for (; zoomIndex < m_pdfZoomCache.size() && !foundIndex;)
            {
                if (xscale <= m_pdfZoomCache[zoomIndex].ratio) {
                    foundIndex = true;
                } else {
                    zoomIndex++;
                }
            }

            if (!foundIndex) // Use the previous one.
                zoomIndex--;


            QImage const &pdfImage = createPDFImageCached(pageNumber, m_pdfZoomCache[zoomIndex]);
            QTransform savedTransform = p->worldTransform();

            double const ratioDifferenceBetweenWorldAndImage = 1.0/m_pdfZoomCache[zoomIndex].ratio;
            // The 'pdfImage' is rendered with a quality equal or superior. We adjust the 'transform' to zoom it
            // out the required ratio.
            QTransform newTransform = savedTransform.scale(ratioDifferenceBetweenWorldAndImage, ratioDifferenceBetweenWorldAndImage);
            p->setWorldTransform(newTransform);
            p->drawImage(QPointF( mSliceX,  mSliceY), pdfImage);

            p->setWorldTransform(savedTransform);
        } else {
            qreal xscale = p->worldTransform().m11();
            qreal yscale = p->worldTransform().m22();

            QImage *pdfImage = createPDFImageHistorical(pageNumber, xscale, yscale, bounds);
            QTransform savedTransform = p->worldTransform();
            p->resetTransform();
            p->drawImage(QPointF(savedTransform.dx() + mSliceX, savedTransform.dy() + mSliceY), *pdfImage);
            p->setWorldTransform(savedTransform);
            delete pdfImage;
        }
    }
}

QImage& XPDFRenderer::createPDFImageCached(int pageNumber, PdfZoomCacheData &cacheData)
{
    if (isValid())
    {      
        SplashColor paperColor = {0xFF, 0xFF, 0xFF}; // white
        if (cacheData.requireUpdateImage(pageNumber))
        {
            cacheData.prepareNewSplash(pageNumber, paperColor);

#ifdef USE_XPDF
            cacheData.splash->startDoc(mDocument->getXRef());
#else
            cacheData.splash->startDoc(mDocument);
#endif
            int rotation = 0; // in degrees (get it from the worldTransform if we want to support rotation)
            bool useMediaBox = false;
            bool crop = true;
            bool printing = false;
            mSliceX = 0.;
            mSliceY = 0.;

            mDocument->displayPage(cacheData.splash, pageNumber, this->dpiForRendering * cacheData.ratio, this->dpiForRendering * cacheData.ratio,
                                   rotation, useMediaBox, crop, printing);
            cacheData.splashBitmap = cacheData.splash->getBitmap();
        }

        // Note this uses the 'cacheData.splash->getBitmap()->getDataPtr()' as data buffer.
        cacheData.cachedImage = QImage(cacheData.splashBitmap->getDataPtr(), cacheData.splashBitmap->getWidth(), cacheData.splashBitmap->getHeight(),
                               cacheData.splashBitmap->getWidth() * 3 /* bytesPerLine, 24 bits for RGB888, = 3 bytes */,
                               QImage::Format_RGB888);
    } else {
        cacheData.cachedImage = QImage();
    }

    return cacheData.cachedImage;
}
