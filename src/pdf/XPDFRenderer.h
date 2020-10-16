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

#define XPDFRENDERER_CACHE_ZOOM_IMAGE
//#define XPDFRENDERER_CACHE_ZOOM_WITH_LOSS

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
#ifdef XPDFRENDERER_CACHE_ZOOM_IMAGE

        enum {
#ifndef XPDFRENDERER_CACHE_ZOOM_WITH_LOSS
            NbrZoomCache = 3
#else //XPDFRENDERER_CACHE_ZOOM_WITH_LOSS
            NbrZoomCache = 1
#endif //XPDFRENDERER_CACHE_ZOOM_WITH_LOSS
        };

        class TypeCacheData {
        public:
            TypeCacheData(double const a_ratio) : splashBitmap(nullptr), cachedPageNumber(-1), splash(nullptr), ratio(a_ratio) {};
            ~TypeCacheData() {};
            SplashBitmap* splashBitmap;
            QImage cachedImage;
            int cachedPageNumber;
            SplashOutputDev* splash;
            double const ratio;
        };

        QImage &createPDFImage(int pageNumber, TypeCacheData &cacheData);
#else
        QImage* createPDFImage(int pageNumber, qreal xscale = 0.5, qreal yscale = 0.5, const QRectF &bounds = QRectF());
#endif

        PDFDoc *mDocument;
        static QAtomicInt sInstancesCount;
        //! The image is rendered with a quality above normal, so we can use that same
        //! image while zooming.
        static const double sRatioZoomRendering[NbrZoomCache];
        qreal mSliceX;
        qreal mSliceY;

        QVector<TypeCacheData> m_cache;
};

#endif // XPDFRENDERER_H
