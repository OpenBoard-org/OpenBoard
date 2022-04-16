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

#include "UBDisplayManager.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"

#include "board/UBBoardView.h"
#include "board/UBBoardController.h"

#include "gui/UBBlackoutWidget.h"

#include "ui_blackoutWidget.h"

#include "core/memcheck.h"

#include "qdesktopwidget.h"

UBDisplayManager::UBDisplayManager(QObject *parent)
    : QObject(parent)
    , mControlScreenIndex(-1)
    , mDisplayScreenIndex(-1)
    , mControlWidget(0)
    , mDisplayWidget(0)
    , mDesktopWidget(0)
{
    mDesktop = qApp->desktop();

    mUseMultiScreen = UBSettings::settings()->appUseMultiscreen->get().toBool();

    initScreenIndexes();

    connect(mDesktop, &QDesktopWidget::resized, this, &UBDisplayManager::adjustScreens);
    connect(mDesktop, &QDesktopWidget::workAreaResized, this, &UBDisplayManager::adjustScreens);
}


void UBDisplayManager::initScreenIndexes()
{
    int screenCount = numScreens();

    mScreenIndexesRoles.clear();

    if (screenCount > 0)
    {
        mControlScreenIndex = mDesktop->primaryScreen();
        if (screenCount > 1 && UBSettings::settings()->swapControlAndDisplayScreens->get().toBool())
        {
            mControlScreenIndex = mControlScreenIndex^1;
        }

        mScreenIndexesRoles << Control;
    }
    else
    {
        mControlScreenIndex = -1;
    }

    if (screenCount > 1 && mUseMultiScreen)
    {
        mDisplayScreenIndex = mControlScreenIndex != 0 ? 0 : 1;
        mScreenIndexesRoles << Display;
    }
    else
    {
        mDisplayScreenIndex = -1;
    }

    mPreviousScreenIndexes.clear();

    if (screenCount > 2)
    {
        for(int i = 2; i < screenCount; i++)
        {
            if(mControlScreenIndex == i)
                mPreviousScreenIndexes.append(1);
            else
                mPreviousScreenIndexes.append(i);
        }
    }
}

void UBDisplayManager::swapDisplayScreens(bool swap)
{
    initScreenIndexes();

    if (swap)
    {
        // As it s a really specific ask and we don't have much time to handle it correctly
        // this code handles only the swap between the main display screen and the first previous one
        int displayScreenIndex = mDisplayScreenIndex;
        mDisplayScreenIndex = mPreviousScreenIndexes.at(0);
        mPreviousScreenIndexes.clear();
        mPreviousScreenIndexes.append(displayScreenIndex);
    }

    positionScreens();

    emit screenLayoutChanged();
    emit adjustDisplayViewsRequired();
}


UBDisplayManager::~UBDisplayManager()
{
    // NOOP
}


int UBDisplayManager::numScreens()
{
    int screenCount = mDesktop->screenCount();
    // Some window managers report two screens when the two monitors are in "cloned" mode; this hack ensures
    // that we consider this as just one screen. On most desktops, at least one of the following conditions is
    // a good indicator of the displays being in cloned or extended mode.
#ifdef Q_OS_LINUX
    if (screenCount > 1
        && (mDesktop->screenNumber(mDesktop->screen(0)) == mDesktop->screenNumber(mDesktop->screen(1))
            || mDesktop->screenGeometry(0) == mDesktop->screenGeometry(1)))
        return 1;
#endif
    return screenCount;
}


int UBDisplayManager::numPreviousViews()
{
    return mPreviousScreenIndexes.size();
}


void UBDisplayManager::setControlWidget(QWidget* pControlWidget)
{
    if(hasControl() && pControlWidget && (pControlWidget != mControlWidget))
        mControlWidget = pControlWidget;
}

void UBDisplayManager::setDesktopWidget(QWidget* pControlWidget )
{
    if(pControlWidget && (pControlWidget != mControlWidget))
        mDesktopWidget = pControlWidget;
}

void UBDisplayManager::setDisplayWidget(QWidget* pDisplayWidget)
{
    if(pDisplayWidget && (pDisplayWidget != mDisplayWidget))
    {
        if (mDisplayWidget)
        {
            mDisplayWidget->hide();
            pDisplayWidget->setGeometry(mDisplayWidget->geometry());
            pDisplayWidget->setWindowFlags(mDisplayWidget->windowFlags());
        }
        mDisplayWidget = pDisplayWidget;
        mDisplayWidget->setGeometry(mDesktop->screenGeometry(mDisplayScreenIndex));
        if (UBSettings::settings()->appUseMultiscreen->get().toBool())
            UBPlatformUtils::showFullScreen(mDisplayWidget);
    }
}


