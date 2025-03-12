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



#include <QString>
#include <QCursor>
#include <QGraphicsRectItem>

#include "UBDocumentThumbnailsView.h"

#include <QWidget>

#include "core/UBSettings.h"
#include "core/UBApplication.h"

#include "document/UBDocument.h"
#include "document/UBDocumentController.h"

#include "gui/UBThumbnail.h"
#include "gui/UBThumbnailArranger.h"
#include "gui/UBThumbnailScene.h"

#include "core/memcheck.h"


UBDocumentThumbnailsView::UBDocumentThumbnailsView(QWidget* parent)
    : UBThumbnailsView(parent)
    , mThumbnailWidth(UBSettings::defaultThumbnailWidth)
    , mSelectionSpan(0)

{
    setThumbnailArranger(new UBDocumentThumbnailArranger(this));

    // By default, the drag is possible
    bCanDrag = true;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    setFrameShape(QFrame::NoFrame);

    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}


UBDocumentThumbnailsView::~UBDocumentThumbnailsView()
{
}

void UBDocumentThumbnailsView::setDocument(std::shared_ptr<UBDocument> document)
{
    mDocument = document;

    if (document)
    {
        auto thumbnailScene = document->thumbnailScene();

        setScene(thumbnailScene);
        thumbnailScene->arrangeThumbnails();
    }
    else
    {
        setScene(nullptr);
    }
}

std::shared_ptr<UBDocument> UBDocumentThumbnailsView::document() const
{
    return mDocument;
}


void UBDocumentThumbnailsView::setThumbnailWidth(qreal pThumbnailWidth)
{
    if (mThumbnailWidth != pThumbnailWidth)
    {
        mThumbnailWidth = pThumbnailWidth;

        emit thumbnailWidthChanged(mThumbnailWidth);

        if (mDocument)
        {
            auto thumbnailScene = mDocument->thumbnailScene();
            thumbnailScene->arrangeThumbnails();

            if (thumbnailScene->lastSelectedThumbnail())
            {
                ensureVisible(thumbnailScene->lastSelectedThumbnail());
            }
        }
    }
}


QList<QGraphicsItem*> UBDocumentThumbnailsView::selectedItems()
{
    if (mDocument)
    {
        QList<QGraphicsItem*> sortedSelectedItems = mDocument->thumbnailScene()->selectedItems();
        std::sort(sortedSelectedItems.begin(), sortedSelectedItems.end(), thumbnailLessThan);
        return sortedSelectedItems;
    }

    return {};
}

void UBDocumentThumbnailsView::clearSelection()
{
    if (mDocument)
    {
        mDocument->thumbnailScene()->clearSelection();
    }
}

void UBDocumentThumbnailsView::setBackgroundBrush(const QBrush& brush)
{
    if (mDocument)
    {
        mDocument->thumbnailScene()->setBackgroundBrush(brush);
    }
}


void UBDocumentThumbnailsView::mousePressEvent(QMouseEvent *event)
{
    mClickTime.restart();
    mMousePressPos = event->pos();

    // first ask the thumbnails to process the event for the UI buttons
    QGraphicsView::mousePressEvent(event);

    // do not further process event if it was one of the UI buttons
    if (event->isAccepted())
    {
        return;
    }

    UBThumbnail* sceneItem = dynamic_cast<UBThumbnail*>(itemAt(mMousePressPos));

    if (!sceneItem)
    {
        event->ignore();
        return;
    }

    auto thumbnailScene = document()->thumbnailScene();

    if (Qt::ShiftModifier & event->modifiers())
    {
        if (thumbnailScene->lastSelectedThumbnail())
        {
            int index1 = thumbnailScene->lastSelectedThumbnail()->sceneIndex();
            int index2 = sceneItem->sceneIndex();

            mSelectionSpan = index2 - index1;
            selectItems(qMin(index1, index2), mSelectionSpan < 0 ? - mSelectionSpan + 1 : mSelectionSpan + 1);
        }
    }
    else
    {
        if (!sceneItem->isSelected())
        {
            int index = sceneItem->sceneIndex();
            selectItemAt(index, Qt::ControlModifier & event->modifiers());
        }
        else
        {
            thumbnailScene->hightlightItem(sceneItem->sceneIndex(), false, false);
        }

        mSelectionSpan = 0;
    }

    UBApplication::documentController->pageSelectionChanged();
}


