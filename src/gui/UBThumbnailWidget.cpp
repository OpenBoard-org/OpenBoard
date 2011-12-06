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
#include <QString>
#include <QCursor>

#include "UBThumbnailWidget.h"
#include "UBRubberBand.h"

#include "core/UBSettings.h"
#include "core/UBApplication.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "core/memcheck.h"

UBThumbnailWidget::UBThumbnailWidget(QWidget* parent)
    : QGraphicsView(parent)
    , mThumbnailWidth(UBSettings::defaultThumbnailWidth)
    , mSpacing(UBSettings::thumbnailSpacing)
    , mLastSelectedThumbnail(0)
    , mSelectionSpan(0)
    , mLassoRectItem(0)
{
    // By default, the drag is possible
    bCanDrag = true;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    setFrameShape(QFrame::NoFrame);
    setScene(&mThumbnailsScene);

    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    connect(&mThumbnailsScene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));
}


UBThumbnailWidget::~UBThumbnailWidget()
{
    disconnect(&mThumbnailsScene, SIGNAL(selectionChanged()));
}


void UBThumbnailWidget::setThumbnailWidth(qreal pThumbnailWidth)
{
    mThumbnailWidth = pThumbnailWidth;

    refreshScene();
}


void UBThumbnailWidget::setSpacing(qreal pSpacing)
{
    mSpacing = pSpacing;

    refreshScene();
}


void UBThumbnailWidget::setGraphicsItems(const QList<QGraphicsItem*>& pGraphicsItems
        , const QList<QUrl>& pItemsPaths
        , const QStringList pLabels
        , const QString& pMimeType)
{
    mGraphicItems = pGraphicsItems;
    mItemsPaths = pItemsPaths;
    mMimeType = pMimeType;
    mLabels = pLabels;

    foreach(QGraphicsItem* it, mThumbnailsScene.items())
    {
        mThumbnailsScene.removeItem(it, true);
    }

    // set lasso to 0 as it has been cleared as well
    mLassoRectItem = 0;

    foreach (QGraphicsItem* item, pGraphicsItems)
    {
        if (item->scene() != &mThumbnailsScene)
        {
            mThumbnailsScene.addItem(item);
        }
    }

    mLabelsItems.clear();

    foreach (const QString label, pLabels)
    {
        QFontMetrics fm(font());
        UBThumbnailTextItem *labelItem =
            new UBThumbnailTextItem(label); // deleted while replace or by the scene destruction

        mThumbnailsScene.addItem(labelItem);
        mLabelsItems << labelItem;
    }

    refreshScene();

    mLastSelectedThumbnail = 0;
}


void UBThumbnailWidget::refreshScene()
{
    int nbColumns = (geometry().width() - mSpacing) / (mThumbnailWidth + mSpacing);

    int labelSpacing = 0;

    if (mLabelsItems.size() > 0)
    {
        QFontMetrics fm(mLabelsItems.at(0)->font());
        labelSpacing = UBSettings::thumbnailSpacing + fm.height();  // TODO UB 4.x where is 20 from ??? configure ?? compute based on mSpacing ?? JBA Is it the font height ?
    }
    nbColumns = qMax(nbColumns, 1);

    qreal thumbnailHeight = mThumbnailWidth / UBSettings::minScreenRatio;

    for (int i = 0; i < mGraphicItems.size(); i++)
    {
        QGraphicsItem* item = mGraphicItems.at(i);

        qreal scaleWidth = mThumbnailWidth / item->boundingRect().width();
        qreal scaleHeight = thumbnailHeight / item->boundingRect().height();

        qreal scaleFactor = qMin(scaleWidth, scaleHeight);

        //bitmap should not be stretched
        UBThumbnail* pix = dynamic_cast<UBThumbnail*>(item);
        if (pix)
            scaleFactor = qMin(scaleFactor, 1.0);

        QTransform transform;
        transform.scale(scaleFactor, scaleFactor);

        item->setTransform(transform);

        item->setFlag(QGraphicsItem::ItemIsSelectable, true);

        int columnIndex = i % nbColumns;
        int rowIndex = i / nbColumns;

        if (pix)
        {
            pix->setColumn(columnIndex);
            pix->setRow(rowIndex);
        }

        int w = item->boundingRect().width();
        int h = item->boundingRect().height();
        QPointF pos(
                mSpacing + (mThumbnailWidth - w * scaleFactor) / 2 + columnIndex * (mThumbnailWidth + mSpacing),
                mSpacing + rowIndex * (thumbnailHeight + mSpacing + labelSpacing) + (thumbnailHeight - h * scaleFactor) / 2);

        item->setPos(pos);

        if (mLabelsItems.size() > i)
        {
            QFontMetrics fm(mLabelsItems.at(i)->font(), this);
            QString elidedText = fm.elidedText(mLabels.at(i), Qt::ElideRight, mThumbnailWidth);

            mLabelsItems.at(i)->setPlainText(elidedText);
            mLabelsItems.at(i)->setWidth(fm.width(elidedText) + 2 * mLabelsItems.at(i)->document()->documentMargin());

            pos.setY(pos.y() + (thumbnailHeight + h * scaleFactor) / 2 + 5);
            qreal labelWidth = fm.width(elidedText);
            pos.setX(mSpacing + (mThumbnailWidth - labelWidth) / 2 + columnIndex * (mThumbnailWidth + mSpacing));
            mLabelsItems.at(i)->setPos(pos);
        }
    }

    QScrollBar *vertScrollBar = verticalScrollBar();
    int scrollBarThickness = 0;
    if (vertScrollBar && vertScrollBar->isVisible())
        scrollBarThickness = vertScrollBar->width();

    setSceneRect(0, 0,
            geometry().width() - scrollBarThickness,
            mSpacing + ((((mGraphicItems.size() - 1) / nbColumns) + 1) * (thumbnailHeight + mSpacing + labelSpacing)));
}


