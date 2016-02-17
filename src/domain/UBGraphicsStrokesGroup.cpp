/*
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
    Delegate()->startUndoStep();

    mStartingPoint = event->scenePos();

    initializeTransform();

    mTranslateX = 0;
    mTranslateY = 0;
    mAngleOffset = 0;

    mInitialTransform = buildTransform();

    event->accept();
}

void UBGraphicsStrokesGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QLineF move = QLineF(mStartingPoint, event->scenePos());

    mTranslateX = move.dx();
    mTranslateY = move.dy();
    //Delegate()->frame()->moveLinkedItems(move);

    setTransform(buildTransform());
    
    event->accept();
     
}

void UBGraphicsStrokesGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Delegate()->commitUndoStep();

    mTotalTranslateX += mTranslateX;
    mTotalTranslateY += mTranslateY;

    event->accept();

    Delegate()->mouseReleaseEvent(event);
    QGraphicsItemGroup::mouseReleaseEvent(event);
}

UBItem* UBGraphicsStrokesGroup::deepCopy() const
{
    QTransform groupTransform = transform();

    UBGraphicsStrokesGroup* copy = new UBGraphicsStrokesGroup();
    copyItemParameters(copy);
    copy->resetTransform();

    const_cast<UBGraphicsStrokesGroup*>(this)->resetTransform();

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
    copy->setTransform(groupTransform);

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

void UBGraphicsStrokesGroup::initializeTransform()
{
    
    QTransform itemTransform = sceneTransform();
    QRectF itemRect = boundingRect();
    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QPointF bottomLeft = itemTransform.map(itemRect.bottomLeft());

    qreal horizontalFlip = (topLeft.x() > topRight.x()) ? -1 : 1;
    mMirrorX = horizontalFlip < 0 ;
    if(horizontalFlip < 0){
        // why this is because of the way of calculating the translations that checks which side is the most is the
        // nearest instead of checking which one is the left side.
        QPointF tmp = topLeft;
        topLeft = topRight;
        topRight = tmp;

        // because of the calculation of the height is done by lenght and not deltaY
        bottomLeft = itemTransform.map(itemRect.bottomRight());
    }

    qreal verticalFlip = (bottomLeft.y() < topLeft.y()) ? -1 : 1;
    // not sure that is usefull
    mMirrorY = verticalFlip < 0;
    if(verticalFlip < 0 && !mMirrorX){
        topLeft = itemTransform.map(itemRect.bottomLeft());
        topRight = itemTransform.map(itemRect.bottomRight());
        bottomLeft = itemTransform.map(itemRect.topLeft());
    }

    QLineF topLine(topLeft, topRight);
    QLineF leftLine(topLeft, bottomLeft);
    qreal width = topLine.length();
    qreal height = leftLine.length();

    mAngle = topLine.angle();

    // the fact the the length is used we loose the horizontalFlip information
    // a better way to do this is using DeltaX that preserve the direction information.
    mTotalScaleX = (width / itemRect.width()) * horizontalFlip;
    mTotalScaleY = height / itemRect.height() * verticalFlip;
    
    

    QTransform tr;
    QPointF center = boundingRect().center();
    tr.translate(center.x() * mTotalScaleX, center.y() * mTotalScaleY);
    tr.rotate(-mAngle);
    tr.translate(-center.x() * mTotalScaleX, -center.y() * mTotalScaleY);
    tr.scale(mTotalScaleX, mTotalScaleY);

    mTotalTranslateX = transform().dx() - tr.dx();
    mTotalTranslateY = transform().dy() - tr.dy();
    
    
}

QTransform UBGraphicsStrokesGroup::buildTransform()
{
    QTransform tr;
    QPointF center = boundingRect().center();

    // Translate
    tr.translate(mTotalTranslateX + mTranslateX, mTotalTranslateY + mTranslateY);

    // Set angle
    tr.translate(center.x() * mTotalScaleX, center.y() * mTotalScaleY);
    tr.rotate(-mAngle);
    tr.translate(-center.x() * mTotalScaleX, -center.y() * mTotalScaleY);

    // Scale
    tr.scale(mTotalScaleX, mTotalScaleY );

    return tr;
}
