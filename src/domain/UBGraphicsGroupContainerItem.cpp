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




#include "UBGraphicsGroupContainerItem.h"

#include <QtGui>

#include "UBGraphicsMediaItem.h"
#include "UBGraphicsTextItem.h"
#include "domain/UBGraphicsItemDelegate.h"
#include "domain/UBGraphicsGroupContainerItemDelegate.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

UBGraphicsGroupContainerItem::UBGraphicsGroupContainerItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mCurrentItem(NULL)
{
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    setDelegate(new UBGraphicsGroupContainerItemDelegate(this, 0));

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    UBGraphicsGroupContainerItem::setAcceptHoverEvents(true);

    setUuid(QUuid::createUuid());

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
}

UBGraphicsGroupContainerItem::~UBGraphicsGroupContainerItem()
{
}

void UBGraphicsGroupContainerItem::addToGroup(QGraphicsItem *item)
{
    if (!item) {
        qWarning("UBGraphicsGroupContainerItem::addToGroup: cannot add null item");
        return;
    }
    if (item == this) {
        qWarning("UBGraphicsGroupContainerItem::addToGroup: cannot add a group to itself");
        return;
    }

    //Check if group is allready rotatable or flippable
    if (childItems().count()) {
        if (UBGraphicsItem::isFlippable(this) && !UBGraphicsItem::isFlippable(item)) {
            Delegate()->setUBFlag(GF_FLIPPABLE_ALL_AXIS, false);
        }
        if (UBGraphicsItem::isRotatable(this) && !UBGraphicsItem::isRotatable(item)) {
            Delegate()->setUBFlag(GF_REVOLVABLE, false);
        }
        if (!UBGraphicsItem::isLocked(this) && UBGraphicsItem::isLocked(item)) {
            Delegate()->setLocked(true);
        }
    }
    else {
        Delegate()->setUBFlag(GF_FLIPPABLE_ALL_AXIS, UBGraphicsItem::isFlippable(item));
        Delegate()->setUBFlag(GF_REVOLVABLE, UBGraphicsItem::isRotatable(item));
        Delegate()->setLocked(UBGraphicsItem::isLocked(item));
    }        

    if (item->data(UBGraphicsItemData::ItemLayerType) == UBItemLayerType::Control)
        setData(UBGraphicsItemData::ItemLayerType, item->data(UBGraphicsItemData::ItemLayerType));

    // COMBINE
    bool ok;
    QTransform itemTransform = item->itemTransform(this, &ok);

    if (!ok) {
        qWarning("UBGraphicsGroupContainerItem::addToGroup: could not find a valid transformation from item to group coordinates");
        return;
    }

    //setting item flags to given item
    item->setSelected(false);
    item->setFlag(QGraphicsItem::ItemIsSelectable, false);
    item->setFlag( QGraphicsItem::ItemIsMovable, false);
    item->setFlag(QGraphicsItem::ItemIsFocusable, true);

    QTransform newItemTransform(itemTransform);
    item->setPos(mapFromItem(item, 0, 0));

    if (item->scene()) {
        item->scene()->removeItem(item);
    }

    if (corescene())
        corescene()->removeItemFromDeletion(item);
    item->setParentItem(this);

    // removing position from translation component of the new transform
    if (!item->pos().isNull())
        newItemTransform *= QTransform::fromTranslate(-item->x(), -item->y());

    // removing additional transformations properties applied with itemTransform()
    QPointF origin = item->transformOriginPoint();
    QMatrix4x4 m;
    QList<QGraphicsTransform*> transformList = item->transformations();
    for (int i = 0; i < transformList.size(); ++i)
        transformList.at(i)->applyTo(&m);
    newItemTransform *= m.toTransform().inverted();
    newItemTransform.translate(origin.x(), origin.y());
    newItemTransform.rotate(-item->rotation());
    newItemTransform.scale(1/item->scale(), 1/item->scale());
    newItemTransform.translate(-origin.x(), -origin.y());

    // ### Expensive, we could maybe use dirtySceneTransform bit for optimization

    item->setTransform(newItemTransform);
    //    item->d_func()->setIsMemberOfGroup(true);
    prepareGeometryChange();
    itemsBoundingRect |= itemTransform.mapRect(item->boundingRect() | item->childrenBoundingRect());
    update();
}
void UBGraphicsGroupContainerItem::removeFromGroup(QGraphicsItem *item)
{
    if (!item) {
        qDebug() << "can't specify the item because of the null pointer";
        return;
    }

    UBCoreGraphicsScene *groupScene = corescene();
    if (groupScene)
    {
        groupScene->addItemToDeletion(item);
    }

    pRemoveFromGroup(item);

    item->setFlags(ItemIsSelectable | ItemIsFocusable);

}

void UBGraphicsGroupContainerItem::deselectCurrentItem()
{
    if (mCurrentItem && (mCurrentItem->type() == UBGraphicsMediaItem::Type
                         || mCurrentItem->type() == UBGraphicsVideoItem::Type
                         || mCurrentItem->type() == UBGraphicsAudioItem::Type))
    {
        dynamic_cast<UBGraphicsMediaItem*>(mCurrentItem)->Delegate()->getToolBarItem()->hide();

        mCurrentItem->setSelected(false);
        mCurrentItem = NULL;
    }
}

QRectF UBGraphicsGroupContainerItem::boundingRect() const
{
    return itemsBoundingRect;
}

void UBGraphicsGroupContainerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(painter);
    Q_UNUSED(option);

    Delegate()->postpaint(painter, option, widget);
}

