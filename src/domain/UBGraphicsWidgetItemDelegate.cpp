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


#include <QtGui>
#include <QtSvg>

#include "UBGraphicsWidgetItemDelegate.h"
#include "UBGraphicsScene.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsDelegateFrame.h"

#include "core/memcheck.h"

UBGraphicsWidgetItemDelegate::UBGraphicsWidgetItemDelegate(UBGraphicsWidgetItem* pDelegated, int widgetType)
    : UBGraphicsItemDelegate(pDelegated, 0, true, false, false)
    , freezeAction(0)
    , setAsToolAction(0)
{
    mWidgetType = widgetType;
}


UBGraphicsWidgetItemDelegate::~UBGraphicsWidgetItemDelegate()
{
    // NOOP
}


void UBGraphicsWidgetItemDelegate::pin()
{
    UBApplication::boardController->moveGraphicsWidgetToControlView(delegated());
}


void UBGraphicsWidgetItemDelegate::updateMenuActionState()
{
    UBGraphicsItemDelegate::updateMenuActionState();

    if (freezeAction)
        freezeAction->setChecked(delegated()->isFrozen());
}

void UBGraphicsWidgetItemDelegate::decorateMenu(QMenu* menu)
{
    UBGraphicsItemDelegate::decorateMenu(menu);

    freezeAction = menu->addAction(tr("Frozen"), this, SLOT(freeze(bool)));

    QIcon freezeIcon;
    freezeIcon.addPixmap(QPixmap(":/images/frozen.svg"), QIcon::Normal, QIcon::On);
    freezeIcon.addPixmap(QPixmap(":/images/unfrozen.svg"), QIcon::Normal, QIcon::Off);
    freezeAction->setIcon(freezeIcon);

    freezeAction->setCheckable(true);

    if (delegated()->canBeTool())
    {
        setAsToolAction = mMenu->addAction(tr("Transform as Tool "), this, SLOT(pin()));
        QIcon pinIcon;
        pinIcon.addPixmap(QPixmap(":/images/unpin.svg"), QIcon::Normal, QIcon::On);
        pinIcon.addPixmap(QPixmap(":/images/pin.svg"), QIcon::Normal, QIcon::Off);
        setAsToolAction->setIcon(pinIcon);
    }
}


void UBGraphicsWidgetItemDelegate::freeze(bool frozen)
{
    if(frozen)
    {
       delegated()->freeze();
    }
    else
    {
       delegated()->unFreeze();
    }
}


UBGraphicsWidgetItem* UBGraphicsWidgetItemDelegate::delegated()
{
    return static_cast<UBGraphicsWidgetItem*>(mDelegated);
}


void UBGraphicsWidgetItemDelegate::remove(bool canundo)
{
    delegated()->removeScript();
    UBGraphicsItemDelegate::remove(canundo);
}
