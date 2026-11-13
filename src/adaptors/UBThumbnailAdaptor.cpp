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

#include <QtCore>

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBPersistenceManager.h"
#include "core/UBDocumentStorage.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "document/UBDocumentProxy.h"

#include "domain/UBGraphicsScene.h"

#include "UBSvgSubsetAdaptor.h"

#include "core/memcheck.h"

QHash<QString, QPixmap> UBThumbnailAdaptor::sPendingThumbnails;

void UBThumbnailAdaptor::writeThumbnail(const QString& fileName, const QPixmap& pixmap)
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "JPG");
    UBPersistenceManager::persistenceManager()->saveThumbnail(fileName, data);
}

void UBThumbnailAdaptor::writePendingThumbnails()
{
    // The thumbnails are not removed here: each one is removed by thumbnailWritten() when the
    // persistence thread confirms the write, so that it is not lost if writing fails again.
    for (auto it = sPendingThumbnails.cbegin(); it != sPendingThumbnails.cend(); ++it)
    {
        writeThumbnail(it.key(), it.value());
    }
}

void UBThumbnailAdaptor::thumbnailWritten(const QString& fileName)
{
    sPendingThumbnails.remove(fileName);
}

void UBThumbnailAdaptor::discardThumbnails(const QString& documentPath)
{
    // the page files are renumbered: thumbnails not written would belong to another page
    const QString prefix = documentPath + "/";

    for (auto it = sPendingThumbnails.begin(); it != sPendingThumbnails.end();)
    {
        if (it.key().startsWith(prefix))
        {
            QFile::remove(it.key()); // created again from the page when needed
            it = sPendingThumbnails.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void UBThumbnailAdaptor::generateMissingThumbnails(std::shared_ptr<UBDocumentProxy> proxy)
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

            std::shared_ptr<UBGraphicsScene> scene = UBSvgSubsetAdaptor::loadScene(proxy, iPageNo);

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

QPixmap UBThumbnailAdaptor::generateMissingThumbnail(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex)
{
    QString thumbFileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex);

    QFile thumbFile(thumbFileName);

    if (!thumbFile.exists())
    {
        std::shared_ptr<UBGraphicsScene> scene = UBSvgSubsetAdaptor::loadScene(proxy, pageIndex);

        if (scene)
        {
            persistScene(proxy, scene, pageIndex);
        }
    }

    if (sPendingThumbnails.contains(thumbFileName))
    {
        return sPendingThumbnails.value(thumbFileName);
    }

    QPixmap pix;
    pix.load(thumbFileName);
    return pix;
}

QPixmap UBThumbnailAdaptor::get(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex);

    if (sPendingThumbnails.contains(fileName))
    {
        return sPendingThumbnails.value(fileName);
    }

    if (!UBPersistenceManager::persistenceManager()->documentStorage()->isAvailable())
    {
        // do not access the storage, and do not try to generate all missing thumbnails
        return QPixmap();
    }

    QFile file(fileName);
    if (!file.exists())
    {
        generateMissingThumbnails(proxy);
    }

    QPixmap pix;
    if (file.exists())
    {
        pix.load(fileName, 0, Qt::AutoColor);
    }
    return pix;
}

void UBThumbnailAdaptor::load(std::shared_ptr<UBDocumentProxy> proxy, QList<std::shared_ptr<QPixmap>>& list)
{
    list.clear();
    UBApplication::showMessage(tr("Loading thumbnails (%1 pages)").arg(proxy->pageCount()));
    for(int i=0; i<proxy->pageCount(); i++)
    {
        list.append(std::make_shared<QPixmap>(get(proxy, i)));
    }
}

void UBThumbnailAdaptor::persistScene(std::shared_ptr<UBDocumentProxy> proxy, std::shared_ptr<UBGraphicsScene> pScene, int pageIndex, bool overrideModified)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex);

    // while the storage is not available, the thumbnail is kept in memory
    const bool available = UBPersistenceManager::persistenceManager()->documentStorage()->isAvailable();
    const bool missing = available ? !QFile::exists(fileName) : !sPendingThumbnails.contains(fileName);

    if (pScene->isModified() || overrideModified || missing)
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

        const QPixmap pixmap = QPixmap::fromImage(thumb.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        // kept in memory until written, also while the storage is not available
        sPendingThumbnails.insert(fileName, pixmap);

        if (available)
        {
            writeThumbnail(fileName, pixmap);
        }
    }
}


QUrl UBThumbnailAdaptor::thumbnailUrl(std::shared_ptr<UBDocumentProxy> proxy, int pageIndex)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex);

    return QUrl::fromLocalFile(fileName);
}
