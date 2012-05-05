/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBThumbnailAdaptor.h"

#include <QtCore>

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBPersistenceManager.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "board/UBBoardController.h"

#include "document/UBDocumentProxy.h"

#include "domain/UBGraphicsScene.h"

#include "UBSvgSubsetAdaptor.h"

#include "core/memcheck.h"

QList<QPixmap> UBThumbnailAdaptor::load(UBDocumentProxy* proxy)
{
    QList<QPixmap> thumbnails;

    if (!proxy || proxy->persistencePath().size() == 0)
        return thumbnails;

    //compatibility with older formats (<= 4.0.b.2.0) : generate missing thumbnails

    int existingPageCount = proxy->pageCount();

    QString thumbFileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", existingPageCount);

    QFile thumbFile(thumbFileName);

    if (!thumbFile.exists())
    {
        bool displayMessage = (existingPageCount > 5);

        int thumbCount = 0;

        for(int i = 0 ; i < existingPageCount; i++)
        {
            UBGraphicsScene* scene = UBSvgSubsetAdaptor::loadScene(proxy, i);

            if (scene)
            {
                thumbCount++;

                if (displayMessage && thumbCount == 1)
					UBApplication::showMessage(tr("Generating preview thumbnails ..."));

                persistScene(proxy->persistencePath(), scene, i);
            }
        }

        if (displayMessage && thumbCount > 0)
            UBApplication::showMessage(tr("%1 thumbnails generated ...").arg(thumbCount));

    }

    //end compatibility with older format

    bool moreToProcess = true;
    int pageCount = 0;

    while (moreToProcess) {
        pageCount++;
        QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageCount);

        QFile file(fileName);
        if (file.exists()) {
            QPixmap pix;
            //Warning. Works only with modified Qt
#ifdef Q_WS_X11
            pix.load(fileName, 0, Qt::AutoColor);
#else
            pix.load(fileName, 0, Qt::AutoColor, false);
#endif
            thumbnails.append(pix);
        } else {
            moreToProcess = false;
        }
    }
    return thumbnails;
}

QPixmap UBThumbnailAdaptor::load(UBDocumentProxy* proxy, int index)
{
    int existingPageCount = proxy->pageCount();

	if (!proxy || proxy->persistencePath().size() == 0 || index < 0 || index >= existingPageCount)
		return QPixmap();
    //compatibility with older formats (<= 4.0.b.2.0) : generate missing thumbnails

    QString thumbFileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", existingPageCount);

    QFile thumbFile(thumbFileName);

    if (!thumbFile.exists())
    {
        bool displayMessage = (existingPageCount > 5);

        int thumbCount = 0;

        for(int i = 0 ; i < existingPageCount; i++)
        {
            UBGraphicsScene* scene = UBSvgSubsetAdaptor::loadScene(proxy, i);

            if (scene)
            {
                thumbCount++;

                if (displayMessage && thumbCount == 1)
					UBApplication::showMessage(tr("Generating preview thumbnails ..."));

                persistScene(proxy->persistencePath(), scene, i);
            }
        }

        if (displayMessage && thumbCount > 0)
            UBApplication::showMessage(tr("%1 thumbnails generated ...").arg(thumbCount));

    }

    //end compatibility with older format
//        QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", index + 1);
        QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", UBApplication::boardController->pageFromSceneIndex(index));

        QFile file(fileName);
        if (file.exists())
		{
            QPixmap pix;
            //Warning. Works only with modified Qt
#ifdef Q_WS_X11
            pix.load(fileName, 0, Qt::AutoColor);
#else
            pix.load(fileName, 0, Qt::AutoColor, false);
#endif
			return pix;
		}
		return QPixmap();
}

void UBThumbnailAdaptor::persistScene(const QString& pDocPath, UBGraphicsScene* pScene, int pageIndex, bool overrideModified)
{
    QString fileName = pDocPath + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", UBApplication::boardController->pageFromSceneIndex(pageIndex));

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
        pScene->setRenderingQuality(UBItem::RenderingQualityHigh);

        pScene->render(&painter, imageRect, sceneRect, Qt::KeepAspectRatio);

        pScene->setRenderingContext(UBGraphicsScene::Screen);
        pScene->setRenderingQuality(UBItem::RenderingQualityNormal);

        thumb.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(fileName, "JPG");
    }
}


QUrl UBThumbnailAdaptor::thumbnailUrl(UBDocumentProxy* proxy, int pageIndex)
{
    QString fileName = proxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", UBApplication::boardController->pageFromSceneIndex(pageIndex));

    return QUrl::fromLocalFile(fileName);
}
