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


#pragma once

#include "PDFRenderer.h"

#include <QBuffer>
#include <QMap>
#include <QPdfDocument>
#include <QPdfPageRenderer>

/**
 * @brief The QtPDFRenderer class is an implementation of the PDFRenderer using
 * the Qt PDF framework. It uses a QPdfPageRenderer to process render requests
 * in the background.
 */
class QtPDFRenderer : public PDFRenderer
{
    Q_OBJECT

public:
    QtPDFRenderer();
    virtual ~QtPDFRenderer();

    virtual void load(const QByteArray& fileData) override;
    virtual bool isValid() const override;
    virtual int pageCount() const override;
    virtual QSizeF pageSizeF(int pageNumber) const override;
    virtual int pageRotation(int pageNumber) const override;
    virtual QSizeF pointSizeF(int pageNumber) const override;

    virtual void render(QPainter* p, int pageNumber, bool const cacheAllowed, const QRectF& bounds = QRectF()) override;

signals:
    void signalUpdateParent();

private slots:
    void onStatusChanged(QPdfDocument::Status status);
    void onPageRendered(int pageNumber, QSize imageSize, const QImage& image, QPdfDocumentRenderOptions options,
                        quint64 requestId);

private:
    struct CacheKey
    {
        int pageNumber{0};
        int zoomIndex{0};

        friend bool operator<(const CacheKey& l, const CacheKey& r)
        {
            return std::tie(l.pageNumber, l.zoomIndex) < std::tie(r.pageNumber, r.zoomIndex);
        }
    };

    struct CacheEntry
    {
        quint64 requestId{0};
        QImage image;
    };

    QImage& createPDFImageCached(CacheKey key);
    QImage createPDFImageUncached(int pageNumber, double scale, const QRectF& bounds);

private:
    QBuffer mBuffer;
    QPdfDocument mPdfDocument;
    QPdfPageRenderer mRenderer;
    mutable QMap<int, QSizeF> mPageSizeCache;
    QMap<CacheKey, CacheEntry> mImageCache;
};