UBCoreGraphicsScene *UBGraphicsGroupContainerItem::corescene()
{
    UBCoreGraphicsScene *castScene = dynamic_cast<UBCoreGraphicsScene*>(QGraphicsItem::scene());

    return castScene;
}

UBGraphicsGroupContainerItem *UBGraphicsGroupContainerItem::deepCopyNoChildDuplication() const
{
    UBGraphicsGroupContainerItem *copy = new UBGraphicsGroupContainerItem();

    copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable

    copyItemParameters(copy);

    return copy;
}


UBGraphicsGroupContainerItem *UBGraphicsGroupContainerItem::deepCopy() const
{
    UBGraphicsGroupContainerItem *copy = new UBGraphicsGroupContainerItem();

    copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable

    foreach (QGraphicsItem *it, childItems()) {
        UBItem *childAsUBItem = dynamic_cast<UBItem*>(it);
        if (childAsUBItem) {
            QGraphicsItem *cloneItem = dynamic_cast<QGraphicsItem*>(childAsUBItem->deepCopy());
            copy->addToGroup(cloneItem);
        }
    }
    copyItemParameters(copy);

    return copy;
}



void UBGraphicsGroupContainerItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsGroupContainerItem *cp = dynamic_cast<UBGraphicsGroupContainerItem*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    }
}

void UBGraphicsGroupContainerItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

void UBGraphicsGroupContainerItem::destroy(bool canUndo) {

    foreach (QGraphicsItem *item, childItems()) {
        pRemoveFromGroup(item);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setFlag(QGraphicsItem::ItemIsFocusable, true);
    }

    remove(canUndo);
}

void UBGraphicsGroupContainerItem::clearSource()
{
    foreach(QGraphicsItem *child, childItems())
    {
        UBGraphicsItem *item = dynamic_cast<UBGraphicsItem *>(child);
        if (item)
        {
            item->clearSource();
        }
    }
}

void UBGraphicsGroupContainerItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mousePressEvent(event)) {
        //NOOP
    } else {

    QGraphicsItem::mousePressEvent(event);
        setSelected(true);
    }


}

void UBGraphicsGroupContainerItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mouseMoveEvent(event)) {
        // NOOP;
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }

}

void UBGraphicsGroupContainerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
//    mDelegate->mouseReleaseEvent(event);
    QGraphicsItem::mouseReleaseEvent(event);
}

QVariant UBGraphicsGroupContainerItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = Delegate()->itemChange(change, value);

    foreach(QGraphicsItem *child, childItems())
    {
        UBGraphicsItem *item = dynamic_cast<UBGraphicsItem*>(child);
        if (item)
        {
            item->Delegate()->positionHandles();
        }
    }

    if (QGraphicsItem::ItemSelectedChange == change)
    {
        deselectCurrentItem();
    }

    return QGraphicsItem::itemChange(change, newValue);
}

void UBGraphicsGroupContainerItem::pRemoveFromGroup(QGraphicsItem *item)
{
    if (!item) {
        qWarning("QGraphicsItemGroup::removeFromGroup: cannot remove null item");
        return;
    }

    QGraphicsItem *newParent = parentItem();

    if (childItems().count()) {
        if (!UBGraphicsItem::isFlippable(item) || !UBGraphicsItem::isRotatable(item)) {
            bool flippableNow = true;
            bool rotatableNow = true;
            bool lockedNow = false;

            foreach (QGraphicsItem *item, childItems()) {
                if (!UBGraphicsItem::isFlippable(item)) {
                    flippableNow = false;
                }
                if (!UBGraphicsItem::isRotatable(item)) {
                    rotatableNow = false;
                }
                if(UBGraphicsItem::isLocked(item))
                    lockedNow = true;

                if (!rotatableNow && !flippableNow && lockedNow) {
                    break;
                }

            }
            Delegate()->setUBFlag(GF_FLIPPABLE_ALL_AXIS, flippableNow);
            Delegate()->setUBFlag(GF_REVOLVABLE, rotatableNow);
            Delegate()->setLocked(lockedNow);
        }
    }

    // COMBINE
    bool ok;
    QTransform itemTransform;
    if (newParent)
        itemTransform = item->itemTransform(newParent, &ok);
    else
        itemTransform = item->sceneTransform();

    QPointF oldPos = item->mapToItem(newParent, 0, 0);
    item->setParentItem(newParent);
    item->setPos(oldPos);

    UBGraphicsScene *Scene = dynamic_cast<UBGraphicsScene *>(item->scene());
    if (Scene)
    {
        Scene->addItem(item);
    }

    // removing position from translation component of the new transform
    if (!item->pos().isNull())
        itemTransform *= QTransform::fromTranslate(-item->x(), -item->y());

    // removing additional transformations properties applied
    // with itemTransform() or sceneTransform()
    QPointF origin = item->transformOriginPoint();
    QMatrix4x4 m;
    QList<QGraphicsTransform*> transformList = item->transformations();
    for (int i = 0; i < transformList.size(); ++i)
        transformList.at(i)->applyTo(&m);
    itemTransform *= m.toTransform().inverted();
    itemTransform.translate(origin.x(), origin.y());
    itemTransform.rotate(-item->rotation());
    itemTransform.scale(1 / item->scale(), 1 / item->scale());
    itemTransform.translate(-origin.x(), -origin.y());

    // ### Expensive, we could maybe use dirtySceneTransform bit for optimization

    item->setTransform(itemTransform);
//    item->d_func()->setIsMemberOfGroup(item->group() != 0);

    // ### Quite expensive. But removeFromGroup() isn't called very often.
    prepareGeometryChange();
    itemsBoundingRect = childrenBoundingRect();

    item->setFlag(ItemIsMovable, true);
}