QList<QGraphicsItem*> UBThumbnailWidget::selectedItems()
{
    QList<QGraphicsItem*> sortedSelectedItems = mThumbnailsScene.selectedItems();
    qSort(sortedSelectedItems.begin(), sortedSelectedItems.end(), thumbnailLessThan);
    return sortedSelectedItems;
}


void UBThumbnailWidget::mousePressEvent(QMouseEvent *event)
{
    mClickTime = QTime::currentTime();
    mMousePressPos = event->pos();
    mMousePressScenePos = mapToScene(mMousePressPos);
    QGraphicsItem* underlyingItem = itemAt(mMousePressPos);

    UBThumbnail *previousSelectedThumbnail = mLastSelectedThumbnail;

    if (!dynamic_cast<UBThumbnail*>(underlyingItem))
    {
        deleteLasso();

        UBRubberBand rubberBand(QRubberBand::Rectangle);
        QStyleOption option;
        option.initFrom(&rubberBand);

        mLassoRectItem = new QGraphicsRectItem(0, scene());

#ifdef Q_WS_MAC
        // The following code must stay in synch with <Qt installation folder>\src\gui\styles\qmacstyle_mac.mm
        QColor strokeColor;
        strokeColor.setHsvF(0, 0, 0.86, 1.0);
        mLassoRectItem->setPen(QPen(strokeColor));
        QColor fillColor(option.palette.color(QPalette::Disabled, QPalette::Highlight));
        fillColor.setHsvF(0, 0, 0.53, 0.25);
        mLassoRectItem->setBrush(fillColor);
#else
        // The following code must stay in synch with <Qt installation folder>\src\gui\styles\qwindowsxpstyle.cpp
        QColor highlight = option.palette.color(QPalette::Active, QPalette::Highlight);
        mLassoRectItem->setPen(highlight.darker(120));
        QColor dimHighlight(qMin(highlight.red() / 2 + 110, 255),
                            qMin(highlight.green() / 2 + 110, 255),
                            qMin(highlight.blue() / 2 + 110, 255),
                            127);
        mLassoRectItem->setBrush(dimHighlight);
#endif

        mLassoRectItem->setZValue(10000);
        mLassoRectItem->setRect(QRectF(mMousePressScenePos, QSizeF()));

        if (Qt::ControlModifier & event->modifiers() || Qt::ShiftModifier & event->modifiers())
        {
            mSelectedThumbnailItems = selectedItems();
            return;
        }

        mSelectedThumbnailItems.clear();
        QGraphicsView::mousePressEvent(event);
    }
    else if (Qt::ShiftModifier & event->modifiers())
    {
        if (previousSelectedThumbnail)
        {
            QGraphicsItem* previousSelectedItem = dynamic_cast<QGraphicsItem*>(previousSelectedThumbnail);
            if (previousSelectedItem)
            {
                int index1 = mGraphicItems.indexOf(previousSelectedItem);
                int index2 = mGraphicItems.indexOf(underlyingItem);
                if (-1 == index2)
                {
                    mSelectedThumbnailItems = selectedItems();
                    return;
                }
                mSelectionSpan = index2 - index1;
                selectItems(qMin(index1, index2), mSelectionSpan < 0 ? - mSelectionSpan + 1 : mSelectionSpan + 1);
                return;
            }
        }
    }
    else
    {
        mLastSelectedThumbnail = dynamic_cast<UBThumbnail*>(underlyingItem);
        if (!underlyingItem->isSelected())
        {
            int index = mGraphicItems.indexOf(underlyingItem);
            selectItemAt(index, Qt::ControlModifier & event->modifiers());
        }
        else
        {
            QGraphicsView::mousePressEvent(event);
        }
        if (!mLastSelectedThumbnail && mGraphicItems.count() > 0)
            mLastSelectedThumbnail = dynamic_cast<UBThumbnail*>(mGraphicItems.at(0));
        mSelectionSpan = 0;
        return;
    }
}