void UBDocumentThumbnailsView::mouseMoveEvent(QMouseEvent *event)
{
    int distance = (mMousePressPos - event->pos()).manhattanLength();

    if (0 == (event->buttons() & Qt::LeftButton) || distance < QApplication::startDragDistance())
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    bSelectionInProgress = false;
    if (0 == selectedItems().size())
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if(bCanDrag)
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mime = new QMimeData();

        if (mMimeType.length() > 0)
            mime->setData(mMimeType, QByteArray()); // trick the d&d system to register our own mime type

        drag->setMimeData(mime);

        QList<QUrl> qlElements;

        foreach (QGraphicsItem* item, selectedItems())
        {
            UBThumbnail* thumbnail = dynamic_cast<UBThumbnail*>(item);

            if (thumbnail)
            {
                const auto pageNumber = UBDocumentContainer::pageFromSceneIndex(thumbnail->sceneIndex());
                const auto itemPath = QUrl::fromLocalFile(document()->proxy()->persistencePath() + QString("/pages/%1").arg(pageNumber));
                qlElements << itemPath;
            }
        }

        if (qlElements.size() > 0){
                        mime->setUrls(qlElements);
                        drag->setMimeData(mime);
                        drag->exec(Qt::CopyAction);
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}


void UBDocumentThumbnailsView::mouseReleaseEvent(QMouseEvent *event)
{
    int elapsedTimeSincePress = mClickTime.elapsed();
    prevMoveMousePos = QPoint();

    if(elapsedTimeSincePress < STARTDRAGTIME) {
        emit mouseClick(itemAt(event->pos()), 0);
    }

    // do not forward event to parent class as this affects selection
}


void UBDocumentThumbnailsView::keyPressEvent(QKeyEvent *event)
{
    auto thumbnailScene = document()->thumbnailScene();

    if (thumbnailScene->lastSelectedThumbnail())
    {
        const int startSelectionIndex = thumbnailScene->lastSelectedThumbnail()->sceneIndex();
        const int previousSelectedThumbnailIndex = startSelectionIndex + mSelectionSpan;

        switch (event->key())
        {
        case Qt::Key_Down:
        case Qt::Key_Up:
            {
                if (rowCount() <= 1) break;
                if (Qt::ShiftModifier & event->modifiers())
                {
                    int endSelectionIndex;
                    if (Qt::Key_Down == event->key())
                    {
                        endSelectionIndex = previousSelectedThumbnailIndex + columnCount();
                        if (endSelectionIndex >= document()->thumbnailScene()->thumbnailCount()) break;
                    }
                    else
                    {
                        endSelectionIndex = previousSelectedThumbnailIndex - columnCount();
                        if (endSelectionIndex < 0) break;
                    }

                    const int startIndex = startSelectionIndex < endSelectionIndex ? startSelectionIndex : endSelectionIndex;
                    const int count = startSelectionIndex < endSelectionIndex ? endSelectionIndex - startSelectionIndex + 1 : startSelectionIndex - endSelectionIndex + 1;
                    mSelectionSpan = startSelectionIndex < endSelectionIndex ? (count - 1) : - (count - 1);
                    selectItems(startIndex, count);
                }
                else
                {
                    int toSelectIndex;
                    if (Qt::Key_Down == event->key())
                    {
                        toSelectIndex = previousSelectedThumbnailIndex + columnCount();
                        if (toSelectIndex >= document()->thumbnailScene()->thumbnailCount()) break;
                    }
                    else
                    {
                        toSelectIndex = previousSelectedThumbnailIndex - columnCount();
                        if (toSelectIndex < 0) break;
                    }

                    selectItemAt(toSelectIndex, Qt::ControlModifier & event->modifiers());
                    mSelectionSpan = 0;
                }
            }
            break;

        case Qt::Key_Left:
        case Qt::Key_Right:
            {
            UBThumbnail *previousSelectedThumbnail = document()->thumbnailScene()->thumbnailAt(previousSelectedThumbnailIndex);

                if (!previousSelectedThumbnail)
                {
                    break;
                }

                if (Qt::Key_Left == event->key())
                {
                    if (0 == previousSelectedThumbnail->column()) break;
                }
                else
                {
                    if (previousSelectedThumbnail->column() == columnCount() - 1 ||
                        previousSelectedThumbnailIndex == document()->thumbnailScene()->thumbnailCount() - 1) break;
                }

                if (Qt::ShiftModifier & event->modifiers())
                {
                    int endSelectionIndex;
                    if (Qt::Key_Left == event->key())
                    {
                        endSelectionIndex = previousSelectedThumbnailIndex - 1;
                        if (endSelectionIndex < 0) break;
                    }
                    else
                    {
                        endSelectionIndex = previousSelectedThumbnailIndex + 1;
                        if (endSelectionIndex >= document()->thumbnailScene()->thumbnailCount()) break;
                    }

                    const int startIndex = startSelectionIndex < endSelectionIndex ? startSelectionIndex : endSelectionIndex;
                    const int count = startSelectionIndex < endSelectionIndex ? endSelectionIndex - startSelectionIndex + 1 : startSelectionIndex - endSelectionIndex + 1;
                    mSelectionSpan = startSelectionIndex < endSelectionIndex ? (count - 1) : - (count - 1);
                    selectItems(startIndex, count);
                }
                else
                {
                    const auto selectedIndex = previousSelectedThumbnailIndex + (Qt::Key_Left == event->key() ? -1 : 1);
                    const auto selectedItem = document()->thumbnailScene()->thumbnailAt(selectedIndex);

                    selectItemAt(selectedIndex, Qt::ControlModifier & event->modifiers());
                    mSelectionSpan = 0;
                }
            }
            break;

        case Qt::Key_Home:
            {
                if (Qt::ShiftModifier & event->modifiers())
                {
                    mSelectionSpan = - startSelectionIndex;
                    selectItems(0, startSelectionIndex + 1);
                }
                else
                {
                    selectItemAt(0, Qt::ControlModifier & event->modifiers());
                    mSelectionSpan = 0;
                }
            }
            break;

        case Qt::Key_End:
            {
                if (Qt::ShiftModifier & event->modifiers())
                {
                    mSelectionSpan = document()->thumbnailScene()->thumbnailCount() - startSelectionIndex - 1;
                    selectItems(startSelectionIndex, mSelectionSpan + 1);
                }
                else
                {
                    const auto selectIndex = document()->thumbnailScene()->thumbnailCount() - 1;
                    selectItemAt(selectIndex, Qt::ControlModifier & event->modifiers());
                    mSelectionSpan = 0;
                }
            }
            break;
        case Qt::Key_A:
            {
                if (Qt::ControlModifier & event->modifiers())
                    selectAll();
            }
            break;
        }
    }
    QGraphicsView::keyPressEvent(event);
}


void UBDocumentThumbnailsView::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    if (0 == selectedItems().count() && document()->thumbnailScene()->thumbnailCount() > 0 && Qt::TabFocusReason == event->reason())
    {
        selectItemAt(0);
        mSelectionSpan = 0;
    }
}


