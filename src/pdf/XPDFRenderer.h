
#ifndef XPDFRENDERER_H
#define XPDFRENDERER_H

#include "PDFRenderer.h"

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

        PDFDoc *mDocument;

        static QAtomicInt sInstancesCount;
};

#endif // XPDFRENDERER_H
