/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBSvgSubsetRasterizer.h"

#include "domain/UBGraphicsScene.h"
#include "adaptors/UBSvgSubsetAdaptor.h"

#include "core/memcheck.h"

UBSvgSubsetRasterizer::UBSvgSubsetRasterizer(UBDocumentProxy* document, int pageIndex, QObject* parent)
    : QObject(parent)
    , mDocument(document)
    , mPageIndex(pageIndex)
{
    // NOOP
}


UBSvgSubsetRasterizer::~UBSvgSubsetRasterizer()
{
    // NOOP
}


bool UBSvgSubsetRasterizer::rasterizeToFile(const QString& filename)
{
    if (QFile::exists(filename))
    {
        if (!QFile::remove(filename))
            return false;
    }

    UBGraphicsScene* scene = UBSvgSubsetAdaptor::loadScene(mDocument, mPageIndex);

    if (!scene)
        return false;

    QRectF sceneRect = scene->normalizedSceneRect();

    qreal width = sceneRect.width();
    qreal height = sceneRect.height();

    QImage image(width, height, QImage::Format_ARGB32);
    QRectF imageRect(0, 0, width, height);

    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (scene->isDarkBackground())
    {
        painter.fillRect(imageRect, Qt::black);
    }
    else
    {
        painter.fillRect(imageRect, Qt::white);
    }

    scene->setRenderingQuality(UBItem::RenderingQualityHigh);
    scene->setRenderingContext(UBGraphicsScene::NonScreen);

    scene->render(&painter, imageRect, sceneRect, Qt::KeepAspectRatio);

    scene->setRenderingQuality(UBItem::RenderingQualityNormal);
    scene->setRenderingContext(UBGraphicsScene::Screen);

    bool success = image.save(filename, "JPG", 100);

    delete scene;

    return success;
}
