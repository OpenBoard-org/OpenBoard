/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
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
    clearStroke();

    mStroke = stroke;
    mStroke->addPolygon(this);
}

UBGraphicsStroke* UBGraphicsPolygonItem::stroke() const
{
    return mStroke;
}


void UBGraphicsPolygonItem::setColor(const QColor& pColor)
{
    QGraphicsPolygonItem::setBrush(QBrush(pColor));

    if (pColor.alphaF() >= 1.0)
    {
        mHasAlpha = false;
        setPen(Qt::NoPen);
    }
    else
    {
        mHasAlpha = true;

        QColor penColor = pColor;

        // trick QT antialiasing
        // TODO UB 4.x see if we can do better ... it does not behave well with 16 bit color depth
        qreal trickAlpha = pColor.alphaF();

        if (trickAlpha >= 0.2 && trickAlpha < 0.6)
        {
            trickAlpha /= 12;
        }
        else if (trickAlpha < 0.8)
        {
            trickAlpha /= 5;
        }
        else if (trickAlpha < 1.0)
        {
            trickAlpha /= 2;
        }

        penColor.setAlphaF(trickAlpha);
        QGraphicsPolygonItem::setPen(QPen(penColor));
    }
}


QColor UBGraphicsPolygonItem::color() const
{
    return QGraphicsPolygonItem::brush().color();
}


UBItem* UBGraphicsPolygonItem::deepCopy() const
{  
    UBGraphicsPolygonItem* copy = new UBGraphicsPolygonItem(polygon(), 0);

    UBGraphicsStroke *stroke = new UBGraphicsStroke();
    
    copyItemParameters(copy);

    copy->setStroke(stroke);

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

        cp->setColorOnDarkBackground(this->colorOnDarkBackground());
        cp->setColorOnLightBackground(this->colorOnLightBackground());
        //cp->setTransform(transform());

        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    }
}

void UBGraphicsPolygonItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    if(mHasAlpha && scene() && scene()->isLightBackground())
    {
        painter->setCompositionMode(QPainter::CompositionMode_Darken);
    }

    QGraphicsPolygonItem::paint(painter, option, widget);
}

QPainterPath UBGraphicsPolygonItem::shape() const
{

    QPainterPath path;
    path.addRect(boundingRect());

    return path;

//    static QPainterPath shapePath = QGraphicsPolygonItem::shape();

//    return shapePath;
}


UBGraphicsScene* UBGraphicsPolygonItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsPolygonItem::scene());
}
