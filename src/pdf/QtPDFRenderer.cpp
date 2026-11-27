/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#include "QtPDFRenderer.h"

#include <QPainter>

namespace
{
constexpr double zoomFactorStart = .25;
constexpr double zoomFactorStepSquare = .25;
constexpr double zoomFactorIterations = 7;

int indexForZoomFactor(double zoomFactor)
{
    if (zoomFactor <= zoomFactorStart)
    {
        return 0;
    }

    return std::ceil(std::sqrt((zoomFactor - zoomFactorStart) / zoomFactorStepSquare));
}

double zoomFactorForIndex(int index)
{
    return zoomFactorStart + zoomFactorStepSquare * index * index;
}
} // namespace

QtPDFRenderer::QtPDFRenderer()
{
    connect(&mPdfDocument, &QPdfDocument::statusChanged, this, &QtPDFRenderer::onStatusChanged);

    mRenderer.setDocument(&mPdfDocument);
    mRenderer.setRenderMode(QPdfPageRenderer::RenderMode::MultiThreaded);
    connect(&mRenderer, &QPdfPageRenderer::pageRendered, this, &QtPDFRenderer::onPageRendered);
}

QtPDFRenderer::~QtPDFRenderer()
{
}

void QtPDFRenderer::load(const QByteArray& fileData)
{
    setFileData(fileData);
    mBuffer.setBuffer(&mFileData);
    mBuffer.open(QBuffer::ReadOnly);
    mPdfDocument.load(&mBuffer);
}

bool QtPDFRenderer::isValid() const
{
    return mPdfDocument.status() == QPdfDocument::Status::Ready;
}

int QtPDFRenderer::pageCount() const
{
    if (isValid())
    {
        return mPdfDocument.pageCount();
    }
    else
    {
        return 0;
    }
}

QSizeF QtPDFRenderer::pageSizeF(int pageNumber) const
{
    return pointSizeF(pageNumber) * dpiForRendering / 72.0;
}

int QtPDFRenderer::pageRotation(int pageNumber) const
{
    // NOTE Nobody uses this information
    return 0;
}

QSizeF QtPDFRenderer::pointSizeF(int pageNumber) const
{
    if (isValid())
    {
        // cache the page size because it is not available while a page is rendered (mutex locked)
        if (!mPageSizeCache.contains(pageNumber))
        {
            mPageSizeCache.insert(pageNumber, mPdfDocument.pagePointSize(pageNumber - 1));
        }

        return mPageSizeCache[pageNumber];
    }
    else
    {
        return {};
    }
}

void QtPDFRenderer::render(QPainter* p, int pageNumber, const bool cacheAllowed, const QRectF& bounds)
{
    if (isValid())
    {
        const auto xscale = p->worldTransform().m11();
        const auto yscale = p->worldTransform().m22();
        Q_ASSERT(qFuzzyCompare(xscale, yscale)); // Zoom equal in all axes expected.
        Q_ASSERT(xscale > 0.0);                  // Potential Div0 later if this assert fail.

        if (cacheAllowed)
        {
            double devicePixelRatio{1.0};

            if (p->device())
            {
                devicePixelRatio = p->device()->devicePixelRatio();
            }

            const auto zoomRequested = xscale * devicePixelRatio;

            // Choose a zoom which is superior or equivalent than the user choice (= no loss, upscaling).
            auto zoomIndex = indexForZoomFactor(zoomRequested);
            auto pdfImage = createPDFImageCached({pageNumber, zoomIndex});

            if (pdfImage.isNull())
            {
                // try to find a coarser image in the cache
                auto iter = mImageCache.upperBound({pageNumber, zoomIndex});

                while ((--iter).key().pageNumber == pageNumber)
                {
                    if (iter == mImageCache.begin() || !iter->image.isNull())
                    {
                        pdfImage = iter->image;
                        zoomIndex = iter.key().zoomIndex;
                        break;
                    }
                }
            }

            if (pdfImage.isNull())
            {
                // No alternate image found. Build an alternate image in order to display some progress.
                const auto ratioDiff = zoomFactorForIndex(zoomIndex);
                pdfImage = QImage((bounds.size() * ratioDiff).toSize(), QImage::Format_RGB888);
                pdfImage.fill(Qt::white);

                QPainter painter(&pdfImage);
                QString const text = tr("Processing...");
                QFont font = painter.font();

                if (font.pixelSize() != -1)
                {
                    font.setPixelSize(ratioDiff * font.pixelSize());
                }
                else
                {
                    font.setPointSizeF(ratioDiff * font.pointSizeF());
                }

                painter.setFont(font);
                QFontMetrics textMetric(font, &pdfImage);
                QSize textSize = textMetric.size(0, text);
                painter.drawText((bounds.width() * ratioDiff - textSize.width()) / 2,
                                 (bounds.height() * ratioDiff - textSize.height()) / 2, text);
            }

            QTransform savedTransform = p->worldTransform();

            const auto ratioDifferenceBetweenWorldAndImage = 1.0 / zoomFactorForIndex(zoomIndex);
            // The 'pdfImage' is maybe rendered with a different quality than requested. We adjust the 'transform' to zoom it
            // in or out of the required ratio.
            QTransform newTransform = savedTransform.scale(ratioDifferenceBetweenWorldAndImage, ratioDifferenceBetweenWorldAndImage);

            p->setWorldTransform(newTransform);
            p->drawImage(QPointF{0, 0}, pdfImage);
            p->setWorldTransform(savedTransform);
        }
        else
        {
            const auto pdfImage = createPDFImageUncached(pageNumber, xscale, bounds);
            const auto topleft = bounds.topLeft() * xscale;
            QTransform savedTransform = p->worldTransform();

            p->resetTransform();
            p->drawImage(QPointF(savedTransform.dx() + topleft.x(), savedTransform.dy() + topleft.y()), pdfImage);
            p->setWorldTransform(savedTransform);
        }
    }
}

