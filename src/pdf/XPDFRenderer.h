
#ifndef XPDFRENDERER_H
#define XPDFRENDERER_H
#include <QImage>
#include "PDFRenderer.h"
#include <splash/SplashBitmap.h>
#include <xpdf/Object.h>
#include <xpdf/GlobalParams.h>
#include <xpdf/SplashOutputDev.h>
#include <xpdf/PDFDoc.h>

class PDFDoc;

class XPDFRenderer : public PDFRenderer
{
    Q_OBJECT

    public:
        XPDFRenderer(const QString &filename);
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
        QImage* createPDFImage(int pageNumber, const qreal xscale = 0.5, const qreal yscale = 0.5, const QRectF &bounds = QRectF());

        PDFDoc *mDocument;
        QList<QImage> mThumbs;
        QMap<int, bool> mPagesMap;
        QMap<int, bool> mThumbMap;
        QMap<int, QImage> mNumPageToPageMap;
        static QAtomicInt sInstancesCount;
        qreal mSliceX;
        qreal mSliceY;
        int bPD;
        bool bThumbGenerated;
        bool bPagesGenerated;
        SplashBitmap* mpSplashBitmap;
        SplashOutputDev* mSplash;
};

#endif // XPDFRENDERER_H
