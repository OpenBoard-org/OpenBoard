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




#include <QtGui>
#include <QtSvg>

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "UBGraphicsCurtainItemDelegate.h"
#include "UBGraphicsCurtainItem.h"

#include "core/memcheck.h"

UBGraphicsCurtainItemDelegate::UBGraphicsCurtainItemDelegate(UBGraphicsCurtainItem* pDelegated, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated, parent, GF_MENU_SPECIFIED | GF_DUPLICATION_ENABLED)
{
    //NOOP
}


UBGraphicsCurtainItemDelegate::~UBGraphicsCurtainItemDelegate()
{
    //NOOP
}

void UBGraphicsCurtainItemDelegate::init()
{
    if(!mFrame){
        createControls();
        mFrame->hide();
        mZOrderUpButton->hide();
        mZOrderDownButton->hide();
    }
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
    if(mZOrderUpButton)
        mZOrderUpButton->hide();
    if(mZOrderDownButton)
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

