/*
 * UBDocumentThumbnailWidget.cpp
 *
 *  Created on: Dec 8, 2008
 *      Author: Luc
 */

#include "UBDocumentThumbnailWidget.h"

#include "core/UBApplication.h"
#include "core/UBMimeData.h"
#include "core/UBSettings.h"


UBDocumentThumbnailWidget::UBDocumentThumbnailWidget(QWidget* parent)
    : UBThumbnailWidget(parent)
    , mDropCaretRectItem(0)
    , mClosestDropItem(0)
    , mDragEnabled(true)
{
    // NOOP
}


UBDocumentThumbnailWidget::~UBDocumentThumbnailWidget()
{
    // NOOP
}


void UBDocumentThumbnailWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragEnabled())
    {
        event->ignore();
        return;
    }

    if (!(event->buttons() & Qt::LeftButton))
        return;

    if ((event->pos() - mMousePressPos).manhattanLength()
             < QApplication::startDragDistance())
             return;

    QList<QGraphicsItem*> graphicsItems = items(mMousePressPos);

    UBSceneThumbnailPixmap* sceneItem = 0;

    while (!graphicsItems.isEmpty() && !sceneItem)
    {
        sceneItem = dynamic_cast<UBSceneThumbnailPixmap*>(graphicsItems.takeFirst());
    }

    if (sceneItem)
    {
        QDrag *drag = new QDrag(this);
        QList<UBMimeDataItem> mimeDataItems;
        foreach (QGraphicsItem *item, selectedItems())
            mimeDataItems.append(UBMimeDataItem(sceneItem->proxy(), mGraphicItems.indexOf(item)));
        UBMimeData *mime = new UBMimeData(mimeDataItems);
        drag->setMimeData(mime);

        drag->setPixmap(sceneItem->pixmap().scaledToWidth(100));
        drag->setHotSpot(QPoint(drag->pixmap().width()/2,
                                     drag->pixmap().height() / 2));

        drag->exec(Qt::MoveAction);
    }

    UBThumbnailWidget::mouseMoveEvent(event);
}

void UBDocumentThumbnailWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage))
    {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
        return;
    }

    UBThumbnailWidget::dragEnterEvent(event);
}

void UBDocumentThumbnailWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    deleteDropCaret();
}

void UBDocumentThumbnailWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QList<UBSceneThumbnailPixmap*> pixmapItems;
    foreach (QGraphicsItem *item, scene()->items(mapToScene(frameRect())))
    {
        UBSceneThumbnailPixmap* sceneItem = dynamic_cast<UBSceneThumbnailPixmap*>(item);
        if (sceneItem)
            pixmapItems.append(sceneItem);
    }

    int minDistance = 0;
    QGraphicsItem *underlyingItem = itemAt(event->pos());
    mClosestDropItem = dynamic_cast<UBSceneThumbnailPixmap*>(underlyingItem);

    if (!mClosestDropItem)
    {
        foreach (UBSceneThumbnailPixmap *item, pixmapItems)
        {
            qreal scale = item->transform().m11();
            QPointF itemCenter(
                item->pos().x() + item->boundingRect().width() * scale / 2,
                item->pos().y() + item->boundingRect().height() * scale / 2);

            int distance = (itemCenter.toPoint() - mapToScene(event->pos()).toPoint()).manhattanLength();
            if (!mClosestDropItem || distance < minDistance)
            {
                mClosestDropItem = item;
                minDistance = distance;
            }
        }
    }

    if (mClosestDropItem)
    {
        qreal scale = mClosestDropItem->transform().m11();

        QPointF itemCenter(
            mClosestDropItem->pos().x() + mClosestDropItem->boundingRect().width() * scale / 2,
            mClosestDropItem->pos().y() + mClosestDropItem->boundingRect().height() * scale / 2);

        mDropIsRight = mapToScene(event->pos()).x() > itemCenter.x();

        if (!mDropCaretRectItem && selectedItems().count() < mGraphicItems.count())
        {
            mDropCaretRectItem = new QGraphicsRectItem(0, scene());
            mDropCaretRectItem->setPen(QPen(Qt::darkGray));
            mDropCaretRectItem->setBrush(QBrush(Qt::lightGray));
        }

        QRectF dropCaretRect(
            mDropIsRight ? mClosestDropItem->pos().x() + mClosestDropItem->boundingRect().width() * scale + spacing() / 2 - 1 : mClosestDropItem->pos().x() - spacing() / 2 - 1,
            mClosestDropItem->pos().y(),
            3,
            mClosestDropItem->boundingRect().height() * scale);

        if (mDropCaretRectItem)
            mDropCaretRectItem->setRect(dropCaretRect);
    }

    event->acceptProposedAction();
}

