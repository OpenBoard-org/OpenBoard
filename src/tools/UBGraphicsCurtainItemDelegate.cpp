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



#include <QtGui>
#include <QtSvg>

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "UBGraphicsCurtainItemDelegate.h"
#include "UBGraphicsCurtainItem.h"

#include "core/memcheck.h"

UBGraphicsCurtainItemDelegate::UBGraphicsCurtainItemDelegate(UBGraphicsCurtainItem* pDelegated, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated, parent, GF_SCALABLE_ALL_AXIS | GF_MENU_SPECIFIED | GF_ZORDER_MANIPULATIONS_ALLOWED)
{
    //NOOP
}


UBGraphicsCurtainItemDelegate::~UBGraphicsCurtainItemDelegate()
{
    //NOOP
}

void UBGraphicsCurtainItemDelegate::init()
{
    mFrame->hide();
    mZOrderUpButton->hide();
    mZOrderDownButton->hide();
}


bool UBGraphicsCurtainItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    startUndoStep();

    if (!mDelegated->isSelected())
    {
        mDelegated->setSelected(true);
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
//        mFrame->setZValue(mDelegated->zValue() + 1);

//        foreach(DelegateButton* button, mButtons)
//        {
//            button->setZValue(mDelegated->zValue() + 2);
//            button->setZValue(mDelegated->zValue() + 2);
//        }
    }

    if (change == QGraphicsItem::ItemVisibleHasChanged)
    {
        UBGraphicsScene* ubScene = qobject_cast<UBGraphicsScene*>(mDelegated->scene());
        if(ubScene)
            ubScene->setModified(true);
    }

    return UBGraphicsItemDelegate::itemChange(change, value);
}

void UBGraphicsCurtainItemDelegate::positionHandles()
{
    UBGraphicsItemDelegate::positionHandles();
    mZOrderUpButton->hide();
    mZOrderDownButton->hide();
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

