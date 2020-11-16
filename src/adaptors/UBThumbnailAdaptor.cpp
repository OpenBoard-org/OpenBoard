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




#include "UBThumbnailAdaptor.h"

#include <QtCore>

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBPersistenceManager.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "document/UBDocumentProxy.h"

#include "domain/UBGraphicsScene.h"

#include "UBSvgSubsetAdaptor.h"

#include "core/memcheck.h"

void UBThumbnailAdaptor::generateMissingThumbnails(UBDocumentProxy* proxy)
{
    int existingPageCount = proxy->pageCount();

    for (int iPageNo = 0; iPageNo < existingPageCount; ++iPageNo)
    {
        QString thumbFileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", iPageNo);

        QFile thumbFile(thumbFileName);

        if (!thumbFile.exists())
        {
            bool displayMessage = (existingPageCount > 5);

            int thumbCount = 0;

            UBGraphicsScene* scene = UBSvgSubsetAdaptor::loadScene(proxy, iPageNo);

            if (scene)
            {
                thumbCount++;

                if (displayMessage && thumbCount == 1)
                    UBApplication::showMessage(tr("Generating preview thumbnails ..."));

                persistScene(proxy, scene, iPageNo);
            }

            if (displayMessage && thumbCount > 0)
                UBApplication::showMessage(tr("%1 thumbnails generated ...").arg(thumbCount));

        }
    }
}

const QPixmap* UBThumbnailAdaptor::get(UBDocumentProxy* proxy, int pageIndex)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex);

    QFile file(fileName);
    if (!file.exists())
    {
        generateMissingThumbnails(proxy);
    }

    QPixmap* pix = new QPixmap();
    if (file.exists())
    {
        //Warning. Works only with modified Qt
#ifdef Q_OS_LINUX
        pix->load(fileName, 0, Qt::AutoColor);
#else
        pix->load(fileName, 0, Qt::AutoColor);
#endif
    }
    return pix;
}

void UBThumbnailAdaptor::load(UBDocumentProxy* proxy, QList<const QPixmap*>& list)
{
    generateMissingThumbnails(proxy);

    foreach(const QPixmap* pm, list){
        delete pm;
        pm = NULL;
    }
    list.clear();
    for(int i=0; i<proxy->pageCount(); i++)
        list.append(get(proxy, i));
}

void UBThumbnailAdaptor::persistScene(UBDocumentProxy* proxy, UBGraphicsScene* pScene, int pageIndex, bool overrideModified)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex);

    QFile thumbFile(fileName);

    if (pScene->isModified() || overrideModified || !thumbFile.exists())
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


QUrl UBThumbnailAdaptor::thumbnailUrl(UBDocumentProxy* proxy, int pageIndex)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex);

    return QUrl::fromLocalFile(fileName);
}
