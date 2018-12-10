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

THIRD_PARTY_WARNINGS_DISABLE
#include <poppler/Object.h>
#include <poppler/GlobalParams.h>
#include <poppler/SplashOutputDev.h>
#include <poppler/PDFDoc.h>
THIRD_PARTY_WARNINGS_ENABLE

class PDFDoc;

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
        QImage* createPDFImage(int pageNumber, qreal xscale = 0.5, qreal yscale = 0.5, const QRectF &bounds = QRectF());

        PDFDoc *mDocument;
        static QAtomicInt sInstancesCount;
        qreal mSliceX;
        qreal mSliceY;

        SplashBitmap* mpSplashBitmap;
        SplashOutputDev* mSplash;
};

#endif // XPDFRENDERER_H