void UBDocumentThumbnailsView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (document())
    {
        auto thumbnailScene = document()->thumbnailScene();

        thumbnailScene->arrangeThumbnails();

        if (thumbnailScene->lastSelectedThumbnail())
        {
            ensureVisible(thumbnailScene->lastSelectedThumbnail());
        }
    }

    emit resized();
}


void UBDocumentThumbnailsView::sceneSelectionChanged()
{
    emit selectionChanged();
}


void UBDocumentThumbnailsView::selectItemAt(int pIndex, bool extend)
{
    const auto thumbnailScene = document()->thumbnailScene();
    thumbnailScene->hightlightItem(pIndex, !extend);
    ensureVisible(thumbnailScene->thumbnailAt(pIndex));
}


void UBDocumentThumbnailsView::selectItems(int startIndex, int count)
{
    auto thumbnailScene = document()->thumbnailScene();

    thumbnailScene->hightlightItem(startIndex, true);

    for (int i = 1; i < count; ++i)
    {
        thumbnailScene->hightlightItem(startIndex + i);
    }
}


void UBDocumentThumbnailsView::selectAll()
{
    auto thumbnailScene = document()->thumbnailScene();

    for (int i = 0; i < thumbnailScene->thumbnailCount(); i++)
    {
        thumbnailScene->hightlightItem(i);
    }
}

