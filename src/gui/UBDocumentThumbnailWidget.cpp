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




#include "UBDocumentThumbnailWidget.h"

#include "core/UBApplication.h"
#include "core/UBMimeData.h"

#include "document/UBDocument.h"

#include "gui/UBThumbnail.h"
#include "gui/UBThumbnailArranger.h"
#include "gui/UBThumbnailScene.h"

#include "core/memcheck.h"


UBDocumentThumbnailWidget::UBDocumentThumbnailWidget(QWidget* parent)
    : UBDocumentThumbnailsView(parent)
    , mDropCaretRectItem(0)
    , mClosestDropItem(0)
    , mDragEnabled(true)
    , mScrollMagnitude(0)
{
    bCanDrag = false;
    mScrollTimer = new QTimer(this);
    connect(mScrollTimer, SIGNAL(timeout()), this, SLOT(autoScroll()));
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
        UBDocumentThumbnailsView::mouseMoveEvent(event);
        return;
    }

    if (!(event->buttons() & Qt::LeftButton))
    {
        UBDocumentThumbnailsView::mouseMoveEvent(event);
        return;
    }

    if ((event->pos() - mMousePressPos).manhattanLength() < QApplication::startDragDistance())
    {
        UBDocumentThumbnailsView::mouseMoveEvent(event);
        return;
    }

    QList<QGraphicsItem*> graphicsItems = items(mMousePressPos);

    UBThumbnail* sceneItem = 0;

    while (!graphicsItems.isEmpty() && !sceneItem)
        sceneItem = dynamic_cast<UBThumbnail*>(graphicsItems.takeFirst());

    if (sceneItem)
    {
        QDrag *drag = new QDrag(this);
        QList<UBMimeDataItem> mimeDataItems;
        foreach (QGraphicsItem *item, selectedItems())
        {
            UBThumbnail* thumbnail = dynamic_cast<UBThumbnail*>(item);

            if (thumbnail)
            {
                mimeDataItems.append(UBMimeDataItem(document()->proxy(), thumbnail->sceneIndex()));
            }
        }

        UBMimeData *mime = new UBMimeData(mimeDataItems);
        drag->setMimeData(mime);

        drag->setPixmap(sceneItem->pixmap().scaledToWidth(100));
        drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height() / 2));

        drag->exec({Qt::MoveAction, Qt::CopyAction});
    }

    UBDocumentThumbnailsView::mouseMoveEvent(event);
}

void UBDocumentThumbnailWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage))
    {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
        return;
    }

    UBDocumentThumbnailsView::dragEnterEvent(event);
}

void UBDocumentThumbnailWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    if (mScrollTimer->isActive())
    {
        mScrollMagnitude = 0;
        mScrollTimer->stop();
    }
    deleteDropCaret();
    UBDocumentThumbnailsView::dragLeaveEvent(event);
}

void UBDocumentThumbnailWidget::autoScroll()
{
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() + mScrollMagnitude);
}

void UBDocumentThumbnailWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QRect boundingFrame = frameRect();
    // setting up automatic scrolling area depending on thumbnail size
    const int SCROLL_DISTANCE = thumbnailArranger()->thumbnailWidth() / 2;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QPoint eventPos = event->position().toPoint();
#else
    QPoint eventPos = event->pos();
#endif
    int bottomDist = boundingFrame.bottom() - eventPos.y(), topDist = boundingFrame.top() - eventPos.y();
    if(qAbs(bottomDist) <= SCROLL_DISTANCE)
    {
        mScrollMagnitude = (SCROLL_DISTANCE - bottomDist)*4;
        if(verticalScrollBar()->isVisible() && !mScrollTimer->isActive()) mScrollTimer->start(100);
    }
    else if(qAbs(topDist) <= SCROLL_DISTANCE)
    {
        mScrollMagnitude = (- SCROLL_DISTANCE - topDist)*4;
        if(verticalScrollBar()->isVisible() && !mScrollTimer->isActive()) mScrollTimer->start(100);
    }
    else
    {
        mScrollMagnitude = 0;
        mScrollTimer->stop();
    }

    qreal minDistance = 0;
    QGraphicsItem *underlyingItem = itemAt(eventPos);
    mClosestDropItem = dynamic_cast<UBThumbnail*>(underlyingItem);

    if (!mClosestDropItem)
    {
        foreach (QGraphicsItem *graphicsItem, scene()->items(mapToScene(boundingFrame)))
        {
            UBThumbnail* item = dynamic_cast<UBThumbnail*>(graphicsItem);

            if (item)
            {
                QPointF itemCenter(item->sceneBoundingRect().center());
                qreal distance = (itemCenter - mapToScene(eventPos)).manhattanLength();

                if (!mClosestDropItem || distance < minDistance)
                {
                    mClosestDropItem = item;
                    minDistance = distance;
                }
            }
        }
    }

    if (mClosestDropItem)
    {
        QPointF itemCenter(mClosestDropItem->sceneBoundingRect().center());

        mDropIsRight = mapToScene(eventPos).x() > itemCenter.x();

        if (!mDropCaretRectItem && selectedItems().count() < document()->thumbnailScene()->thumbnailCount())
        {
            mDropCaretRectItem = new QGraphicsRectItem();
            scene()->addItem(mDropCaretRectItem);
            mDropCaretRectItem->setPen(QPen(Qt::darkGray));
            mDropCaretRectItem->setBrush(QBrush(Qt::lightGray));
        }

        const auto thumbnailWidth = thumbnailArranger()->thumbnailWidth();
        const auto halfSpacingWidth = thumbnailArranger()->spacing().width() / 2;
        const auto offset = mDropIsRight ? thumbnailWidth + halfSpacingWidth : -halfSpacingWidth;
        QRectF dropCaretRect(
                    mClosestDropItem->pos().x() + offset - 1,
                    mClosestDropItem->pos().y(),
                    3,
                    std::ceil(thumbnailWidth / UBSettings::minScreenRatio));

        if (mDropCaretRectItem)
            mDropCaretRectItem->setRect(dropCaretRect);
    }

    event->acceptProposedAction();
}


void UBDocumentThumbnailWidget::dropEvent(QDropEvent *event)
{
    if (mScrollTimer->isActive())
    {
        mScrollMagnitude = 0;
        mScrollTimer->stop();
    }
    deleteDropCaret();

    if (mClosestDropItem)
    {
        int targetIndex = mDropIsRight ? mClosestDropItem->sceneIndex() + 1 : mClosestDropItem->sceneIndex();

        QList<UBMimeDataItem> mimeDataItems;
        if (event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage))
        {
            const UBMimeData* mimeData = qobject_cast<const UBMimeData*>(event->mimeData());
            if (mimeData)
                mimeDataItems = mimeData->items();
        }

        if (1 == mimeDataItems.count() &&
                (mimeDataItems.at(0).sceneIndex() == mClosestDropItem->sceneIndex() ||
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

            //event->acceptProposedAction();
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
    UBDocumentThumbnailsView::dropEvent(event);
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
    document()->thumbnailScene()->hightlightItem(index);
}