void UBDocumentThumbnailWidget::dropEvent(QDropEvent *event)
{
    deleteDropCaret();

    if (mClosestDropItem)
    {
        int targetIndex = mDropIsRight ? mGraphicItems.indexOf(mClosestDropItem) + 1 : mGraphicItems.indexOf(mClosestDropItem);

        QList<UBMimeDataItem> mimeDataItems;
        if (event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage))
        {
            const UBMimeData* mimeData = qobject_cast<const UBMimeData*>(event->mimeData());
            if (mimeData)
                mimeDataItems = mimeData->items();
        }

        if (1 == mimeDataItems.count() &&
            (mimeDataItems.at(0).sceneIndex() == mGraphicItems.indexOf(mClosestDropItem) ||
             targetIndex == mimeDataItems.at(0).sceneIndex() ||
             targetIndex == mimeDataItems.at(0).sceneIndex() + 1))
        {
            return;
        }

        int sourceIndexOffset = 0;
        int actualTargetIndex = targetIndex;
        for (int i = mimeDataItems.count() - 1; i >= 0; i--)
        {
            UBMimeDataItem sourceItem = mimeDataItems.at(i);
            int actualSourceIndex = sourceItem.sceneIndex();
            if (sourceItem.sceneIndex() >= targetIndex)
                actualSourceIndex += sourceIndexOffset;

            event->acceptProposedAction();

            if (sourceItem.sceneIndex() < targetIndex)
            {
                if (actualSourceIndex != actualTargetIndex - 1)
                    emit sceneDropped(sourceItem.documentProxy(), actualSourceIndex, actualTargetIndex - 1);
                actualTargetIndex -= 1;
            }
            else
            {
                if (actualSourceIndex != actualTargetIndex)
                    emit sceneDropped(sourceItem.documentProxy(), actualSourceIndex, actualTargetIndex);
                sourceIndexOffset += 1;
            }
        }
    }
}

void UBDocumentThumbnailWidget::deleteDropCaret()
{
    if (mDropCaretRectItem && scene())
    {
        scene()->removeItem(mDropCaretRectItem);
        delete mDropCaretRectItem;
        mDropCaretRectItem = 0;
    }
}


void UBDocumentThumbnailWidget::setGraphicsItems(const QList<QGraphicsItem*>& pGraphicsItems,
    const QList<QUrl>& pItemPaths, const QStringList pLabels,
    const QString& pMimeType)
{
    deleteDropCaret();

    UBThumbnailWidget::setGraphicsItems(pGraphicsItems, pItemPaths, pLabels, pMimeType);
}

void UBDocumentThumbnailWidget::setDragEnabled(bool enabled)
{
    mDragEnabled = enabled;
}

bool UBDocumentThumbnailWidget::dragEnabled() const
{
    return mDragEnabled;
}

void UBDocumentThumbnailWidget::hightlightItem(int index)
{
    if (0 <= index && index < mLabelsItems.length())
    {
        mLabelsItems.at(index)->highlight();
    }
    if (0 <= index && index < mGraphicItems.length())
    {
        UBSceneThumbnailPixmap *thumbnail = dynamic_cast<UBSceneThumbnailPixmap*>(mGraphicItems.at(index));
        if (thumbnail)
            thumbnail->highlight();
    }
}