void UBThumbnailWidget::mouseMoveEvent(QMouseEvent *event)
{
    int distance = (mMousePressPos - event->pos()).manhattanLength();

    if (0 == (event->buttons() & Qt::LeftButton) || distance < QApplication::startDragDistance())
        return;

    if (mLassoRectItem)
    {
        bSelectionInProgress = true;
        QPointF currentScenePos = mapToScene(event->pos());
        QRectF lassoRect(
            qMin(mMousePressScenePos.x(), currentScenePos.x()), qMin(mMousePressScenePos.y(), currentScenePos.y()),
            qAbs(mMousePressScenePos.x() - currentScenePos.x()), qAbs(mMousePressScenePos.y() - currentScenePos.y()));
        mLassoRectItem->setRect(lassoRect);

        QList<QGraphicsItem*> lassoSelectedItems = scene()->items(lassoRect, Qt::IntersectsItemBoundingRect);
        QList<QGraphicsItem*> lassoSelectedThumbnailItems;
        foreach (QGraphicsItem *lassoSelectedItem, lassoSelectedItems)
        {
            UBThumbnail *thumbnailItem = dynamic_cast<UBThumbnail*>(lassoSelectedItem);
            if (thumbnailItem)
                lassoSelectedThumbnailItems.append(lassoSelectedItems);
        }

        unselectAll();
        foreach (QGraphicsItem *lassoSelectedItem, lassoSelectedThumbnailItems)
        {
            if (Qt::ControlModifier & event->modifiers())
            {
                if (!mSelectedThumbnailItems.contains(lassoSelectedItem))
                    selectItemAt(mGraphicItems.indexOf(lassoSelectedItem), true);
            }
            else
            {
                selectItemAt(mGraphicItems.indexOf(lassoSelectedItem), true);
            }
        }
        if (Qt::ControlModifier & event->modifiers())
        {
            foreach (QGraphicsItem *selectedItem, mSelectedThumbnailItems)
            {
                if (!lassoSelectedThumbnailItems.contains(selectedItem))
                    selectItemAt(mGraphicItems.indexOf(selectedItem), true);
            }
        }
    }
    else
    {
        bSelectionInProgress = false;
        if (0 == selectedItems().size())
            return;

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
                if (mGraphicItems.contains(item))
                {
                    if (mGraphicItems.indexOf(item) <= mItemsPaths.size()){
                        qlElements << mItemsPaths.at(mGraphicItems.indexOf(item));
                    }
                }
            }

            if (qlElements.size() > 0){
                            mime->setUrls(qlElements);
                            drag->setMimeData(mime);
                            drag->exec(Qt::CopyAction);
            }
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}


void UBThumbnailWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int elapsedTimeSincePress = mClickTime.elapsed();
    deleteLasso();
    QGraphicsView::mouseReleaseEvent(event);

    if(elapsedTimeSincePress < STARTDRAGTIME)
    {
        if(!bSelectionInProgress)
            emit mouseClick(itemAt(event->pos()), 0);
        bSelectionInProgress = false;
    }
}