void QtPDFRenderer::onStatusChanged(QPdfDocument::Status status)
{
    const auto hasError = status == QPdfDocument::Status::Error;

    if (hasError)
    {
        qDebug() << "Renderer status" << status << mPdfDocument.error();
    }
    else
    {
        qDebug() << "Renderer status" << status;
    }
}

void QtPDFRenderer::onPageRendered(int pageNumber, QSize imageSize, const QImage& image,
                                 QPdfDocumentRenderOptions options, quint64 requestId)
{
    // find matching cache entry
    ++pageNumber;   // convert zero-based page number of QPdfDocument to 1-based API of renderer
    auto iter = mImageCache.lowerBound({pageNumber, 0});

    while (iter != mImageCache.end() && iter.key().pageNumber == pageNumber && iter->requestId != requestId)
    {
        ++iter;
    }

    if (iter != mImageCache.end() && iter.value().requestId == requestId)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 12, 0))
        iter->image = image;
#else
        iter->image = QImage(imageSize, QImage::Format_ARGB32);
        iter->image.fill(Qt::white);
        QPainter p(&iter->image);
        p.drawImage(0, 0, image);
#endif
        iter->requestId = 0;
    }

    emit signalUpdateParent();
}

QImage& QtPDFRenderer::createPDFImageCached(CacheKey key)
{
    CacheEntry& entry = mImageCache[key];

    if (isValid())
    {
        if (entry.image.isNull() && entry.requestId == 0)
        {
            const QSizeF imageSize = pageSizeF(key.pageNumber) * zoomFactorForIndex(key.zoomIndex);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 12, 0))
            entry.requestId = mRenderer.requestPage(key.pageNumber - 1, imageSize.toSize(), QImage::Format_ARGB32, Qt::white);
#else
            entry.requestId = mRenderer.requestPage(key.pageNumber - 1, imageSize.toSize());
#endif
        }
    }
    else
    {
        entry.image = {};
    }

    return entry.image;
}

QImage QtPDFRenderer::createPDFImageUncached(int pageNumber, double scale, const QRectF& bounds)
{
    QImage image;

    if (isValid())
    {
        QPdfDocumentRenderOptions options;
        options.setScaledSize((pageSizeF(pageNumber) * scale).toSize());
        options.setScaledClipRect(bounds.toRect());

#if (QT_VERSION >= QT_VERSION_CHECK(6, 12, 0))
        image = mPdfDocument.render(pageNumber - 1, bounds.size().toSize(), options, QImage::Format_ARGB32, Qt::white);
#else
        QImage renderedImage = mPdfDocument.render(pageNumber - 1, bounds.size().toSize(), options);
        image = QImage(renderedImage.size(), QImage::Format_ARGB32);
        image.fill(Qt::white);
        QPainter p(&image);
        p.drawImage(0, 0, renderedImage);
#endif
    }

    return image;
}
