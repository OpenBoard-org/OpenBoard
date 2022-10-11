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


#include "UBSelectionFrame.h"

#include <QtGui>

#include "domain/UBItem.h"
#include "domain/UBGraphicsItemZLevelUndoCommand.h"
#include "domain/UBGraphicsGroupContainerItem.h"
#include "board/UBBoardController.h"
#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "gui/UBResources.h"
#include "gui/UBMainWindow.h"
#include "core/UBApplication.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"

UBSelectionFrame::UBSelectionFrame()
    : mThickness(UBSettings::settings()->objectFrameWidth)
    , mAntiscaleRatio(1.0)
    , mRotationAngle(0)
    , mDeleteButton(0)
    , mDuplicateButton(0)
    , mZOrderUpButton(0)
    , mZOrderDownButton(0)
    , mGroupButton(0)
    , mRotateButton(0)
{
    setLocalBrush(QBrush(UBSettings::paletteColor));
    setPen(Qt::NoPen);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::SelectionFrame)); //Necessary to set if we want z value to be assigned correctly
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsSelectable | ItemIsMovable);

    connect(UBApplication::boardController, SIGNAL(zoomChanged(qreal)), this, SLOT(onZoomChanged(qreal)));

    onZoomChanged(UBApplication::boardController->currentZoom());
}

void UBSelectionFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPainterPath path;
    QRectF shRect = option->rect;
    path.addRoundedRect(shRect, adjThickness() / 2, adjThickness() / 2);

    if (rect().width() > 1 && rect().height() > 1) {
        QPainterPath extruded;
        extruded.addRect(shRect.adjusted(adjThickness(), adjThickness(), (adjThickness() * -1), (adjThickness() * -1)));
        path = path.subtracted(extruded);
    }

    painter->fillPath(path, mLocalBrush);
}

QRectF UBSelectionFrame::boundingRect() const
{
    return rect().adjusted(-adjThickness(), -adjThickness(), adjThickness(), adjThickness());
}

QPainterPath UBSelectionFrame::shape() const
{
    QPainterPath resShape;
    QRectF ownRect = rect();
    QRectF shRect = ownRect.adjusted(-adjThickness(), -adjThickness(), adjThickness(), adjThickness());
    resShape.addRoundedRect(shRect, adjThickness() / 2, adjThickness() / 2);

    if (rect().width() > 1 && rect().height() > 1) {
        QPainterPath extruded;
        extruded.addRect(ownRect);
        resShape = resShape.subtracted(extruded);
    }

    return resShape;
}

void UBSelectionFrame::setEnclosedItems(const QList<QGraphicsItem*> pGraphicsItems)
{
    mButtons.clear();
    mButtons.append(mDeleteButton);
    mRotationAngle = 0;

    QRegion resultRegion;
    UBGraphicsFlags resultFlags;
    mEnclosedtems.clear();

    // If at least one of the enclosed items is locked, the entire selection is
    // considered to be locked.
    mIsLocked = false;

    foreach (QGraphicsItem *nextItem, pGraphicsItems) {
        UBGraphicsItemDelegate *nextDelegate = UBGraphicsItem::Delegate(nextItem);
        if (nextDelegate) {
            mIsLocked = (mIsLocked || nextDelegate->isLocked());
            mEnclosedtems.append(nextDelegate);
            resultRegion |= nextItem->boundingRegion(nextItem->sceneTransform());
            resultFlags |= nextDelegate->ubflags();
        }
    }

    QRectF resultRect = resultRegion.boundingRect();
    setRect(resultRect);

    mButtons = buttonsForFlags(resultFlags);
    placeButtons();

    if (resultRect.isEmpty()) {
        hide();
    }

    if (mIsLocked) {
        QColor baseColor = UBSettings::paletteColor;
        baseColor.setAlphaF(baseColor.alphaF() / 3);
        setLocalBrush(QBrush(baseColor));
    }
    else
        setLocalBrush(QBrush(UBSettings::paletteColor));
}

void UBSelectionFrame::updateRect()
{
    QRegion resultRegion;
    foreach (UBGraphicsItemDelegate *curDelegateItem, mEnclosedtems) {
        resultRegion |= curDelegateItem->delegated()->boundingRegion(curDelegateItem->delegated()->sceneTransform());
    }

    QRectF result = resultRegion.boundingRect();
    setRect(result);

    placeButtons();

    if (result.isEmpty()) {
        setVisible(false);
    }
}

void UBSelectionFrame::updateScale()
{
    setScale(-UBApplication::boardController->currentZoom());
}