void UBThumbnailWidget::keyPressEvent(QKeyEvent *event)
{
    if (mLastSelectedThumbnail)
    {
        QGraphicsItem *lastSelectedGraphicsItem = dynamic_cast<QGraphicsItem*>(mLastSelectedThumbnail);
        if (!lastSelectedGraphicsItem) return;
        int startSelectionIndex = mGraphicItems.indexOf(lastSelectedGraphicsItem);
        int previousSelectedThumbnailIndex = startSelectionIndex + mSelectionSpan;

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
                        if (endSelectionIndex >= mGraphicItems.count()) break;
                    }
                    else
                    {
                        endSelectionIndex = previousSelectedThumbnailIndex - columnCount();
                        if (endSelectionIndex < 0) break;
                    }

                    int startIndex = startSelectionIndex < endSelectionIndex ? startSelectionIndex : endSelectionIndex;
                    int count = startSelectionIndex < endSelectionIndex ? endSelectionIndex - startSelectionIndex + 1 : startSelectionIndex - endSelectionIndex + 1;
                    mSelectionSpan = startSelectionIndex < endSelectionIndex ? (count - 1) : - (count - 1);
                    selectItems(startIndex, count);
                }
                else
                {
                    int toSelectIndex;
                    if (Qt::Key_Down == event->key())
                    {
                        toSelectIndex = previousSelectedThumbnailIndex + columnCount();
                        if (toSelectIndex >= mGraphicItems.count()) break;
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
                QGraphicsItem *previousSelectedItem = mGraphicItems.at(previousSelectedThumbnailIndex);
                UBThumbnail *previousSelectedThumbnail = dynamic_cast<UBThumbnail*>(previousSelectedItem);
                if (!previousSelectedThumbnail) break;

                if (Qt::Key_Left == event->key())
                {
                    if (0 == previousSelectedThumbnail->column()) break;
                }
                else
                {
                    if (previousSelectedThumbnail->column() == columnCount() - 1 ||
                        previousSelectedThumbnailIndex == mGraphicItems.count() - 1) break;
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
                        if (endSelectionIndex >= mGraphicItems.count()) break;
                    }

                    int startIndex = startSelectionIndex < endSelectionIndex ? startSelectionIndex : endSelectionIndex;
                    int count = startSelectionIndex < endSelectionIndex ? endSelectionIndex - startSelectionIndex + 1 : startSelectionIndex - endSelectionIndex + 1;
                    mSelectionSpan = startSelectionIndex < endSelectionIndex ? (count - 1) : - (count - 1);
                    selectItems(startIndex, count);
                }
                else
                {
                    if (Qt::Key_Left == event->key())
                        selectItemAt(previousSelectedThumbnailIndex - 1, Qt::ControlModifier & event->modifiers());
                    else
                        selectItemAt(previousSelectedThumbnailIndex + 1, Qt::ControlModifier & event->modifiers());

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
                    mSelectionSpan = mGraphicItems.count() - startSelectionIndex - 1;
                    selectItems(startSelectionIndex, mSelectionSpan + 1);
                }
                else
                {
                    selectItemAt(mGraphicItems.count() - 1, Qt::ControlModifier & event->modifiers());
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


void UBThumbnailWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    if (0 == selectedItems().count() && mGraphicItems.count() > 0 && Qt::TabFocusReason == event->reason())
    {
        selectItemAt(0);
        mSelectionSpan = 0;
    }
}


void UBThumbnailWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    refreshScene();

    emit resized();
}


void UBThumbnailWidget::sceneSelectionChanged()
{
    emit selectionChanged();
}


void UBThumbnailWidget::selectItemAt(int pIndex, bool extend)
{
    QGraphicsItem* itemToSelect = 0;

    if (pIndex >= 0 && pIndex < mGraphicItems.size())
        itemToSelect = mGraphicItems.at(pIndex);

    foreach (QGraphicsItem* item, items())
    {
        if (item == itemToSelect)
        {
            mLastSelectedThumbnail = dynamic_cast<UBThumbnail*>(item);
            item->setSelected(true);
            ensureVisible(item);
        }
        else if (!extend)
        {
            item->setSelected(false);
        }
    }
}

