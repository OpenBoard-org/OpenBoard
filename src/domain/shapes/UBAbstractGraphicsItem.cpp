/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#include "UBAbstractGraphicsItem.h"

#include "domain/UBGraphicsDelegateFrame.h"
#include "domain/UBGraphicsItemDelegate.h"
#include "domain/UBGraphicsScene.h"
#include "domain/shapes/UBShapeFactory.h"


UBAbstractGraphicsItem::UBAbstractGraphicsItem(QGraphicsItem* parent)
    : QAbstractGraphicsShapeItem(parent)
{
    setDelegate(new UBGraphicsItemDelegate(this, nullptr,
                                           {GF_REVOLVABLE, GF_DUPLICATION_ENABLED, GF_ZORDER_MANIPULATIONS_ALLOWED}));
    Delegate()->createControls();

    // used for the podcast
    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    setData(UBGraphicsItemData::itemLayerType,
            QVariant(itemLayerType::ObjectItem)); // Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
}

UBAbstractGraphicsItem::~UBAbstractGraphicsItem()
{
}

void UBAbstractGraphicsItem::applyItemStyle(const UBItemStyle& style, bool isDark)
{
    if (style.lineColor(isDark).isValid())
    {
        setStrokeColor(style.lineColor(isDark));
        mItemStyle.setLineColor(style.lineColor(false), style.lineColor(true));
    }

    if (style.lineWidth() > 0)
    {
        setStrokeWidth(style.lineWidth());
        mItemStyle.setLineWidth(style.lineWidth());
    }

    if (style.lineStyle() != Qt::NoPen)
    {
        setStyle(style.lineStyle());
        mItemStyle.setLineStyle(style.lineStyle());
    }

    if (style.fillColor(isDark).isValid())
    {
        setFillColor(style.fillColor(isDark));
        mItemStyle.setFillColor(style.fillColor(false), style.fillColor(true));
    }
}

bool UBAbstractGraphicsItem::isShape() const
{
    return true;
}

bool UBAbstractGraphicsItem::isMarker() const
{
    return false;
}

void UBAbstractGraphicsItem::setStyle(Qt::PenStyle penStyle)
{
    if (hasStrokeProperty())
    {
        const auto pattern = UBShapeFactory::dashPattern(penStyle);
        QPen p = pen();

        if (pattern.isEmpty())
        {
            p.setStyle(penStyle);
        }
        else
        {
            p.setStyle(Qt::CustomDashLine);
            p.setDashPattern(pattern);
        }

        setPen(p);
    }
}

void UBAbstractGraphicsItem::setFillColor(const QColor& color)
{
    QBrush b = brush();

    if (color != Qt::transparent && color.isValid() && b.style() == Qt::NoBrush)
    {
        b.setStyle(Qt::SolidPattern);
    }

    b.setColor(color);
    setBrush(b);
}

void UBAbstractGraphicsItem::setStrokeColor(const QColor& color)
{
    if (hasStrokeProperty())
    {
        QPen p = pen();
        p.setColor(color);
        setPen(p);
    }
}

void UBAbstractGraphicsItem::setStrokeWidth(double width)
{
    if (hasStrokeProperty())
    {
        QPen p = pen();
        p.setWidthF(width);
        setPen(p);
    }
}

QPainterPath UBAbstractGraphicsItem::shape() const
{
    QPainterPathStroker stroker{pen()};
    stroker.setDashPattern(Qt::SolidLine);
    stroker.setWidth(std::max(stroker.width(), 6.));
    const auto outline = painterPath();

    QPainterPath path = stroker.createStroke(outline);

    if (hasFillingProperty())
    {
        path = path.united(outline);
    }

    addExtraShapes(path);

    return path;
}

QVariant UBAbstractGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    QVariant newValue = value;

    if (Delegate())
        newValue = Delegate()->itemChange(change, value);

    if (change == GraphicsItemChange::ItemSelectedHasChanged)
    {
        scene()->styledItemSelectionChanged(this, value.toBool());
    }

    return QAbstractGraphicsShapeItem::itemChange(change, newValue);
}

void UBAbstractGraphicsItem::setStyle(QPainter* painter)
{
    if (hasStrokeProperty())
    {
        auto currentPen = pen();
        currentPen.setCapStyle(Qt::RoundCap);
        currentPen.setJoinStyle(Qt::RoundJoin);
        painter->setPen(currentPen);
    }

    if (hasFillingProperty())
    {
        painter->setBrush(brush());
    }
    else
    {
        painter->setBrush(QBrush{});
    }
}

void UBAbstractGraphicsItem::initializeStrokeProperty()
{
    QPen p(Qt::SolidLine);
    p.setWidth(1);
    p.setColor(Qt::black);
    setPen(p);
}

void UBAbstractGraphicsItem::initializeFillingProperty()
{
    QBrush b(Qt::SolidPattern);
    b.setColor(Qt::black);
    setBrush(b);
}

QRectF UBAbstractGraphicsItem::adjustBoundingRect(QRectF rect) const
{
    if (hasStrokeProperty())
    {
        int r = pen().width() / 2;

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

bool UBAbstractGraphicsItem::hasFillingProperty() const
{
    return brush() != QBrush{} && brush().color() != Qt::transparent && brush().color() != QColor{};
}

bool UBAbstractGraphicsItem::hasStrokeProperty() const
{
    return true;
}

void UBAbstractGraphicsItem::copyItemParameters(UBItem* copy) const
{
    UBAbstractGraphicsItem* cp = dynamic_cast<UBAbstractGraphicsItem*>(copy);

    if (!cp)
        return;

    cp->prepareGeometryChange();
    cp->setPos(this->pos());
    cp->setTransform(this->transform());
    cp->setFlag(QGraphicsItem::ItemIsMovable, true);
    cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
    cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));

    cp->setBrush(brush());
    cp->setPen(pen());
    cp->setItemStyle(itemStyle());
    cp->setUuid(this->uuid());
    cp->setZValue(this->zValue());
}

std::shared_ptr<UBGraphicsScene> UBAbstractGraphicsItem::scene()
{
    auto scenePtr = dynamic_cast<UBGraphicsScene*>(QGraphicsItem::scene());
    return scenePtr ? scenePtr->shared_from_this() : nullptr;
}
