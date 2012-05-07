#include "ubgraphicsgroupcontaineritem.h"

#include <QtGui>

#include "domain/UBGraphicsItemDelegate.h"
#include "domain/ubgraphicsgroupcontaineritemdelegate.h"
#include "domain/UBGraphicsScene.h"

UBGraphicsGroupContainerItem::UBGraphicsGroupContainerItem(QGraphicsItem *parent)
    : QGraphicsItemGroup(parent)
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


UBGraphicsScene *UBGraphicsGroupContainerItem::scene()
{
    UBGraphicsScene *castScene = dynamic_cast<UBGraphicsScene*>(scene());

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

void UBGraphicsGroupContainerItem::destroy() {

    foreach (QGraphicsItem *item, childItems()) {
        removeFromGroup(item);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }

    mDelegate->remove(true);
}

void UBGraphicsGroupContainerItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    qDebug() << "hover move group";
    QGraphicsItemGroup::hoverMoveEvent(event);
}

void UBGraphicsGroupContainerItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mousePressEvent(event)) {
        //NOOP
    } else {
        QGraphicsItemGroup::mousePressEvent(event);
        setSelected(true);
    }
}

void UBGraphicsGroupContainerItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mouseMoveEvent(event)) {
        // NOOP;
    } else {
        QGraphicsItemGroup::mouseMoveEvent(event);
    }
}

void UBGraphicsGroupContainerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mDelegate->mouseReleaseEvent(event);
    QGraphicsItemGroup::mouseReleaseEvent(event);
}

QVariant UBGraphicsGroupContainerItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = mDelegate->itemChange(change, value);
    return QGraphicsItemGroup::itemChange(change, newValue);
}