void UBSelectionFrame::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mPressedPos = mLastMovedPos = event->pos();
    mLastTranslateOffset = QPointF();
    mRotationAngle = 0;

    if (scene()->itemAt(event->scenePos(), transform()) == mRotateButton) {
        mOperationMode = om_rotating;
    } else {
        mOperationMode = om_moving;
    }
}

void UBSelectionFrame::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mIsLocked)
        return;

    QPointF dp = event->pos() - mPressedPos;
    QPointF rotCenter = mapToScene(rect().center());

    foreach (UBGraphicsItemDelegate *curDelegate, mEnclosedtems) {

        switch (static_cast<int>(mOperationMode)) {
        case om_moving : {
            QGraphicsItem *item = curDelegate->delegated();
            QTransform ownTransform = item->transform();
            QTransform dTransform(
                        ownTransform.m11()
                        , ownTransform.m12()
                        , ownTransform.m13()

                        , ownTransform.m21()
                        , ownTransform.m22()
                        , ownTransform.m23()

                        , ownTransform.m31() + (dp - mLastTranslateOffset).x()
                        , ownTransform.m32() + (dp - mLastTranslateOffset).y()
                        , ownTransform.m33()
                        );

            item->setTransform(dTransform);
        } break;

        case om_rotating : {
            QLineF startLine(sceneBoundingRect().center(), event->lastScenePos());
            QLineF currentLine(sceneBoundingRect().center(), event->scenePos());
            qreal dAngle = startLine.angleTo(currentLine);

            QGraphicsItem *item = curDelegate->delegated();
            QTransform ownTransform = item->transform();


            QPointF nextRotCenter = item->mapFromScene(rotCenter);

            qreal cntrX = nextRotCenter.x();
            qreal cntrY = nextRotCenter.y();

            ownTransform.translate(cntrX, cntrY);
            mRotationAngle -= dAngle;
            ownTransform.rotate(-dAngle);
            ownTransform.translate(-cntrX, -cntrY);

            item->update();
            item->setTransform(ownTransform, false);

            qDebug() << "curAngle" << mRotationAngle;
        } break;

        }

    }

    updateRect();
    mLastMovedPos = event->pos();
    mLastTranslateOffset = dp;
}

void UBSelectionFrame::mouseReleaseEvent(QGraphicsSceneMouseEvent */*event*/)
{
    mPressedPos = mLastMovedPos = mLastTranslateOffset = QPointF();

    if (mOperationMode == om_moving || mOperationMode == om_rotating) {
        UBApplication::undoStack->beginMacro(UBSettings::undoCommandTransactionName);
        foreach (UBGraphicsItemDelegate *d, mEnclosedtems) {
            d->commitUndoStep();
        }
        UBApplication::undoStack->endMacro();
    }
    mOperationMode = om_idle;

}

void UBSelectionFrame::onZoomChanged(qreal pZoom)
{
    mAntiscaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * pZoom);

}

void UBSelectionFrame::remove()
{
    UBApplication::undoStack->beginMacro(UBSettings::undoCommandTransactionName);
    foreach (UBGraphicsItemDelegate *d, mEnclosedtems) {
        d->remove(true);
    }
    UBApplication::undoStack->endMacro();

    updateRect();
}

static bool sortByZ(UBGraphicsItemDelegate* A, UBGraphicsItemDelegate* B)
{
    return (A->delegated()->data(UBGraphicsItemData::ItemOwnZValue).toReal()
            < B->delegated()->data(UBGraphicsItemData::ItemOwnZValue).toReal() );
}

void UBSelectionFrame::duplicate()
{
    UBApplication::undoStack->beginMacro(UBSettings::undoCommandTransactionName);

    // The mEnclosedtems list items are in order of selection. To avoid losing their
    // relative zValues when duplicating, we re-order the list.
    std::sort(mEnclosedtems.begin(), mEnclosedtems.end(), sortByZ);

    foreach (UBGraphicsItemDelegate *d, mEnclosedtems) {
        d->duplicate();
    }
    UBApplication::undoStack->endMacro();

    updateRect();
}

void UBSelectionFrame::increaseZlevelUp()
{
    QList<QGraphicsItem*> selItems = sortedByZ(scene()->selectedItems());

    QList<QGraphicsItem*>::iterator itemIt = selItems.end();
    while(itemIt != selItems.begin()){
        itemIt--;
        QGraphicsItem* item = *itemIt;
        ubscene()->changeZLevelTo(item, UBZLayerController::up);
    }

    addSelectionUndo(selItems, UBZLayerController::up);
}

