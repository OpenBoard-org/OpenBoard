/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBGraphicsStrokesGroup.h"

#include "domain/UBGraphicsPolygonItem.h"

#include "core/memcheck.h"

UBGraphicsStrokesGroup::UBGraphicsStrokesGroup(QGraphicsItem *parent)
    :QGraphicsItemGroup(parent), UBGraphicsItem()
{
    setDelegate(new UBGraphicsItemDelegate(this, 0, GF_COMMON
                                           | GF_RESPECT_RATIO
                                           | GF_REVOLVABLE
                                           | GF_FLIPPABLE_ALL_AXIS));

    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

}

UBGraphicsStrokesGroup::~UBGraphicsStrokesGroup()
{
}

void UBGraphicsStrokesGroup::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}
void UBGraphicsStrokesGroup::setColor(const QColor &color, colorType pColorType)
{
    //TODO Implement common mechanism of managing groups, drop UBGraphicsStroke if it's obsolete
    //Using casting for the moment
    foreach (QGraphicsItem *item, childItems()) {
        if (item->type() == UBGraphicsPolygonItem::Type) {
            UBGraphicsPolygonItem *curPolygon = static_cast<UBGraphicsPolygonItem *>(item);

            switch (pColorType) {
            case currentColor :
                curPolygon->setColor(color);
                break;
            case colorOnLightBackground :
                 curPolygon->setColorOnLightBackground(color);
                break;
            case colorOnDarkBackground :
                 curPolygon->setColorOnDarkBackground(color);
                break;
            }
        }
    }
}

QColor UBGraphicsStrokesGroup::color(colorType pColorType) const
{
    QColor result;

    foreach (QGraphicsItem *item, childItems()) {
        if (item->type() == UBGraphicsPolygonItem::Type) {
            UBGraphicsPolygonItem *curPolygon = static_cast<UBGraphicsPolygonItem *>(item);

            switch (pColorType) {
            case currentColor :
                result = curPolygon->color();
                break;
            case colorOnLightBackground :
                result = curPolygon->colorOnLightBackground();
                break;
            case colorOnDarkBackground :
                result = curPolygon->colorOnDarkBackground();
                break;
            }

        }
    }

    return result;
}

void UBGraphicsStrokesGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mousePressEvent(event))
    {
        //NOOP
    }
    else
    {
//        QGraphicsItemGroup::mousePressEvent(event);
    }
}

void UBGraphicsStrokesGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mouseMoveEvent(event))
    {
        // NOOP;
    }
    else
    {
        QGraphicsItemGroup::mouseMoveEvent(event);
    }
}

void UBGraphicsStrokesGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Delegate()->mouseReleaseEvent(event);
    QGraphicsItemGroup::mouseReleaseEvent(event);
}

UBItem* UBGraphicsStrokesGroup::deepCopy() const
{
    UBGraphicsStrokesGroup* copy = new UBGraphicsStrokesGroup();

    QTransform groupTransform = transform();
    const_cast<UBGraphicsStrokesGroup*>(this)->resetTransform();

    QList<QGraphicsItem*> chl = childItems();

    foreach(QGraphicsItem *child, chl)
    {
        UBGraphicsPolygonItem *polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);

        if (polygon){
            UBGraphicsPolygonItem *polygonCopy = dynamic_cast<UBGraphicsPolygonItem*>(polygon->deepCopy());
            if (polygonCopy)
            {
                QGraphicsItem* pItem = dynamic_cast<QGraphicsItem*>(polygonCopy);
                copy->addToGroup(pItem);
                polygonCopy->setStrokesGroup(copy);
            }
        }

    }
    const_cast<UBGraphicsStrokesGroup*>(this)->setTransform(groupTransform);
    copyItemParameters(copy);

    return copy;
}

void UBGraphicsStrokesGroup::copyItemParameters(UBItem *copy) const
{
    QGraphicsItem *cp = dynamic_cast<QGraphicsItem*>(copy);
    if(NULL != cp)
    {
        cp->setTransform(transform());

        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    }
}

void UBGraphicsStrokesGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    bool selectedState = false;
    if (styleOption.state & QStyle::State_Selected) {
        selectedState = true;
    }
    QStyle::State svState = option->state;
    styleOption.state &= ~QStyle::State_Selected;
    QGraphicsItemGroup::paint(painter, &styleOption, widget);
    //Restoring state
    styleOption.state |= svState;

    Delegate()->postpaint(painter, &styleOption, widget);
}

QVariant UBGraphicsStrokesGroup::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = Delegate()->itemChange(change, value);
    return QGraphicsItemGroup::itemChange(change, newValue);
}

QPainterPath UBGraphicsStrokesGroup::shape() const
{
    QPainterPath path;

    if (isSelected())
    {
        path.addRect(boundingRect());
    }
    else
    {
        foreach(QGraphicsItem* item, childItems())
        {
            path.addPath(item->shape());
        }
    }

    return path;
}
