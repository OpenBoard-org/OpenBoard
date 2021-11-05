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

namespace constants{
    SplashColor paperColor = {0xFF, 0xFF, 0xFF}; // white
}

XPDFRenderer::XPDFRenderer(const QString &filename, bool importingFile)
    : m_pdfZoomMode(UBSettings::settings()->pdfZoomBehavior->get().toUInt())
    , mpSplashBitmapHistorical(nullptr)
    , mSplashHistorical(nullptr)
    , mDocument(nullptr)
{
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

    if (mDocument->isOk())
    {
        initPDFZoomData();

        sInstancesCount.ref();
        connect(&m_cacheThread, SIGNAL(finished()), this, SLOT(OnThreadFinished()));
    }
    else
    {
        qWarning() << tr("an error occured while trying to open the PDF file");
    }
}

XPDFRenderer::~XPDFRenderer()
{
    disconnect(&m_cacheThread, SIGNAL(finished()), this, SLOT(OnThreadFinished()));
    m_cacheThread.cancelPending();
    m_cacheThread.wait(XPDFThreadMaxTimeoutOnExit::timeout_ms);
    if (m_cacheThread.isRunning())
    {
        // Kill the thread, which might still run for minutes if the user choose a heavy pdf highly zoomed.
        // Since there is no data written, but only processing, this is safe on a modern OS.
        m_cacheThread.terminate();
    }

    foreach (QVector<PdfZoomCacheData> v, m_perPagepdfZoomCache)
    {
        for(int i = 0; i < v.size(); i++)
        {
            PdfZoomCacheData &cacheData = v[i];
            cacheData.cleanup();
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

void XPDFRenderer::initPDFZoomData()
{
    qDebug() << "initPDFZoomData";
    if (mDocument)
    {
        for (int i=1; i <= mDocument->getNumPages(); i++)
        {
            m_perPagepdfZoomCache.insert(i, QVector<PdfZoomCacheData>());

            for (int j = 0; j < XPDFRendererZoomFactor::mode4_zoomFactorIterations; j++ )
            {
                double const zoomValue = XPDFRendererZoomFactor::mode4_zoomFactorStart+XPDFRendererZoomFactor::mode4_zoomFactorStepSquare*static_cast<double>(j*j);
                m_perPagepdfZoomCache[i].push_back(zoomValue);
            }
        }
    }
    else
    {
        qDebug() << "an error occured. mDocument failed to open requested file";
    }
    qDebug() << "end initPDFZoomData";
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
        if(mSplashHistorical)
            delete mSplashHistorical;

        mSplashHistorical = new SplashOutputDev(splashModeRGB8, 1, false, constants::paperColor);
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

void XPDFRenderer::OnThreadFinished()
{
    emit signalUpdateParent();
    if (m_cacheThread.isJobPending())
        m_cacheThread.start();
}

void XPDFRenderer::render(QPainter *p, int pageNumber, bool const cacheAllowed, const QRectF &bounds)
{
    //qDebug() << "render enter";
    Q_UNUSED(bounds);
    if (isValid())
    {
        if (m_perPagepdfZoomCache.contains(pageNumber) && m_perPagepdfZoomCache[pageNumber].size() > 0 && cacheAllowed)
        {
            qreal xscale = p->worldTransform().m11();
            qreal yscale = p->worldTransform().m22();
            Q_ASSERT(qFuzzyCompare(xscale, yscale)); // Zoom equal in all axes expected.
            Q_ASSERT(xscale > 0.0); // Potential Div0 later if this assert fail.

            qreal zoomRequested = xscale;
            int zoomIndex = 0;
            if (m_pdfZoomMode == 3)
            {
                // Choose a zoom which is inferior or equivalent than the user choice (= minor loss, downscaling).
                bool foundIndex = false;
                for (zoomIndex = m_perPagepdfZoomCache[pageNumber].size()-1; zoomIndex >= 0 && !foundIndex;)
                {
                    if (zoomRequested >= m_perPagepdfZoomCache[pageNumber][zoomIndex].ratio) {
                        foundIndex = true;
                    } else {
                        zoomIndex--;
                    }
                }

                if (!foundIndex) // Use the smallest one.
                    zoomIndex = 0;

                if (zoomIndex == 0 && m_perPagepdfZoomCache[pageNumber][zoomIndex].ratio != zoomRequested)
                {
                    m_perPagepdfZoomCache[pageNumber][zoomIndex].cleanup();
                    m_perPagepdfZoomCache[pageNumber][zoomIndex] = PdfZoomCacheData(zoomRequested);
                }
            } else {
                // Choose a zoom which is superior or equivalent than the user choice (= no loss, upscaling).
                bool foundIndex = false;
                for (; zoomIndex < m_perPagepdfZoomCache[pageNumber].size() && !foundIndex;)
                {
                    if (zoomRequested <= (m_perPagepdfZoomCache[pageNumber][zoomIndex].ratio+0.1)) {
                        foundIndex = true;
                    } else {
                        zoomIndex++;
                    }
                }

                if (!foundIndex) // Use the previous one.
                    zoomIndex--;
            }

            QImage pdfImage = createPDFImageCached(pageNumber, m_perPagepdfZoomCache[pageNumber][zoomIndex]);
            qreal ratioExpected = m_perPagepdfZoomCache[pageNumber][zoomIndex].ratio;
            qreal ratioObtained = ratioExpected;
            int const initialZoomIndex = zoomIndex;

            if (pdfImage == QImage() && m_perPagepdfZoomCache[pageNumber][zoomIndex].hasToBeProcessed)
            {
                // Try to temporarily fallback on a valid image, for a fuzzy or downsampled preview.
                // The actual result will be updated after the processing.
                bool isCurrent = true;
                while (zoomIndex < m_perPagepdfZoomCache[pageNumber].size()-1 && (m_perPagepdfZoomCache[pageNumber][zoomIndex].cachedImage == QImage() || (m_perPagepdfZoomCache[pageNumber][zoomIndex].cachedPageNumber != pageNumber && !isCurrent)))
                {
                    zoomIndex = zoomIndex+1;
                    isCurrent = false;
                }
                while (zoomIndex > 0 && (m_perPagepdfZoomCache[pageNumber][zoomIndex].cachedImage == QImage() || m_perPagepdfZoomCache[pageNumber][zoomIndex].cachedPageNumber != pageNumber))
                    zoomIndex = zoomIndex-1;
                ratioObtained = m_perPagepdfZoomCache[pageNumber][zoomIndex].ratio;
            }

            if (m_perPagepdfZoomCache[pageNumber][zoomIndex].cachedImage == QImage() || m_perPagepdfZoomCache[pageNumber][zoomIndex].cachedPageNumber != pageNumber)
            {
                qDebug() << "no alternate image found for page number " << pageNumber;
                if (pageNumber == 1)
                {
                    qDebug() << "why are you different..";
                }
                // No alternate image found. Build an alternate image in order to display some progress.
                // Also make sure we fallback to the initial ratio request.
                zoomIndex = initialZoomIndex;
                qreal ratioDiff = m_perPagepdfZoomCache[pageNumber][zoomIndex].ratio;
                pdfImage = QImage(bounds.width()*ratioDiff, bounds.height()*ratioDiff, QImage::Format_RGB888);
                pdfImage.fill("white");

                QPainter painter(&pdfImage);
                QString const text = tr("Processing...");
                QFont font = painter.font();
                if (font.pixelSize() != -1)
                    font.setPixelSize(ratioDiff*font.pixelSize());
                else
                    font.setPointSizeF(ratioDiff*font.pointSizeF());
                painter.setFont(font);
                QFontMetrics textMetric(font, &pdfImage);
                QSize textSize = textMetric.size(0, text);
                painter.drawText((bounds.width()*ratioDiff-textSize.width())/2, (bounds.height()*ratioDiff-textSize.height())/2, text);
            } else {
                pdfImage = m_perPagepdfZoomCache[pageNumber][zoomIndex].cachedImage;
            }

            QTransform savedTransform = p->worldTransform();

            double const ratioDifferenceBetweenWorldAndImage = 1.0/m_perPagepdfZoomCache[pageNumber][zoomIndex].ratio;
            // The 'pdfImage' is maybe rendered with a different quality than requested. We adjust the 'transform' to zoom it
            // in or out of the required ratio.
            QTransform newTransform = savedTransform.scale(ratioDifferenceBetweenWorldAndImage, ratioDifferenceBetweenWorldAndImage);
            p->setWorldTransform(newTransform);
            /* qDebug() << "drawImage size=" << p->viewport() << "bounds" << bounds <<
                        "pdfImage" << pdfImage.size() << "savedTransform" << savedTransform.m11() <<
                        "ratioDiff" << ratioDifferenceBetweenWorldAndImage << "zoomRequested" << zoomRequested <<
                        "zoomIndex" << zoomIndex; */
            p->drawImage(QPointF( mSliceX,  mSliceY), pdfImage);

            p->setWorldTransform(savedTransform);
        } else {
            qreal xscale = p->worldTransform().m11();
            qreal yscale = p->worldTransform().m22();

            QImage *pdfImage = createPDFImageHistorical(pageNumber, xscale, yscale, bounds);
            QTransform savedTransform = p->worldTransform();
            p->resetTransform();
            //qDebug() << "drawImage size=" << p->viewport() << "bounds" << bounds << "pdfImage" << pdfImage->size() << "savedTransform" << savedTransform.m11();
            p->drawImage(QPointF(savedTransform.dx() + mSliceX, savedTransform.dy() + mSliceY), *pdfImage);
            p->setWorldTransform(savedTransform);
            delete pdfImage;
        }
    }
    //qDebug() << "render leave";
}

QImage& XPDFRenderer::createPDFImageCached(int pageNumber, PdfZoomCacheData &cacheData)
{
    if (isValid())
    {      
        if (cacheData.requireUpdateImage(pageNumber) && !cacheData.hasToBeProcessed)
        {           
            mSliceX = 0.;
            mSliceY = 0.;

            CacheThread::JobData jobData;
            jobData.cacheData = &cacheData;
            jobData.document = mDocument;
            jobData.dpiForRendering = this->dpiForRendering;
            jobData.pageNumber = pageNumber;
            jobData.cacheData->hasToBeProcessed = true;
            // Make sure we reset that image, because the data uses 'splash' buffer, which will be deallocated and
            // reallocated when the job is started.
            jobData.cacheData->cachedImage = QImage();
            m_cacheThread.pushJob(jobData);

            if (m_pdfZoomMode == 4)
            {
                // Start the job multithreaded. The item will be refreshed when the signal 'finished' is emitted.
                m_cacheThread.start();
            } else {
                // Perform the job now. Note this will lock the GUI until the job is done.
                m_cacheThread.run();
            }
        }
    } else {
        cacheData.cachedImage = QImage();
    }

    return cacheData.cachedImage;
}

void XPDFRenderer::CacheThread::run()
{
    m_jobMutex.lock();

    CacheThread::JobData jobData = m_nextJob.first();
    m_nextJob.pop_front();
    /* qDebug() << "XPDFRenderer::CacheThread starting page" << jobData.pageNumber
             << "ratio" << jobData.cacheData->ratio; */

    jobData.cacheData->prepareNewSplash(jobData.pageNumber, constants::paperColor);

#ifdef USE_XPDF
    jobData.cacheData->splash->startDoc(jobData.document->getXRef());
#else
    jobData.cacheData->splash->startDoc(jobData.document);
#endif

    m_jobMutex.unlock();

    int rotation = 0; // in degrees (get it from the worldTransform if we want to support rotation)
    bool useMediaBox = false;
    bool crop = true;
    bool printing = false;

    jobData.document->displayPage(jobData.cacheData->splash, jobData.pageNumber, jobData.dpiForRendering * jobData.cacheData->ratio,
                                  jobData.dpiForRendering * jobData.cacheData->ratio,
                           rotation, useMediaBox, crop, printing);

    m_jobMutex.lock();
    jobData.cacheData->splashBitmap = jobData.cacheData->splash->getBitmap();
    // Note this uses the 'cacheData.splash->getBitmap()->getDataPtr()' as data buffer.
    jobData.cacheData->cachedImage = QImage(jobData.cacheData->splashBitmap->getDataPtr(), jobData.cacheData->splashBitmap->getWidth(), jobData.cacheData->splashBitmap->getHeight(),
                           jobData.cacheData->splashBitmap->getWidth() * 3 /* bytesPerLine, 24 bits for RGB888, = 3 bytes */,
                           QImage::Format_RGB888);

    /* qDebug() << "XPDFRenderer::CacheThread completed page" << jobData.pageNumber
             << "ratio" << jobData.cacheData->ratio << "final size is" << jobData.cacheData->cachedImage.size(); */

    jobData.cacheData->hasToBeProcessed = false;
    m_jobMutex.unlock();
}
