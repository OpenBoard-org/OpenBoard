/*
 * UBSvgSubsetRasterizer.cpp
 *
 *  Created on: 23 mars 2010
 *      Author: Luc
 */

#include "UBSvgSubsetRasterizer.h"

#include "domain/UBGraphicsScene.h"
#include "adaptors/UBSvgSubsetAdaptor.h"

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
