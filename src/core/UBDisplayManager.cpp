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
//    , mControlWidget(nullptr)
//    , mDisplayWidget(nullptr)
//    , mDesktopWidget(nullptr)
{
//    mDesktop = qApp->desktop();

    mUseMultiScreen = UBSettings::settings()->appUseMultiscreen->get().toBool();

    initScreenIndexes();

    connect(qApp, &QGuiApplication::screenAdded, this, &UBDisplayManager::addOrRemoveScreen);
    connect(qApp, &QGuiApplication::screenRemoved, this, &UBDisplayManager::addOrRemoveScreen);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &UBDisplayManager::addOrRemoveScreen);
}

UBDisplayManager::~UBDisplayManager()
{
    // NOOP
}

void UBDisplayManager::initScreenIndexes()
{
    mScreensByRole.clear();
    QScreen* primaryScreen = QGuiApplication::primaryScreen();

    QList<QScreen*> screens = primaryScreen->virtualSiblings();

    // make sure primary screen is first element and therefore never dropped
    screens.insert(0, primaryScreen);

    // drop screens which duplicate another screen, i.e. have same geometry
    for (int i = 1; i < screens.size(); )
    {
        for (int j = 0; j < i; ++j)
        {
            if (screens[i]->geometry() == screens[j]->geometry())
            {
                screens.removeAt(i);
                break;
            }
        }

        ++i;
    }

    bool swapScreens = UBSettings::settings()->swapControlAndDisplayScreens->get().toBool();
    bool useMultiscreen = UBSettings::settings()->appUseMultiscreen->get().toBool();

    mScreensByRole[Control] = screens[0];

    if (screens.count() > 1)
    {
        QScreen* controlScreen = screens[0];
        QScreen* displayScreen = screens[1];

        if (swapScreens)
        {
            std::swap(controlScreen, displayScreen);
        }

        mScreensByRole[Control] = controlScreen;

        if (useMultiscreen)
        {
            mScreensByRole[Display] = displayScreen;
        }
    }

    for (int i = 2; i < screens.count(); ++i)
    {
        mScreensByRole[static_cast<DisplayRole>(Previous1 + (i-2))] = screens[i];
    }
}

int UBDisplayManager::numScreens()
{
    return mScreensByRole.count();
}


int UBDisplayManager::numPreviousViews()
{
    int previousViews = 0;

    for (int i = 0; i < Previous5 - Previous1; ++i)
    {
        if (mScreensByRole.contains(static_cast<DisplayRole>(Previous1 + i)))
        {
            ++previousViews;
        }
    }

    return previousViews;
}


void UBDisplayManager::setControlWidget(QWidget* pControlWidget)
{
    if(hasControl() && pControlWidget)
        mWidgetsByRole[Control] = pControlWidget;
}

void UBDisplayManager::setDesktopWidget(QWidget* pDesktopWidget )
{
    if(pDesktopWidget)
        mWidgetsByRole[Desktop] = pDesktopWidget;
}

void UBDisplayManager::setDisplayWidget(QWidget* pDisplayWidget)
{
    if(pDisplayWidget)
    {
        if (mWidgetsByRole.contains(Display))
        {
            mWidgetsByRole[Display]->hide();
            pDisplayWidget->setGeometry(mWidgetsByRole[Display]->geometry());
            pDisplayWidget->setWindowFlags(mWidgetsByRole[Display]->windowFlags());
        }

        mWidgetsByRole[Display] = pDisplayWidget;

        if (mScreensByRole.contains(Display))
        {
            mWidgetsByRole[Display]->setGeometry(mScreensByRole[Display]->geometry());
            UBPlatformUtils::showFullScreen(mWidgetsByRole[Display]);
        }
    }
}