void UBThumbnailWidget::unselectItemAt(int pIndex)
{
    if (pIndex >= 0 && pIndex < mGraphicItems.size())
    {
        QGraphicsItem *itemToUnselect = mGraphicItems.at(pIndex);
        itemToUnselect->setSelected(false);
    }
}


void UBThumbnailWidget::selectItems(int startIndex, int count)
{
    for (int i = 0; i < mGraphicItems.count(); i++)
    {
        mGraphicItems.at(i)->setSelected(i >= startIndex && i < startIndex + count);
    }
}


void UBThumbnailWidget::selectAll()
{
    foreach (QGraphicsItem* item, mGraphicItems)
    {
        item->setSelected(true);
    }
}

void UBThumbnailWidget::unselectAll()
{
    foreach (QGraphicsItem* item, mGraphicItems)
    {
        item->setSelected(false);
    }
}

int UBThumbnailWidget::rowCount() const
{
    UBThumbnail *lastThumbnail = dynamic_cast<UBThumbnail*>(mGraphicItems.last());
    return lastThumbnail ? lastThumbnail->row() + 1 : 0;
}

int UBThumbnailWidget::columnCount() const
{
    UBThumbnail *lastThumbnail = dynamic_cast<UBThumbnail*>(mGraphicItems.last());
    if (!lastThumbnail) return 0;
    int lastRow = lastThumbnail->row();
    int lastColumn = lastThumbnail->column();
    return lastRow > 0 ? (mGraphicItems.count() - lastColumn - 1) / lastRow : mGraphicItems.count();
}


void UBThumbnailWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    QGraphicsItem* item = itemAt(event->pos());

    if (item)
    {
        int index = mGraphicItems.indexOf(item);
        emit mouseDoubleClick(item, index);
    }
}


bool UBThumbnailWidget::thumbnailLessThan(QGraphicsItem* item1, QGraphicsItem* item2)
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

void UBThumbnailWidget::deleteLasso()
{
    if (mLassoRectItem && scene())
    {
        scene()->removeItem(mLassoRectItem);
        delete mLassoRectItem;
        mLassoRectItem = 0;
    }
}


UBThumbnail::UBThumbnail()
    : mAddedToScene(false)
{
    mSelectionItem = new QGraphicsRectItem(0, 0, 0, 0);
    mSelectionItem->setPen(QPen(UBSettings::treeViewBackgroundColor, 8));
    // TODO UB 4.x fix nasty dependencies : 8 is a bit less than half of UBThumbnailWidget.mSpacing
}

UBThumbnail::~UBThumbnail()
{
    if (mSelectionItem && !mAddedToScene)
        delete mSelectionItem;
}


UBSceneThumbnailNavigPixmap::UBSceneThumbnailNavigPixmap(const QPixmap& pix, UBDocumentProxy* proxy, int pSceneIndex)
    : UBSceneThumbnailPixmap(pix, proxy, pSceneIndex)
    , bButtonsVisible(false)
    , bCanDelete(false)
    , bCanMoveUp(false)
    , bCanMoveDown(false)
{
    setAcceptsHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

UBSceneThumbnailNavigPixmap::~UBSceneThumbnailNavigPixmap()
{

}

void UBSceneThumbnailNavigPixmap::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    event->accept();
    updateButtonsState();
    update();
}

void UBSceneThumbnailNavigPixmap::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    event->accept();
    bButtonsVisible = false;
    update();
}

void UBSceneThumbnailNavigPixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    UBSceneThumbnailPixmap::paint(painter, option, widget);
    if(bButtonsVisible)
    {
        if(bCanDelete)
            painter->drawPixmap(0, 0, BUTTONSIZE, BUTTONSIZE, QPixmap(":images/close.svg"));
        else
            painter->drawPixmap(0, 0, BUTTONSIZE, BUTTONSIZE, QPixmap(":images/closeDisabled.svg"));
        if(bCanMoveUp)
            painter->drawPixmap(BUTTONSIZE + BUTTONSPACING, 0, BUTTONSIZE, BUTTONSIZE, QPixmap(":images/moveUp.svg"));
        else
            painter->drawPixmap(BUTTONSIZE + BUTTONSPACING, 0, BUTTONSIZE, BUTTONSIZE, QPixmap(":images/moveUpDisabled.svg"));
        if(bCanMoveDown)
            painter->drawPixmap(2*(BUTTONSIZE + BUTTONSPACING), 0, BUTTONSIZE, BUTTONSIZE, QPixmap(":images/menu.svg"));
        else
            painter->drawPixmap(2*(BUTTONSIZE + BUTTONSPACING), 0, BUTTONSIZE, BUTTONSIZE, QPixmap(":images/menuDisabled.svg"));
    }
}

