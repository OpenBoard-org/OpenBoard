/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBThumbnailAdaptor.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "document/UBDocument.h"

#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

void UBThumbnailAdaptor::generateMissingThumbnails(UBDocument* document)
{
    int existingPageCount = document->pageCount();

    for (int iPageNo = 0; iPageNo < existingPageCount; ++iPageNo)
    {
        QString thumbFileName = document->thumbnailFile(iPageNo);

        if (!QFile::exists(thumbFileName))
        {
            bool displayMessage = (existingPageCount > 5);

            int thumbCount = 0;

            std::shared_ptr<UBGraphicsScene> scene = document->loadScene(iPageNo);

            if (scene)
            {
                thumbCount++;

                if (displayMessage && thumbCount == 1)
                    UBApplication::showMessage(tr("Generating preview thumbnails ..."));

                persistScene(document, scene, iPageNo);
            }

            if (displayMessage && thumbCount > 0)
                UBApplication::showMessage(tr("%1 thumbnails generated ...").arg(thumbCount));

        }
    }
}

QPixmap UBThumbnailAdaptor::generateMissingThumbnail(UBDocument* document, int pageIndex)
{
    QString thumbFileName = document->thumbnailFile(pageIndex);

    if (!QFile::exists(thumbFileName))
    {
        std::shared_ptr<UBGraphicsScene> scene = document->loadScene(pageIndex);

        if (scene)
        {
            persistScene(document, scene, pageIndex);
        }
    }

    QPixmap pix;
    pix.load(thumbFileName);
    return pix;
}

QPixmap UBThumbnailAdaptor::get(UBDocument* document, int pageIndex)
{
    QString fileName = document->thumbnailFile(pageIndex);

    QFile file(fileName);

    if (!file.exists())
    {
        generateMissingThumbnails(document);
    }

    QPixmap pix;

    if (file.exists())
    {
        pix.load(fileName, 0, Qt::AutoColor);
    }

    return pix;
}

void UBThumbnailAdaptor::persistScene(UBDocument* document, std::shared_ptr<UBGraphicsScene> pScene, int pageIndex, bool overrideModified)
{
    QString fileName = document->thumbnailFile(pageIndex);

    if (pScene->isModified() || overrideModified || !QFile::exists(fileName))
    {
        qreal nominalWidth = pScene->nominalSize().width();
        qreal nominalHeight = pScene->nominalSize().height();
        qreal ratio = nominalWidth / nominalHeight;
        QRectF sceneRect = pScene->normalizedSceneRect(ratio);

        qreal width = UBSettings::maxThumbnailWidth;
        qreal height = width / ratio;

        QImage thumb(width, height, QImage::Format_ARGB32);

        QRectF imageRect(0, 0, width, height);

        QPainter painter(&thumb);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        if (pScene->isDarkBackground())
        {
            painter.fillRect(imageRect, Qt::black);
        }
        else
        {
            painter.fillRect(imageRect, Qt::white);
        }

        pScene->setRenderingContext(UBGraphicsScene::NonScreen);
        pScene->setRenderingQuality(UBItem::RenderingQualityHigh, UBItem::CacheNotAllowed);

        pScene->render(&painter, imageRect, sceneRect, Qt::KeepAspectRatio);

        pScene->setRenderingContext(UBGraphicsScene::Screen);
        pScene->setRenderingQuality(UBItem::RenderingQualityNormal, UBItem::CacheAllowed);

        thumb.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(fileName, "JPG");
    }
}


QUrl UBThumbnailAdaptor::thumbnailUrl(UBDocument* document, int pageIndex)
{
    QString fileName = document->thumbnailFile(pageIndex);

    return QUrl::fromLocalFile(fileName);
}