void UBDisplayManager::setPreviousDisplaysWidgets(QList<UBBoardView*> pPreviousViews)
{
    for (int i = 0; i < pPreviousViews.size(); ++i)
    {
        mWidgetsByRole[static_cast<DisplayRole>(Previous1 + i)] = pPreviousViews[i];
    }
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
    if(mWidgetsByRole.contains(Desktop) && hasControl())
    {
        mWidgetsByRole[Desktop]->hide();
        mWidgetsByRole[Desktop]->setGeometry(mScreensByRole[Control]->geometry());
    }
    if (mWidgetsByRole.contains(Control) && hasControl())
    {
        mWidgetsByRole[Control]->hide();
        mWidgetsByRole[Control]->setGeometry(mScreensByRole[Control]->geometry());
        UBPlatformUtils::showFullScreen(mWidgetsByRole[Control]);
    }

    if (mWidgetsByRole.contains(Display) && hasDisplay())
    {
        mWidgetsByRole[Display]->showNormal();
        mWidgetsByRole[Display]->setGeometry(mScreensByRole[Display]->geometry());
        UBPlatformUtils::showFullScreen(mWidgetsByRole[Display]);
    }
    else if(mWidgetsByRole.contains(Display))
    {
        mWidgetsByRole[Display]->hide();
    }

    for (int i = 0; i < Previous5 - Previous1; ++i)
    {
        DisplayRole role = static_cast<DisplayRole>(Previous1 + i);

        if (mWidgetsByRole.contains(role))
        {
            if (mScreensByRole.contains(role)) {
                QWidget* previous = mWidgetsByRole[role];
                previous->setGeometry(mScreensByRole[role]->geometry());
                UBPlatformUtils::showFullScreen(previous);
            }
            else
            {
                mWidgetsByRole[role]->hide();
            }
        }
    }

    if (mWidgetsByRole.contains(Control) && hasControl())
        mWidgetsByRole[Control]->activateWindow();
}


void UBDisplayManager::blackout()
{
    for (auto screen : mScreensByRole)
    {
        UBBlackoutWidget *blackoutWidget = new UBBlackoutWidget(); //deleted in UBDisplayManager::unBlackout
        Ui::BlackoutWidget *blackoutUi = new Ui::BlackoutWidget();
        blackoutUi->setupUi(blackoutWidget);

        connect(blackoutUi->iconButton, SIGNAL(pressed()), blackoutWidget, SLOT(doActivity()));
        connect(blackoutWidget, SIGNAL(activity()), this, SLOT(unBlackout()));

        // display Uniboard logo on main screen
        bool isControlScreen = screen == mScreensByRole[Control];
        blackoutUi->iconButton->setVisible(isControlScreen);
        blackoutUi->labelClickToReturn->setVisible(isControlScreen);

        blackoutWidget->setGeometry(screen->geometry());

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

void UBDisplayManager::addOrRemoveScreen(QScreen *screen)
{

}


void UBDisplayManager::setUseMultiScreen(bool pUse)
{
    mUseMultiScreen = pUse;
}

int UBDisplayManager::controleScreenIndex()
{
    // FIXME should later be removed, just for compatibility
    return mScreensByRole[Control] == QGuiApplication::primaryScreen() ? 0 : 1;
}

QSize UBDisplayManager::screenSize(DisplayRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->size() : QSize();
}

QSize UBDisplayManager::availableScreenSize(DisplayRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->availableSize() : QSize();
}

qreal UBDisplayManager::physicalDpi(DisplayRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->physicalDotsPerInch() : 96.;
}

qreal UBDisplayManager::logicalDpi(DisplayRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->logicalDotsPerInch() : 96.;
}

QRect UBDisplayManager::controlGeometry()
{
    return qApp->desktop()->screenGeometry(controleScreenIndex());
}

QPixmap UBDisplayManager::grab(DisplayRole role, QRect rect)
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    QWidget* widget = mWidgetsByRole.value(role, nullptr);

    if (screen && widget)
    {
        return screen->grabWindow(widget->winId(), rect.x(), rect.y(), rect.width(), rect.height());
    }

    return QPixmap();
}