void UBSelectionFrame::increaseZlevelTop()
{
    QList<QGraphicsItem*> selItems = sortedByZ(scene()->selectedItems());
    foreach (QGraphicsItem *item, selItems) {
        ubscene()->changeZLevelTo(item, UBZLayerController::top);
    }
    addSelectionUndo(selItems, UBZLayerController::top);
}

void UBSelectionFrame::increaseZlevelDown()
{
    QList<QGraphicsItem*> selItems = sortedByZ(scene()->selectedItems());
    foreach (QGraphicsItem *item, selItems) {
        ubscene()->changeZLevelTo(item, UBZLayerController::down);
    }
    addSelectionUndo(selItems, UBZLayerController::down);
}

void UBSelectionFrame::increaseZlevelBottom()
{
    QList<QGraphicsItem*> selItems = sortedByZ(scene()->selectedItems());
    QListIterator<QGraphicsItem*> iter(selItems);
    iter.toBack();
    while (iter.hasPrevious()) {
        ubscene()->changeZLevelTo(iter.previous(), UBZLayerController::bottom);
    }
    addSelectionUndo(selItems, UBZLayerController::bottom);
}

void UBSelectionFrame::groupItems()
{
    UBGraphicsGroupContainerItem *groupItem = ubscene()->createGroup(enclosedGraphicsItems());

    groupItem->setSelected(true);
    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    qDebug() << "Grouping items";
}

void UBSelectionFrame::addSelectionUndo(QList<QGraphicsItem*> items, UBZLayerController::moveDestination dest){
    if(!items.empty()){
        qreal topItemLevel = items.at(0)->data(UBGraphicsItemData::ItemOwnZValue).toReal();
        UBGraphicsItemZLevelUndoCommand* cmd = new UBGraphicsItemZLevelUndoCommand(ubscene(), items, topItemLevel, dest);
        UBApplication::undoStack->push(cmd);
    }
}

void UBSelectionFrame::translateItem(QGraphicsItem */*item*/, const QPointF &/*translatePoint*/)
{
}

void UBSelectionFrame::placeButtons()
{
    if (!mButtons.count()) {
        return;
    }

    QTransform tr;
    tr.scale(mAntiscaleRatio, mAntiscaleRatio);
    mDeleteButton->setParentItem(this);
    mDeleteButton->setTransform(tr);

    QRectF frRect = boundingRect();

    qreal topX = frRect.left() - mDeleteButton->renderer()->viewBox().width() * mAntiscaleRatio / 2;
    qreal topY = frRect.top() - mDeleteButton->renderer()->viewBox().height() * mAntiscaleRatio / 2;

    qreal bottomX = topX;
    qreal bottomY = frRect.bottom() - mDeleteButton->renderer()->viewBox().height() * mAntiscaleRatio / 2;

    mDeleteButton->setPos(topX, topY);
    mDeleteButton->show();

    int i = 1, j = 0, k = 0;
    while ((i + j + k) < mButtons.size())  {
        DelegateButton* button = mButtons[i + j];

        if (button->getSection() == Qt::TopLeftSection) {
            button->setParentItem(this);
            button->setPos(topX + (i++ * 1.6 * adjThickness()), topY);
            button->setTransform(tr);
        } else if (button->getSection() == Qt::BottomLeftSection) {
            button->setParentItem(this);
            button->setPos(bottomX + (++j * 1.6 * adjThickness()), bottomY);
            button->setTransform(tr);
        } else if (button->getSection() == Qt::NoSection) {
            button->setParentItem(this);
            placeExceptionButton(button, tr);
            k++;
        } else {
            ++k;
        }
            button->show();
    }
}

void UBSelectionFrame::placeExceptionButton(DelegateButton *pButton, QTransform pTransform)
{
    QRectF frRect = boundingRect();

    if (pButton == mRotateButton) {
        qreal topX = frRect.right() - (mRotateButton->renderer()->viewBox().width()) * mAntiscaleRatio - 5;
        qreal topY = frRect.top() + 5;
        mRotateButton->setPos(topX, topY);
        mRotateButton->setTransform(pTransform);
    }
}

void UBSelectionFrame::clearButtons()
{
    foreach (DelegateButton *b, mButtons)
    {
        b->setParentItem(0);
        b->hide();
    }

    mButtons.clear();
}

inline UBGraphicsScene *UBSelectionFrame::ubscene()
{
    return qobject_cast<UBGraphicsScene*>(scene());
}

