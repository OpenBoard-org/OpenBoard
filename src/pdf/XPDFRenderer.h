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




#ifndef XPDFRENDERER_H
#define XPDFRENDERER_H
#include <QImage>
#include <QThread>
#include <QMutexLocker>
#include "PDFRenderer.h"
#include <splash/SplashBitmap.h>

#include "globals/UBGlobals.h"

#ifdef USE_XPDF
    THIRD_PARTY_WARNINGS_DISABLE
    #include <xpdf/Object.h>
    #include <xpdf/GlobalParams.h>
    #include <xpdf/SplashOutputDev.h>
    #include <xpdf/PDFDoc.h>
    THIRD_PARTY_WARNINGS_ENABLE
#else
    #include <poppler/Object.h>
    #include <poppler/GlobalParams.h>
    #include <poppler/SplashOutputDev.h>
    #include <poppler/PDFDoc.h>
#endif

class PDFDoc;


namespace XPDFRendererZoomFactor
{
    const double mode1_zoomFactor = 3.0;
    const double mode2_zoomFactorStage1 = 2.5;
    const double mode2_zoomFactorStage2 = 5.0;
    const double mode2_zoomFactorStage3 = 10.0;
    const double mode3_zoomFactorStage1 = 1.0;
    const double mode3_zoomFactorStage2 = 3.0;
    const double mode4_zoomFactorStart = .25;
    const double mode4_zoomFactorStepSquare = .25;
    const double mode4_zoomFactorIterations = 7;
}

namespace XPDFThreadMaxTimeoutOnExit
{
    const double timeout_ms = 3000;
}

class XPDFRenderer : public PDFRenderer
{
    Q_OBJECT

    public:
        XPDFRenderer(const QString &filename, bool importingFile = false);
        virtual ~XPDFRenderer();

        virtual bool isValid() const override;
        virtual int pageCount() const override;
        virtual QSizeF pageSizeF(int pageNumber) const override;
        virtual int pageRotation(int pageNumber) const override;
        virtual QString title() const override;
        virtual void render(QPainter *p, int pageNumber, const bool cacheAllowed, const QRectF &bounds = QRectF()) override;

    signals:
        void signalUpdateParent();

    private:
        void init();

        struct PdfZoomCacheData {
            PdfZoomCacheData() : splashBitmap(nullptr), cachedPageNumber(-1), splash(nullptr), ratio(1.0), hasToBeProcessed(false) {};
            PdfZoomCacheData(double const a_ratio) : splashBitmap(nullptr), cachedPageNumber(-1), splash(nullptr), ratio(a_ratio), hasToBeProcessed(false) {};
            ~PdfZoomCacheData() {};
            SplashBitmap* splashBitmap;
            //! Note: The 'cachedImage' uses a buffer from 'splash'. Make sure it is invalidated BEFORE 'splash' deallocation.
            QImage cachedImage;
            int cachedPageNumber;
            SplashOutputDev* splash;
            double ratio;
            bool hasToBeProcessed;
            QList<QObject *> updateListAfterProcessing;

            bool requireUpdateImage(int const pageNumber) const {
                return (pageNumber != cachedPageNumber) || (splash == nullptr);
            }

            void prepareNewSplash(int const pageNumber, SplashColor &paperColor)
            {
                if(splash != nullptr)
                {
                    cachedImage = QImage();
                    delete splash;
                }
                splash = new SplashOutputDev(splashModeRGB8, 1, false, paperColor);
                cachedPageNumber = pageNumber;
            }

            void cleanup()
            {
                if(splash != nullptr){
                    cachedImage = QImage();
                    delete splash;
                    splash = nullptr;
                }
            }
        };

        //! Spawned when a pdf processing is required, when no matching image is found in cache.
        class CacheThread : public QThread
        {
        public:
            struct JobData {
                PdfZoomCacheData* cacheData;
                PDFDoc *document;
                int pageNumber;
                double dpiForRendering;
            };

            CacheThread() {}
            ~CacheThread() {}
            void pushJob(JobData &jobData) {               
                QMutexLocker lock(&m_jobMutex);
                m_nextJob.push_back(jobData);
            }

            virtual void run() override;
            bool isJobPending() { QMutexLocker lock(&m_jobMutex); return m_nextJob.size() > 0; }
            void cancelPending() { QMutexLocker lock(&m_jobMutex); m_nextJob.clear(); }
        private:
            QList<JobData> m_nextJob;
            QMutex m_jobMutex;
        };

        CacheThread m_cacheThread;

        QImage &createPDFImageCached(int pageNumber, PdfZoomCacheData &cacheData);
        QImage* createPDFImageHistorical(int pageNumber, qreal xscale, qreal yscale, const QRectF &bounds);

        // Used when 'ZoomBehavior == 1, 2, 3 or 4'.
        // =1 has only x3 zoom in cache (= loss if user zoom > 3.0).
        // =2, has 2.5, 5 and 10 (= no loss, but a bit slower).
        // =3, has 1.0, 2.5, 5 and 10, but downsampled instead of upsampled (= minor quality loss, a bit faster).
        // =4, multithreaded, multiple level of zoom.
        QVector<PdfZoomCacheData> m_pdfZoomCache;
        int const m_pdfZoomMode;

        // Used when 'ZoomBehavior == 0' (no cache).
        SplashBitmap* mpSplashBitmapHistorical;
        // Used when 'ZoomBehavior == 0' (no cache).
        SplashOutputDev* mSplashHistorical;

        PDFDoc *mDocument;
        static QAtomicInt sInstancesCount;
        qreal mSliceX;
        qreal mSliceY;

private slots:
        void OnThreadFinished();
};

#endif // XPDFRENDERER_H
