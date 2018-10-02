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




#include "UBGraphicsCurtainItem.h"

#include <QtGui>

#include "domain/UBGraphicsScene.h"

#include "UBGraphicsCurtainItemDelegate.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h" // TODO UB 4.x clean this dependency
#include "core/UBSettings.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "core/memcheck.h"

const QColor                        UBGraphicsCurtainItem::sDrawColor = Qt::white;
const QColor          UBGraphicsCurtainItem::sDarkBackgroundDrawColor = Qt::black;
const QColor               UBGraphicsCurtainItem::sOpaqueControlColor = QColor(191,191,191,255);
const QColor UBGraphicsCurtainItem::sDarkBackgroundOpaqueControlColor = QColor(63,63,63,255);

UBGraphicsCurtainItem::UBGraphicsCurtainItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    UBGraphicsCurtainItemDelegate* delegate = new UBGraphicsCurtainItemDelegate(this, 0);
    delegate->init();
    setDelegate(delegate);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

#if QT_VERSION >= 0x040600 // needs Qt 4.6.0 or better
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif

    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Tool);
    setPen(Qt::NoPen);
    this->setAcceptHoverEvents(true);

    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Curtain)); //Necessary to set if we want z value to be assigned correctly
}

UBGraphicsCurtainItem::~UBGraphicsCurtainItem()
{
}

QVariant UBGraphicsCurtainItem::itemChange(GraphicsItemChange change, const QVariant &value)
{

    QVariant newValue = value;

    if (Delegate())
    {
        newValue = Delegate()->itemChange(change, value);
    }

    return QGraphicsRectItem::itemChange(change, newValue);
}

void UBGraphicsCurtainItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

void UBGraphicsCurtainItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mousePressEvent(event))
    {
        //NOOP
    }
    else
    {
        QGraphicsRectItem::mousePressEvent(event);
    }
}

void UBGraphicsCurtainItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mouseMoveEvent(event))
    {
        // NOOP;
    }
    else
    {
        QGraphicsRectItem::mouseMoveEvent(event);
    }
}

void UBGraphicsCurtainItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Delegate()->mouseReleaseEvent(event);
    QGraphicsRectItem::mouseReleaseEvent(event);
}


void UBGraphicsCurtainItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor color = drawColor();
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    if(widget == UBApplication::boardController->controlView()->viewport())
    {
        color = UBSettings::paletteColor;
        if(!UBApplication::applicationController->displayManager()->hasDisplay())
        {
            color = opaqueControlColor();
        }
    }

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;

    painter->fillRect(rect(), color);
    Delegate()->postpaint(painter, option, widget);
}


UBItem* UBGraphicsCurtainItem::deepCopy() const
{
   UBGraphicsCurtainItem* copy = new UBGraphicsCurtainItem();

    copyItemParameters(copy);

   // TODO UB 4.7 ... complete all members ?

   return copy;
}

void UBGraphicsCurtainItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsCurtainItem *cp = dynamic_cast<UBGraphicsCurtainItem*>(copy);
    if (cp)
    {
        cp->setRect(this->rect());
        cp->setPos(this->pos());
        cp->setBrush(this->brush());
        cp->setPen(this->pen());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setZValue(this->zValue());
    }
}

QColor UBGraphicsCurtainItem::drawColor() const
{
    UBGraphicsScene* pScene = static_cast<UBGraphicsScene*>(QGraphicsRectItem::scene());
    return pScene->isDarkBackground() ? sDarkBackgroundDrawColor : sDrawColor;
}


QColor UBGraphicsCurtainItem::opaqueControlColor() const
{
    UBGraphicsScene* pScene = static_cast<UBGraphicsScene*>(QGraphicsRectItem::scene());
    return pScene->isDarkBackground() ? sDarkBackgroundOpaqueControlColor : sOpaqueControlColor;
}


void UBGraphicsCurtainItem::triggerRemovedSignal()
{
    emit removed();
}
