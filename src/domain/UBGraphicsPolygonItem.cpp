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




#include "UBGraphicsPolygonItem.h"

#include "frameworks/UBGeometryUtils.h"
#include "UBGraphicsScene.h"
#include "domain/UBGraphicsPolygonItem.h"
#include "domain/UBGraphicsStroke.h"

#include "core/memcheck.h"

UBGraphicsPolygonItem::UBGraphicsPolygonItem (QGraphicsItem * parent)
    : QGraphicsPolygonItem(parent)
    , mHasAlpha(false)
    , mOriginalWidth(-1)
    , mIsNominalLine(false)
    , mStroke(0)
    , mpGroup(NULL)
{
    // NOOP
    initialize();
}


UBGraphicsPolygonItem::UBGraphicsPolygonItem (const QPolygonF & polygon, QGraphicsItem * parent)
    : QGraphicsPolygonItem(polygon, parent)
    , mOriginalWidth(-1)
    , mIsNominalLine(false)
    , mStroke(0)
    , mpGroup(NULL)
{
    // NOOP
    initialize();
}


UBGraphicsPolygonItem::UBGraphicsPolygonItem (const QLineF& pLine, qreal pWidth)
    : QGraphicsPolygonItem(UBGeometryUtils::lineToPolygon(pLine, pWidth))
    , mOriginalLine(pLine)
    , mOriginalWidth(pWidth)
    , mIsNominalLine(true)
    , mStroke(0)
    , mpGroup(NULL)
{
    // NOOP
    initialize();
}

UBGraphicsPolygonItem::UBGraphicsPolygonItem (const QLineF& pLine, qreal pStartWidth, qreal pEndWidth)
    : QGraphicsPolygonItem(UBGeometryUtils::lineToPolygon(pLine, pStartWidth, pEndWidth))
    , mOriginalLine(pLine)
    , mOriginalWidth(pEndWidth)
    , mIsNominalLine(true)
    , mStroke(0)
    , mpGroup(NULL)
{
    // NOOP
    initialize();
}


void UBGraphicsPolygonItem::initialize()
{
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::DrawingItem)); //Necessary to set if we want z value to be assigned correctly
    setUuid(QUuid::createUuid());
}

void UBGraphicsPolygonItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

void UBGraphicsPolygonItem::clearStroke()
{
    if (mStroke!=NULL)
    {
        mStroke->remove(this);
        if (mStroke->polygons().empty())
            delete mStroke;
        mStroke = NULL;
    }
}

UBGraphicsPolygonItem::~UBGraphicsPolygonItem()
{
    clearStroke();
}

void UBGraphicsPolygonItem::setStrokesGroup(UBGraphicsStrokesGroup *group)
{
    mpGroup = group;
}

void UBGraphicsPolygonItem::setStroke(UBGraphicsStroke* stroke)
{
    if (stroke) {
        clearStroke();

        mStroke = stroke;
        mStroke->addPolygon(this);
    }
}

UBGraphicsStroke* UBGraphicsPolygonItem::stroke() const
{
    return mStroke;
}


void UBGraphicsPolygonItem::setColor(const QColor& pColor)
{
    QGraphicsPolygonItem::setBrush(QBrush(pColor));
    setPen(Qt::NoPen);

    mHasAlpha = (pColor.alphaF() < 1.0);
}


QColor UBGraphicsPolygonItem::color() const
{
    return QGraphicsPolygonItem::brush().color();
}


UBItem* UBGraphicsPolygonItem::deepCopy() const
{
    UBGraphicsPolygonItem* copy = new UBGraphicsPolygonItem(polygon(), 0);
    copyItemParameters(copy);
    return copy;
}


void UBGraphicsPolygonItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsPolygonItem *cp = dynamic_cast<UBGraphicsPolygonItem*>(copy);
    if (cp)
    {
        cp->mOriginalLine = this->mOriginalLine;
        cp->mOriginalWidth = this->mOriginalWidth;
        cp->mIsNominalLine = this->mIsNominalLine;

        cp->setTransform(transform());
        cp->setBrush(this->brush());
        cp->setPen(this->pen());
        cp->mHasAlpha = this->mHasAlpha;
        cp->setFillRule(this->fillRule());

        cp->setColorOnDarkBackground(this->colorOnDarkBackground());
        cp->setColorOnLightBackground(this->colorOnLightBackground());

        cp->setZValue(this->zValue());
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    }
}

void UBGraphicsPolygonItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    if(mHasAlpha && scene() && scene()->isLightBackground())
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter->setRenderHints(QPainter::Antialiasing);

    QGraphicsPolygonItem::paint(painter, option, widget);
}

UBGraphicsScene* UBGraphicsPolygonItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsPolygonItem::scene());
}