int UBDocumentThumbnailsView::rowCount() const
{
    const auto thumbnailScene = document()->thumbnailScene();

    UBThumbnail *lastThumbnail = thumbnailScene->thumbnailAt(thumbnailScene->thumbnailCount() - 1);

    return lastThumbnail ? lastThumbnail->row() + 1 : 0;
}

int UBDocumentThumbnailsView::columnCount() const
{
    const auto thumbnailScene = document()->thumbnailScene();

    UBThumbnail *lastThumbnail = thumbnailScene->thumbnailAt(thumbnailScene->thumbnailCount() - 1);

    if (!lastThumbnail) return 0;
    int lastRow = lastThumbnail->row();
    int lastColumn = lastThumbnail->column();
    return lastRow > 0 ? (thumbnailScene->thumbnailCount() - lastColumn - 1) / lastRow : lastColumn + 1;
}


void UBDocumentThumbnailsView::mouseDoubleClickEvent(QMouseEvent * event)
{
    QGraphicsItem* item = itemAt(event->pos());
    UBThumbnail* thumbnail = dynamic_cast<UBThumbnail*>(item);


    if (thumbnail)
    {
        int index = thumbnail->sceneIndex();
        emit mouseDoubleClick(item, index);
    }
}


bool UBDocumentThumbnailsView::thumbnailLessThan(QGraphicsItem* item1, QGraphicsItem* item2)
{
    UBThumbnail *thumbnail1 = dynamic_cast<UBThumbnail*>(item1);
    UBThumbnail *thumbnail2 = dynamic_cast<UBThumbnail*>(item2);
    if (thumbnail1 && thumbnail2)
    {
        if (thumbnail1->row() != thumbnail2->row())
            return thumbnail1->row() < thumbnail2->row();
        else
            return thumbnail1->column() < thumbnail2->column();
    }
    return false;
}

/* ---- UBDocumentThumbnailArranger ---- */

UBDocumentThumbnailsView::UBDocumentThumbnailArranger::UBDocumentThumbnailArranger(UBDocumentThumbnailsView* thumbnailView)
    : UBThumbnailArranger(thumbnailView)
{
    QObject::connect(thumbnailView, &UBDocumentThumbnailsView::thumbnailWidthChanged,
                       thumbnailView, [this](int width) { setThumbnailWidth(width); });
}

int UBDocumentThumbnailsView::UBDocumentThumbnailArranger::columnCount() const
{
    const auto horizontalSpacing = spacing().width();
    const int columns =
        std::max((availableViewWidth() + horizontalSpacing) / (thumbnailWidth() + horizontalSpacing), 1.);

    return columns;
}

double UBDocumentThumbnailsView::UBDocumentThumbnailArranger::thumbnailWidth() const
{
    return mThumbnailWidth;
}

bool UBDocumentThumbnailsView::UBDocumentThumbnailArranger::isUIEnabled() const
{
    return false;
}

void UBDocumentThumbnailsView::UBDocumentThumbnailArranger::setThumbnailWidth(int width)
{
    mThumbnailWidth = width;
}
