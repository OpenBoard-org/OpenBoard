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

    mDelegate = new UBGraphicsGroupContainerItemDelegate(this, 0);
    mDelegate->init();

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    UBGraphicsGroupContainerItem::setAcceptHoverEvents(true);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly

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
        qWarning("QGraphicsItemGroup::removeFromGroup: cannot remove null item");
        return;
    }

    QGraphicsItem *newParent = parentItem();

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
}

void UBGraphicsGroupContainerItem::deselectCurrentItem()
{
    if (mCurrentItem)
    {
        switch(mCurrentItem->type())
        {
        case UBGraphicsTextItem::Type:
              {
                  dynamic_cast<UBGraphicsTextItem*>(mCurrentItem)->Delegate()->getToolBarItem()->hide();
              }
              break;
        case UBGraphicsMediaItem::Type:
              {
                  dynamic_cast<UBGraphicsMediaItem*>(mCurrentItem)->Delegate()->getToolBarItem()->hide();
              }
              break;                   

        }
        mCurrentItem->setSelected(false);
        mCurrentItem = NULL;
    }
}

QRectF UBGraphicsGroupContainerItem::boundingRect() const
{
    return itemsBoundingRect;
}
void UBGraphicsGroupContainerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(painter);
    Q_UNUSED(option);

//    we would not use paint smth for the moment
//    if (option->state & QStyle::State_Selected) {
//        painter->setBrush(Qt::NoBrush);
//        QPen tmpPen;
//        qreal tmpPenWidth = 1.0;
//        tmpPen.setWidth(tmpPenWidth);
//        tmpPen.setColor(Qt::lightGray);
//        painter->setPen(tmpPen);
//        painter->drawRect(itemsBoundingRect.adjusted(tmpPenWidth / 2, tmpPenWidth / 2, -tmpPenWidth / 2, -tmpPenWidth / 2));
//    }
}

UBGraphicsScene *UBGraphicsGroupContainerItem::scene()
{
    UBGraphicsScene *castScene = dynamic_cast<UBGraphicsScene*>(QGraphicsItem::scene());

    return castScene;
}
UBGraphicsGroupContainerItem *UBGraphicsGroupContainerItem::deepCopy() const
{

    UBGraphicsGroupContainerItem *copy = new UBGraphicsGroupContainerItem(parentItem());

    copy->setPos(this->pos());
    copy->setTransform(this->transform());
    copy->setFlag(QGraphicsItem::ItemIsMovable, true);
    copy->setFlag(QGraphicsItem::ItemIsSelectable, true);
    copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    copy->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable

//    copy->resize(this->size());

    return copy;
}
void UBGraphicsGroupContainerItem::remove()
{
    if (mDelegate)
        mDelegate->remove();
}

void UBGraphicsGroupContainerItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

void UBGraphicsGroupContainerItem::destroy() {

    foreach (QGraphicsItem *item, childItems()) {
        removeFromGroup(item);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setFlag(QGraphicsItem::ItemIsFocusable, true);
    }

    remove();

    if (scene()) {
        qDebug() << "scene is well casted";
        scene()->removeItem(this);
    }
}

void UBGraphicsGroupContainerItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mousePressEvent(event)) {
        //NOOP
    } else {

    QGraphicsItem::mousePressEvent(event);
        setSelected(true);
    }


}

void UBGraphicsGroupContainerItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mouseMoveEvent(event)) {
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
    QVariant newValue = mDelegate->itemChange(change, value);

    foreach(QGraphicsItem *child, children())
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
