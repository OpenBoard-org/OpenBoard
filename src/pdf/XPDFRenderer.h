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
}

class XPDFRenderer : public PDFRenderer
{
    Q_OBJECT

    public:
        XPDFRenderer(const QString &filename, bool importingFile = false);
        virtual ~XPDFRenderer();

        bool isValid() const;

        virtual int pageCount() const;

        virtual QSizeF pageSizeF(int pageNumber) const;

        virtual int pageRotation(int pageNumber) const;

        virtual QString title() const;

    public slots:
        void render(QPainter *p, int pageNumber, const QRectF &bounds = QRectF());

    private:
        void init();

        struct PdfZoomCacheData {
            PdfZoomCacheData(double const a_ratio) : splashBitmap(nullptr), cachedPageNumber(-1), splash(nullptr), ratio(a_ratio) {};
            ~PdfZoomCacheData() {};
            SplashBitmap* splashBitmap;
            QImage cachedImage;
            int cachedPageNumber;
            SplashOutputDev* splash;
            double const ratio;

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
        };

        QImage &createPDFImageCached(int pageNumber, PdfZoomCacheData &cacheData);
        QImage* createPDFImageHistorical(int pageNumber, qreal xscale, qreal yscale, const QRectF &bounds);

        // Used when 'ZoomBehavior == 1 or 2'.
        // =1 has only x3 zoom in cache (= loss if user zoom > 3.0).
        // =2, has 2.5, 5 and 10 (= no loss, but a bit slower).
        QVector<PdfZoomCacheData> m_pdfZoomCache;

        // Used when 'ZoomBehavior == 0' (no cache).
        SplashBitmap* mpSplashBitmapHistorical;
        // Used when 'ZoomBehavior == 0' (no cache).
        SplashOutputDev* mSplashHistorical;

        PDFDoc *mDocument;
        static QAtomicInt sInstancesCount;
        qreal mSliceX;
        qreal mSliceY;
};

#endif // XPDFRENDERER_H