void UBDisplayManager::setPreviousDisplaysWidgets(QList<UBBoardView*> pPreviousViews)
{
    mPreviousDisplayWidgets = pPreviousViews;
}


QRect UBDisplayManager::controlGeometry()
{
    return mDesktop->screenGeometry(mControlScreenIndex);
}

QRect UBDisplayManager::displayGeometry()
{
    return mDesktop->screenGeometry(mDisplayScreenIndex);
}

void UBDisplayManager::reinitScreens(bool swap)
{
    Q_UNUSED(swap);
    adjustScreens(-1);
}

void UBDisplayManager::adjustScreens(int screen)
{
    Q_UNUSED(screen);

    initScreenIndexes();

    positionScreens();

    emit screenLayoutChanged();
}


void UBDisplayManager::positionScreens()
{
    if(mDesktopWidget && mControlScreenIndex > -1)
    {
        mDesktopWidget->hide();
        mDesktopWidget->setGeometry(mDesktop->screenGeometry(mControlScreenIndex));
    }
    if (mControlWidget && mControlScreenIndex > -1)
    {
        mControlWidget->hide();
        mControlWidget->setGeometry(mDesktop->screenGeometry(mControlScreenIndex));
        UBPlatformUtils::showFullScreen(mControlWidget);
    }

    if (mDisplayWidget && mDisplayScreenIndex > -1)
    {
        mDisplayWidget->hide();
        mDisplayWidget->setGeometry(mDesktop->screenGeometry(mDisplayScreenIndex));
        UBPlatformUtils::showFullScreen(mDisplayWidget);
    }
    else if(mDisplayWidget)
    {
        mDisplayWidget->hide();
    }

    for (int wi = mPreviousScreenIndexes.size(); wi < mPreviousDisplayWidgets.size(); wi++)
    {
        mPreviousDisplayWidgets.at(wi)->hide();
    }

    for (int psi = 0; psi < mPreviousScreenIndexes.size(); psi++)
    {
        if (mPreviousDisplayWidgets.size() > psi)
        {
            QWidget* previous = mPreviousDisplayWidgets.at(psi);
            previous->setGeometry(mDesktop->screenGeometry(mPreviousScreenIndexes.at(psi)));
            UBPlatformUtils::showFullScreen(previous);
        }
    }

    if (mControlWidget && mControlScreenIndex > -1)
        mControlWidget->activateWindow();

}


void UBDisplayManager::blackout()
{
    QList<int> screenIndexes;

    if (mControlScreenIndex > -1)
        screenIndexes << mControlScreenIndex;

    if (mDisplayScreenIndex > -1)
        screenIndexes << mDisplayScreenIndex;

    screenIndexes << mPreviousScreenIndexes;

    for (int i = 0; i < screenIndexes.size(); i++)
    {
        int screenIndex = screenIndexes.at(i);

        UBBlackoutWidget *blackoutWidget = new UBBlackoutWidget(); //deleted in UBDisplayManager::unBlackout
        Ui::BlackoutWidget *blackoutUi = new Ui::BlackoutWidget();
        blackoutUi->setupUi(blackoutWidget);

        connect(blackoutUi->iconButton, SIGNAL(pressed()), blackoutWidget, SLOT(doActivity()));
        connect(blackoutWidget, SIGNAL(activity()), this, SLOT(unBlackout()));

        // display Uniboard logo on main screen
        blackoutUi->iconButton->setVisible(screenIndex == mControlScreenIndex);
        blackoutUi->labelClickToReturn->setVisible(screenIndex == mControlScreenIndex);

        blackoutWidget->setGeometry(mDesktop->screenGeometry(screenIndex));

        mBlackoutWidgets << blackoutWidget;
    }

    UBPlatformUtils::fadeDisplayOut();

    foreach(UBBlackoutWidget *blackoutWidget, mBlackoutWidgets)
    {
        UBPlatformUtils::showFullScreen(blackoutWidget);
    }
}

void UBDisplayManager::unBlackout()
{
    while (!mBlackoutWidgets.isEmpty())
    {
        // the widget is also destroyed thanks to its Qt::WA_DeleteOnClose attribute
        mBlackoutWidgets.takeFirst()->close();
    }

    UBPlatformUtils::fadeDisplayIn();

    UBApplication::boardController->freezeW3CWidgets(false);

}


void UBDisplayManager::setRoleToScreen(DisplayRole role, int screenIndex)
{
    Q_UNUSED(role);
    Q_UNUSED(screenIndex);
}


void UBDisplayManager::setUseMultiScreen(bool pUse)
{
    mUseMultiScreen = pUse;
}

