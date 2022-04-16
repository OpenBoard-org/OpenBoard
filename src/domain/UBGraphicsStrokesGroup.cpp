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




#include "UBGraphicsStrokesGroup.h"
#include "UBGraphicsStroke.h"

#include "domain/UBGraphicsPolygonItem.h"

#include "core/memcheck.h"

UBGraphicsStrokesGroup::UBGraphicsStrokesGroup(QGraphicsItem *parent)
    : QGraphicsItemGroup(parent)
    , UBGraphicsItem()
    , debugTextEnabled(false) // set to true to get a graphical display of strokes' Z-levels
    , mDebugText(nullptr)
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

    if (mDebugText)
        mDebugText->setBrush(QBrush(color));
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
    Delegate()->startUndoStep();

    QGraphicsItemGroup::mousePressEvent(event);
    event->accept();

    setSelected(false);
}

void UBGraphicsStrokesGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isLocked(this)) {
        QGraphicsItemGroup::mouseMoveEvent(event);

        event->accept();
        setSelected(false);
    }
}

void UBGraphicsStrokesGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Delegate()->commitUndoStep();
    event->accept();

    Delegate()->mouseReleaseEvent(event);
    QGraphicsItemGroup::mouseReleaseEvent(event);
}

UBItem* UBGraphicsStrokesGroup::deepCopy() const
{
    QTransform groupTransform = transform();
    QPointF groupPos = pos();

    UBGraphicsStrokesGroup* copy = new UBGraphicsStrokesGroup();
    copyItemParameters(copy);
    copy->resetTransform();
    copy->setPos(0,0);

    const_cast<UBGraphicsStrokesGroup*>(this)->resetTransform();
    const_cast<UBGraphicsStrokesGroup*>(this)->setPos(0,0);

    QList<QGraphicsItem*> chl = childItems();

    UBGraphicsStroke* newStroke = new UBGraphicsStroke;

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
                polygonCopy->setStroke(newStroke);
            }
        }
    }
    const_cast<UBGraphicsStrokesGroup*>(this)->setTransform(groupTransform);
    const_cast<UBGraphicsStrokesGroup*>(this)->setPos(groupPos);
    copy->setTransform(groupTransform);
    copy->setPos(groupPos);

    return copy;
}

void UBGraphicsStrokesGroup::copyItemParameters(UBItem *copy) const
{
    QGraphicsItem *cp = dynamic_cast<QGraphicsItem*>(copy);
    if(NULL != cp)
    {
        cp->setTransform(transform());
        cp->setPos(pos());

        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setZValue(this->zValue());
    }
}

void UBGraphicsStrokesGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    QStyle::State svState = option->state;
    styleOption.state &= ~QStyle::State_Selected;
    QGraphicsItemGroup::paint(painter, &styleOption, widget);
    //Restoring state
    styleOption.state |= svState;

    Delegate()->postpaint(painter, &styleOption, widget);
}

QVariant UBGraphicsStrokesGroup::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (debugTextEnabled && change == ItemZValueChange) {
        double newZ = qvariant_cast<double>(value);

        UBGraphicsPolygonItem * poly = NULL;
        if (childItems().size() > 2)
            poly = dynamic_cast<UBGraphicsPolygonItem*>(childItems()[1]);

        if (poly) {
            if (!mDebugText) {
                mDebugText = new QGraphicsSimpleTextItem("None", this);
                mDebugText->setPos(poly->boundingRect().topLeft() + QPointF(10, 10));
                mDebugText->setBrush(QBrush(poly->color()));
            }
            mDebugText->setText(QString("Z: %1").arg(newZ));
        }
    }

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
