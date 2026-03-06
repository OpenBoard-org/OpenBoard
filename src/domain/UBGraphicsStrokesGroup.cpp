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




#include "UBGraphicsStrokesGroup.h"
#include "UBGraphicsStroke.h"

#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsScene.h"

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

void UBGraphicsStrokesGroup::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Delegate()->startUndoStep();

    QGraphicsItemGroup::mousePressEvent(event);
    event->accept();
}

void UBGraphicsStrokesGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isLocked(this)) {
        QGraphicsItemGroup::mouseMoveEvent(event);

        event->accept();
    }
}

void UBGraphicsStrokesGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
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
    QHash<UBGraphicsStroke*, UBGraphicsStroke*> groupClone;

    foreach (QGraphicsItem* child, chl)
    {
        UBGraphicsPolygonItem* polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);

        if (polygon)
        {
            UBGraphicsPolygonItem* polygonCopy = dynamic_cast<UBGraphicsPolygonItem*>(polygon->deepCopy());

            if (polygonCopy)
            {
                copy->addToGroup(polygonCopy);
                polygonCopy->setStrokesGroup(copy);

                UBGraphicsStroke* stroke = polygon->stroke();

                if (stroke)
                {
                    UBGraphicsStroke* cloneStroke = groupClone.value(stroke);

                    if (!cloneStroke)
                    {
                        cloneStroke = stroke->deepCopy();
                        groupClone.insert(stroke, cloneStroke);
                    }

                    polygonCopy->setStroke(cloneStroke);
                }
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
        cp->setData(UBGraphicsItemData::ItemIsHiddenOnDisplay, this->data(UBGraphicsItemData::ItemIsHiddenOnDisplay));
        cp->setData(UBGraphicsItemData::ItemOwnZValue, this->data(UBGraphicsItemData::ItemOwnZValue));
        cp->setZValue(this->zValue());
    }
}

UBItemStyle UBGraphicsStrokesGroup::itemStyle() const
{
    UBItemStyle style;
    style.setLineStyle(Qt::SolidLine);

    for (const auto child : childItems())
    {
        const auto polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);

        if (polygon)
        {
            if (polygon->stroke() && polygon->stroke()->role() == UBGraphicsStroke::FILL)
            {
                style.setFillColor(polygon->colorOnLightBackground(), polygon->colorOnDarkBackground());
            }
            else
            {
                style.setLineColor(polygon->colorOnLightBackground(), polygon->colorOnDarkBackground());
            }
        }
    }

    return style;
}

void UBGraphicsStrokesGroup::applyItemStyle(const UBItemStyle& style, bool isDark)
{
    for (const auto child : childItems())
    {
        const auto polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);

        if (polygon)
        {
            if (polygon->stroke() && polygon->stroke()->role() == UBGraphicsStroke::FILL)
            {
                if (style.fillColor(isDark).isValid())
                {
                    polygon->setColorOnDarkBackground(style.fillColor(true));
                    polygon->setColorOnLightBackground(style.fillColor(false));
                    polygon->setColor(style.fillColor(isDark));
                }
            }
            else
            {
                if (style.lineColor(isDark).isValid())
                {
                    polygon->setColorOnDarkBackground(style.lineColor(true));
                    polygon->setColorOnLightBackground(style.lineColor(false));
                    polygon->setColor(style.lineColor(isDark));
                }
            }
        }
    }
}

bool UBGraphicsStrokesGroup::isShape() const
{
    for (const auto child : childItems())
    {
        const auto polygon = dynamic_cast<UBGraphicsPolygonItem*>(child);

        if (polygon && polygon->stroke())
        {
            const auto role = polygon->stroke()->role();

            if (role == UBGraphicsStroke::OUTLINE || role == UBGraphicsStroke::FILL)
            {
                return true;
            }
        }
    }

    return false;
}

bool UBGraphicsStrokesGroup::isMarker() const
{
    const auto strokes = childItems();

    if (!strokes.isEmpty())
    {
        const auto polygon = dynamic_cast<UBGraphicsPolygonItem*>(strokes.first());

        if (polygon && polygon->stroke())
        {
            return polygon->stroke()->role() == UBGraphicsStroke::MARKER;
        }
    }

    return false;
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

    if (change == GraphicsItemChange::ItemSelectedHasChanged)
    {
        auto ubScene = dynamic_cast<UBGraphicsScene*>(QGraphicsItem::scene());
        ubScene->styledItemSelectionChanged(this, value.toBool());
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
