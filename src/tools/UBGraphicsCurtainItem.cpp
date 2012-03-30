/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    mDelegate = new UBGraphicsCurtainItemDelegate(this, 0);
    mDelegate->init();
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
    delete mDelegate;
}

QVariant UBGraphicsCurtainItem::itemChange(GraphicsItemChange change, const QVariant &value)
{

    QVariant newValue = value;

    if (mDelegate)
    {
        newValue = mDelegate->itemChange(change, value);
    }

    return QGraphicsRectItem::itemChange(change, newValue);
}

void UBGraphicsCurtainItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDelegate->mousePressEvent(event))
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
    if (mDelegate->mouseMoveEvent(event))
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
    mDelegate->mouseReleaseEvent(event);
    QGraphicsRectItem::mouseReleaseEvent(event);
}


void UBGraphicsCurtainItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor color = drawColor();

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
}


UBItem* UBGraphicsCurtainItem::deepCopy() const
{
   UBGraphicsCurtainItem* copy = new UBGraphicsCurtainItem();

   copy->setRect(this->rect());
   copy->setPos(this->pos());
   copy->setBrush(this->brush());
   copy->setPen(this->pen());
   copy->setTransform(this->transform());
   copy->setFlag(QGraphicsItem::ItemIsMovable, true);
   copy->setFlag(QGraphicsItem::ItemIsSelectable, true);
   copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));

   // TODO UB 4.7 ... complete all members ?

   return copy;
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


void UBGraphicsCurtainItem::remove()
{
    if (mDelegate)
        mDelegate->remove(true);
}


void UBGraphicsCurtainItem::triggerRemovedSignal()
{
    emit removed();
}