void UBSelectionFrame::setCursorFromAngle(QString angle)
{
    QWidget *controlViewport = UBApplication::boardController->controlView()->viewport();

    QSize cursorSize(45,30);


    QImage mask_img(cursorSize, QImage::Format_Mono);
    mask_img.fill(0xff);
    QPainter mask_ptr(&mask_img);
    mask_ptr.setBrush( QBrush( QColor(0, 0, 0) ) );
    mask_ptr.drawRoundedRect(0,0, cursorSize.width()-1, cursorSize.height()-1, 6, 6);
    QBitmap bmpMask = QBitmap::fromImage(mask_img);


    QPixmap pixCursor(cursorSize);
    pixCursor.fill(QColor(Qt::white));

    QPainter painter(&pixCursor);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setBrush(QBrush(Qt::white));
    painter.setPen(QPen(QColor(Qt::black)));
    painter.drawRoundedRect(1,1,cursorSize.width()-2,cursorSize.height()-2,6,6);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(1,1,cursorSize.width(),cursorSize.height(), Qt::AlignCenter, angle.append(QChar(176)));
    painter.end();

    pixCursor.setMask(bmpMask);
    controlViewport->setCursor(QCursor(pixCursor));
}

QList<QGraphicsItem*> UBSelectionFrame::sortedByZ(const QList<QGraphicsItem *> &pItems)
{
    //select only items wiht the same z-level as item's one and push it to sortedItems QMultiMap
    // It means: keep only the selected items and remove the selection frame from the list
    QMultiMap<qreal, QGraphicsItem*> sortedItems;
    foreach (QGraphicsItem *tmpItem, pItems) {
        if (tmpItem->type() == Type) {
            continue;
        }
        sortedItems.insert(tmpItem->data(UBGraphicsItemData::ItemOwnZValue).toReal(), tmpItem);
    }

    return sortedItems.values();
}

QList<DelegateButton*> UBSelectionFrame::buttonsForFlags(UBGraphicsFlags fls) {

    QList<DelegateButton*> result;

    if (!mDeleteButton) {
        mDeleteButton = new DelegateButton(":/images/close.svg", this, 0, Qt::TopLeftSection);
        connect(mDeleteButton, SIGNAL(clicked()), this, SLOT(remove()));
    }
    result << mDeleteButton;

    if (fls | GF_DUPLICATION_ENABLED) {
        if (!mDuplicateButton) {
            mDuplicateButton = new DelegateButton(":/images/duplicate.svg", this, 0, Qt::TopLeftSection);
            connect(mDuplicateButton, SIGNAL(clicked(bool)), this, SLOT(duplicate()));
        }
        result <<  mDuplicateButton;
    }

    if (mEnclosedtems.count() >= 1) {
        if (!mGroupButton) {
            mGroupButton = new DelegateButton(":/images/groupItems.svg", this, 0, Qt::TopLeftSection);
            mGroupButton->setShowProgressIndicator(false);
            connect(mGroupButton, SIGNAL(clicked()), this, SLOT(groupItems()));
        }
        result << mGroupButton;
    }

    if (fls | GF_ZORDER_MANIPULATIONS_ALLOWED) {
        if (!mZOrderUpButton) {
            mZOrderUpButton = new DelegateButton(":/images/z_layer_up.svg", this, 0, Qt::BottomLeftSection);
            mZOrderUpButton->setShowProgressIndicator(true);
            connect(mZOrderUpButton, SIGNAL(clicked()), this, SLOT(increaseZlevelUp()));
            connect(mZOrderUpButton, SIGNAL(longClicked()), this, SLOT(increaseZlevelTop()));
        }

        if (!mZOrderDownButton) {
            mZOrderDownButton = new DelegateButton(":/images/z_layer_down.svg", this, 0, Qt::BottomLeftSection);
            mZOrderDownButton->setShowProgressIndicator(true);
            connect(mZOrderDownButton, SIGNAL(clicked()), this, SLOT(increaseZlevelDown()));
            connect(mZOrderDownButton, SIGNAL(longClicked()), this, SLOT(increaseZlevelBottom()));
        }

        result << mZOrderUpButton;
        result << mZOrderDownButton;
    }

    if (fls | GF_REVOLVABLE) {
        if (!mRotateButton) {
            mRotateButton = new DelegateButton(":/images/rotate.svg", this, 0, Qt::NoSection);
            mRotateButton->setCursor(UBResources::resources()->rotateCursor);
            mRotateButton->setShowProgressIndicator(false);
            mRotateButton->setTransparentToMouseEvent(true);
        }
        result << mRotateButton;
    }

    return result;
}

QList<QGraphicsItem*> UBSelectionFrame::enclosedGraphicsItems()
{
    QList<QGraphicsItem*> result;
    foreach (UBGraphicsItemDelegate *d, mEnclosedtems) {
        result << d->delegated();
    }

    return result;
}