void UBSceneThumbnailNavigPixmap::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // INFO: This implementation should work but this method is not called on a mousePressEvent, why?
    //       PLEASE DO NOT REMOVE THIS METHOD! We should reactivate this code when we will fix
    //       the mousePressEvent-not-called issue!

//    QPointF p = event->pos();

//    // Here we check the position of the click and verify if it has to trig an action or not.
//    if(bCanDelete && p.x() >= 0 && p.x() <= BUTTONSIZE && p.y() >= 0 && p.y() <= BUTTONSIZE)
//    {
//        deletePage();
//    }
//    if(bCanMoveUp && p.x() >= BUTTONSIZE + BUTTONSPACING && p.x() <= 2*BUTTONSIZE + BUTTONSPACING && p.y() >= 0 && p.y() <= BUTTONSIZE)
//    {
//        moveUpPage();
//    }
//    if(bCanMoveDown && p.x() >= 2*(BUTTONSIZE + BUTTONSPACING) && p.x() <= 2*(BUTTONSIZE + BUTTONSPACING) + BUTTONSIZE && p.y() >= 0 && p.y() <= BUTTONSIZE)
//    {
//        moveDownPage();
//    }
    event->accept();
}

void UBSceneThumbnailNavigPixmap::updateButtonsState()
{
    bCanDelete = false;
    bCanMoveUp = false;
    bCanMoveDown = false;

    UBDocumentProxy* p = proxy();
    if(NULL != p)
    {
        int iNbPages = p->pageCount();
        if(1 < iNbPages)
        {
            bCanDelete = true;
            if(sceneIndex() > 0)
            {
                bCanMoveUp = true;
            }
            if(sceneIndex() != iNbPages - 1)
            {
                bCanMoveDown = true;
            }
        }
    }

    if(bCanDelete || bCanMoveUp || bCanMoveDown)
    {
        bButtonsVisible = true;
    }
}

void UBSceneThumbnailNavigPixmap::deletePage()
{
    QList<QGraphicsItem*> itemsToDelete;
    itemsToDelete << this;

    UBApplication::documentController->deletePages(itemsToDelete);
}

void UBSceneThumbnailNavigPixmap::moveUpPage()
{
    UBApplication::documentController->moveSceneToIndex(proxy(), sceneIndex(), sceneIndex() - 1);
}

void UBSceneThumbnailNavigPixmap::moveDownPage()
{
    UBApplication::documentController->moveSceneToIndex(proxy(), sceneIndex(), sceneIndex() + 1);
}

void UBSceneThumbnailNavigPixmap::notifyClick(QPointF clickedScenePos)
{
    QPointF p = clickedPos(clickedScenePos);

    // Here we check the position of the click and verify if it has to trig an action or not.
    if(bCanDelete && p.x() >= 0 && p.x() <= BUTTONSIZE/2 && p.y() >= 0 && p.y() <= BUTTONSIZE/2)
    {
        deletePage();
    }
    if(bCanMoveUp && p.x() >= (BUTTONSIZE + BUTTONSPACING)/2 && p.x() <= BUTTONSIZE + BUTTONSPACING && p.y() >= 0 && p.y() <= BUTTONSIZE/2)
    {
        moveUpPage();
    }
    if(bCanMoveDown && p.x() >= BUTTONSIZE + BUTTONSPACING && p.x() <= BUTTONSIZE + BUTTONSPACING + BUTTONSIZE/2 && p.y() >= 0 && p.y() <= BUTTONSIZE/2)
    {
        moveDownPage();
    }
}

QPointF UBSceneThumbnailNavigPixmap::clickedPos(QPointF clickedScenePos)
{
    QPointF p;

    p.setX(clickedScenePos.x() - scenePos().x());
    p.setY(clickedScenePos.y() - scenePos().y());

    return p;
}
