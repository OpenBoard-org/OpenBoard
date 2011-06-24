/*
 * UBGraphicsCurtainItemDelegate.cpp
 *
 *  Created on: June 15, 2009
 *      Author: Patrick
 */


#include <QtGui>
#include <QtSvg>

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "UBGraphicsCurtainItemDelegate.h"
#include "UBGraphicsCurtainItem.h"

#include "core/memcheck.h"

UBGraphicsCurtainItemDelegate::UBGraphicsCurtainItemDelegate(UBGraphicsCurtainItem* pDelegated, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated, parent, false)
{
    setCanDuplicate(false);
}


UBGraphicsCurtainItemDelegate::~UBGraphicsCurtainItemDelegate()
{
    //NOOP
}

void UBGraphicsCurtainItemDelegate::init()
{
    UBGraphicsItemDelegate::init();
    mFrame->hide();
}


bool UBGraphicsCurtainItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    startUndoStep();

    if (!mDelegated->isSelected())
    {
        mDelegated->setSelected(true);
        mDelegated->setZValue(UBGraphicsScene::toolLayerStart + UBGraphicsScene::toolOffsetCurtain);
        positionHandles();

        return true;
    }
    else
    {
        return false;
    }

}


QVariant UBGraphicsCurtainItemDelegate::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemZValueHasChanged)
    {
        mFrame->setZValue(mDelegated->zValue() + 1);

        foreach(DelegateButton* button, mButtons)
        {
            button->setZValue(mDelegated->zValue() + 2);
            button->setZValue(mDelegated->zValue() + 2);
        }
    }

    if (change == QGraphicsItem::ItemVisibleHasChanged)
    {
        UBGraphicsScene* ubScene = qobject_cast<UBGraphicsScene*>(mDelegated->scene());
        if(ubScene)
            ubScene->setModified(true);
    }

    return UBGraphicsItemDelegate::itemChange(change, value);
}


void UBGraphicsCurtainItemDelegate::remove(bool checked, bool canUndo)
{
    Q_UNUSED(checked);
    Q_UNUSED(canUndo);

    UBGraphicsCurtainItem *curtain = dynamic_cast<UBGraphicsCurtainItem*>(mDelegated);

    if (curtain)
    {
        curtain->setVisible(false);
        curtain->triggerRemovedSignal();
    }
}

