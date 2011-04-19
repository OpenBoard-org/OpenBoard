/*
 * UBGraphicsWidgetItemDelegate.cpp
 *
 *  Created on: October 9, 2009
 *      Author: Luc
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


UBGraphicsWidgetItemDelegate::UBGraphicsWidgetItemDelegate(UBGraphicsWidgetItem* pDelegated, int widgetType)
    : UBGraphicsItemDelegate(pDelegated, 0, true, false)
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
        freezeAction->setChecked(delegated()->widgetWebView()->isFrozen());
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

    if (delegated()->widgetWebView()->canBeTool())
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
       delegated()->widgetWebView()->freeze();
    }
    else
    {
       delegated()->widgetWebView()->unFreeze();
    }
}


UBGraphicsWidgetItem* UBGraphicsWidgetItemDelegate::delegated()
{
    return static_cast<UBGraphicsWidgetItem*>(mDelegated);
}


void UBGraphicsWidgetItemDelegate::remove(bool canundo)
{
    delegated()->remove();
    UBGraphicsItemDelegate::remove(canundo);

}
