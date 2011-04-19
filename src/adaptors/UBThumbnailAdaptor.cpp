
#include "UBThumbnailAdaptor.h"

#include <QtCore>

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBPersistenceManager.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "document/UBDocumentProxy.h"

#include "domain/UBGraphicsScene.h"

#include "UBSvgSubsetAdaptor.h"

UBThumbnailAdaptor::UBThumbnailAdaptor(QObject *parent)
    : QObject(parent)
{
    // NOOP
}


UBThumbnailAdaptor::~UBThumbnailAdaptor()
{
    // NOOP
}


QList<QPixmap> UBThumbnailAdaptor::load(UBDocumentProxy* proxy)
{

    QList<QPixmap> thumbnails;

    if (!proxy || proxy->persistencePath().size() == 0)
        return thumbnails;

    //compatibility with older formats (<= 4.0.b.2.0) : generate missing thumbnails

    int existingPageCount = proxy->pageCount();

    QString thumbFileName = proxy->persistencePath() +
        UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", existingPageCount);

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

    while (moreToProcess)
    {
        pageCount++;

        QString fileName = proxy->persistencePath() +
                UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageCount);

        QFile file(fileName);

        if (file.exists())
        {
            QPixmap pix;

            pix.load(fileName);

            thumbnails.append(pix);
        }
        else
        {
            moreToProcess = false;
        }
    }

    return thumbnails;
}


void UBThumbnailAdaptor::persistScene(const QString& pDocPath, UBGraphicsScene* pScene, const int pageIndex, const bool overrideModified)
{
    QString fileName = pDocPath + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex + 1);

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


QUrl UBThumbnailAdaptor::thumbnailUrl(UBDocumentProxy* proxy, const int pageIndex)
{
    QString fileName = proxy->persistencePath() +
            UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", pageIndex + 1);

    return QUrl::fromLocalFile(fileName);
}
