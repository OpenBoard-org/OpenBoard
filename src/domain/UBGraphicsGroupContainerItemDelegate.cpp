/*
 * Copyright (C) 2015-2016 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBGraphicsGroupContainerItemDelegate.h"

#include <QtGui>

#include "UBGraphicsScene.h"
#include "gui/UBResources.h"

#include "domain/UBGraphicsDelegateFrame.h"
#include "domain/UBGraphicsGroupContainerItem.h"

#include "board/UBBoardController.h"
#include "gui/UBCreateLinkPalette.h"

#include "core/memcheck.h"
#include "UBGraphicsScene.h"

#include "core/UBApplication.h"

#include "board/UBBoardPaletteManager.h"


#include "gui/UBResources.h"
#include "board/UBDrawingController.h"


#include "customWidgets/UBGraphicsItemAction.h"

#include "core/memcheck.h"

UBGraphicsGroupContainerItemDelegate::UBGraphicsGroupContainerItemDelegate(QGraphicsItem *pDelegated, QObject *parent) :
    UBGraphicsItemDelegate(pDelegated, parent, GF_COMMON | GF_RESPECT_RATIO), mDestroyGroupButton(0)

{
}

UBGraphicsGroupContainerItem *UBGraphicsGroupContainerItemDelegate::delegated()
{
    return dynamic_cast<UBGraphicsGroupContainerItem*>(mDelegated);
}

void UBGraphicsGroupContainerItemDelegate::decorateMenu(QMenu *menu)
{
    mLockAction = menu->addAction(tr("Locked"), this, SLOT(lock(bool)));
    QIcon lockIcon;
    lockIcon.addPixmap(QPixmap(":/images/locked.svg"), QIcon::Normal, QIcon::On);
    lockIcon.addPixmap(QPixmap(":/images/unlocked.svg"), QIcon::Normal, QIcon::Off);
    mLockAction->setIcon(lockIcon);
    mLockAction->setCheckable(true);

    mShowOnDisplayAction = mMenu->addAction(tr("Visible on Extended Screen"), this, SLOT(showHide(bool)));
    mShowOnDisplayAction->setCheckable(true);

    QIcon showIcon;
    showIcon.addPixmap(QPixmap(":/images/eyeOpened.svg"), QIcon::Normal, QIcon::On);
    showIcon.addPixmap(QPixmap(":/images/eyeClosed.svg"), QIcon::Normal, QIcon::Off);
    mShowOnDisplayAction->setIcon(showIcon);

    mShowPanelToAddAnAction = menu->addAction(tr("Add an action"),this,SLOT(onAddActionClicked()));
}

//TODO claudio
// duplicated code UBGraphicsDelegateItem
void UBGraphicsGroupContainerItemDelegate::onAddActionClicked()
{
    UBCreateLinkPalette* linkPalette = UBApplication::boardController->paletteManager()->linkPalette();
    linkPalette->show();
    connect(linkPalette,SIGNAL(definedAction(UBGraphicsItemAction*)),this,SLOT(saveAction(UBGraphicsItemAction*)));
}

//TODO claudio
// duplicated code UBGraphicsDelegateItem
void UBGraphicsGroupContainerItemDelegate::saveAction(UBGraphicsItemAction* action)
{
    mAction = action;
    mMenu->removeAction(mShowPanelToAddAnAction);
    QString actionLabel;
    switch (mAction->linkType()) {
    case eLinkToAudio:
        actionLabel= tr("Remove link to audio");
        break;
    case eLinkToPage:
        actionLabel = tr("Remove link to page");
        break;
    case eLinkToWebUrl:
        actionLabel = tr("Remove link to web url");
    default:
        break;
    }

    mRemoveAnAction = mMenu->addAction(actionLabel,this,SLOT(onRemoveActionClicked()));
    mMenu->addAction(mRemoveAnAction);
}

void UBGraphicsGroupContainerItemDelegate::setAction(UBGraphicsItemAction* action)
{
    //qWarning()<<"setAction.................................................. @@";
    UBGraphicsItemDelegate::setAction(action);
}

//TODO claudio
// duplicated code UBGraphicsDelegateItem
void UBGraphicsGroupContainerItemDelegate::onRemoveActionClicked()
{
    if(mAction){
        mAction->actionRemoved();
        delete mAction;
        mAction = NULL;
    }
    mMenu->removeAction(mRemoveAnAction);
    mMenu->addAction(mShowPanelToAddAnAction);
}

void UBGraphicsGroupContainerItemDelegate::buildButtons()
{
    if (!mDestroyGroupButton) {
        mDestroyGroupButton = new DelegateButton(":/images/ungroupItems.svg", mDelegated, mFrame, Qt::TopLeftSection);
        mDestroyGroupButton->setShowProgressIndicator(false);
        connect(mDestroyGroupButton, SIGNAL(clicked()), this, SLOT(destroyGroup()));
        mButtons << mDestroyGroupButton;
    }

    UBGraphicsItemDelegate::buildButtons();
}

void UBGraphicsGroupContainerItemDelegate::freeButtons()
{
    UBGraphicsItemDelegate::freeButtons();
    mDestroyGroupButton = 0;
    mButtons.clear();
}

bool UBGraphicsGroupContainerItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    delegated()->deselectCurrentItem();
    return false;
}

bool UBGraphicsGroupContainerItemDelegate::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    return false;
}

bool UBGraphicsGroupContainerItemDelegate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    return false;
}

void UBGraphicsGroupContainerItemDelegate::destroyGroup()
{
    qDebug() << "Destroying group";
    delegated()->destroy();
}
