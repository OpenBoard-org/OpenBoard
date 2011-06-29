
#include "XPDFRenderer.h"

#include <QtGui>

#include <frameworks/UBPlatformUtils.h>

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
    bThumbGenerated = false;
    bPagesGenerated = false;
    mPagesMap.clear();
    mThumbs.clear();
    mThumbMap.clear();
    mScaleX = 0.0;
    mScaleY = 0.0;
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
        qreal xscale = p->worldTransform().m11();
        qreal yscale = p->worldTransform().m22();
        bool bZoomChanged = false;

        if(mScaleX != xscale || mScaleY != yscale)
        {
            mScaleX = xscale;
            mScaleY = yscale;
            bZoomChanged = true;
        }

        // First verify if the thumbnails and the pages are generated
        if(!bThumbGenerated)
        {

            if(!mThumbMap[pageNumber - 1])
            {
                // Generate the thumbnail
                mThumbs << *createPDFImage(pageNumber, xscale, yscale, bounds);
                mThumbMap[pageNumber - 1] = true;
                if(pageNumber == mDocument->getNumPages())
                {
                    bThumbGenerated = true;
                }
            }
        }
        else if(!bPagesGenerated || bZoomChanged)
        {
            if(!mPagesMap[pageNumber - 1] || bZoomChanged)
            {
                // Generate the page
                mNumPageToPageMap[pageNumber] = *createPDFImage(pageNumber, xscale, yscale, bounds);
                mPagesMap[pageNumber - 1] = true;
                if(mPagesMap.size() == mDocument->getNumPages())
                {
                    bPagesGenerated = true;
                }
            }
        }

        // Warning: verify pagenumber
        QImage pdfImage;

        if(!bThumbGenerated)
        {
            pdfImage = mThumbs.at(pageNumber - 1);
        }
        else
        {
            pdfImage = mNumPageToPageMap[pageNumber];
        }

        pdfImage.rect();

        QTransform savedTransform = p->worldTransform();
        p->resetTransform();
        p->drawImage(QPointF(savedTransform.dx() + mSliceX, savedTransform.dy() + mSliceY), pdfImage);
        p->setWorldTransform(savedTransform);
    }
}

QImage* XPDFRenderer::createPDFImage(int pageNumber, const qreal xscale, const qreal yscale, const QRectF &bounds)
{
    QImage* img = new QImage();
    if (isValid())
    {
        SplashColor paperColor = {0xFF, 0xFF, 0xFF}; // white
        mSplash = new SplashOutputDev(splashModeRGB8, 1, gFalse, paperColor);
        mSplash->startDoc(mDocument->getXRef());
        int hResolution = 72;
        int vResolution = 72;
        int rotation = 0; // in degrees (get it from the worldTransform if we want to support rotation)
        GBool useMediaBox = gFalse;
        GBool crop = gTrue;
        GBool printing = gFalse;
        const qreal xScale = xscale;
        const qreal yScale = yscale;
        mSliceX = 0.;
        mSliceY = 0.;

        if (bounds.isNull())
        {
            mDocument->displayPage(mSplash, pageNumber, hResolution * xScale, vResolution * yScale,
                                   rotation, useMediaBox, crop, printing);
        }
        else
        {
            mSliceX = bounds.x() * xScale;
            mSliceY = bounds.y() * yScale;
            qreal sliceW = bounds.width() * xScale;
            qreal sliceH = bounds.height() * yScale;

            mDocument->displayPageSlice(mSplash, pageNumber, hResolution * xScale, vResolution * yScale,
                                        rotation, useMediaBox, crop, printing, mSliceX, mSliceY, sliceW, sliceH);
        }

        mpSplashBitmap = mSplash->getBitmap();
        img = new QImage(mpSplashBitmap->getDataPtr(), mpSplashBitmap->getWidth(), mpSplashBitmap->getHeight(), mpSplashBitmap->getWidth() * 3, QImage::Format_RGB888);
    }
    return img;
}
