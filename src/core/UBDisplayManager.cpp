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
#include "core/UBSettings.h"

#include "board/UBBoardView.h"
#include "board/UBBoardController.h"

#include "gui/UBBlackoutWidget.h"

#include "ui_blackoutWidget.h"

#include "core/memcheck.h"

UBDisplayManager::UBDisplayManager(QObject *parent)
    : QObject(parent)
    , mAvailableScreenCount(0)
{
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

    if (screens.count() != mAvailableScreenCount)
    {
        mAvailableScreenCount = screens.count();
        emit availableScreenCountChanged(mAvailableScreenCount);
    }

    bool swapScreens = UBSettings::settings()->swapControlAndDisplayScreens->get().toBool();

    mScreensByRole[DisplayRole::Control] = screens[0];

    if (screens.count() > 1)
    {
        QScreen* controlScreen = screens[0];
        QScreen* displayScreen = screens[1];

        if (swapScreens)
        {
            std::swap(controlScreen, displayScreen);
        }

        mScreensByRole[DisplayRole::Control] = controlScreen;

        if (mUseMultiScreen)
        {
            mScreensByRole[DisplayRole::Display] = displayScreen;
        }
    }

    for (int i = 2; i < screens.count(); ++i)
    {
        mScreensByRole[static_cast<DisplayRole>(int(DisplayRole::Previous1) + (i-2))] = screens[i];
    }

    // Desktop screen is same as Control screen
    mScreensByRole[DisplayRole::Desktop] = mScreensByRole[DisplayRole::Control];
}

int UBDisplayManager::numScreens()
{
    return mAvailableScreenCount;
}


int UBDisplayManager::numPreviousViews()
{
    int previousViews = 0;

    for (int i = 0; i < 5; ++i)
    {
        if (mScreensByRole.contains(static_cast<DisplayRole>(int(DisplayRole::Previous1) + i)))
        {
            ++previousViews;
        }
    }

    return previousViews;
}


void UBDisplayManager::setControlWidget(QWidget* pControlWidget)
{
    if(hasControl() && pControlWidget)
        mWidgetsByRole[DisplayRole::Control] = pControlWidget;
}

void UBDisplayManager::setDesktopWidget(QWidget* pDesktopWidget )
{
    if(pDesktopWidget)
        mWidgetsByRole[DisplayRole::Desktop] = pDesktopWidget;
}

void UBDisplayManager::setDisplayWidget(QWidget* pDisplayWidget)
{
    if(pDisplayWidget)
    {
        if (mWidgetsByRole.contains(DisplayRole::Display))
        {
            mWidgetsByRole[DisplayRole::Display]->hide();
            pDisplayWidget->setGeometry(mWidgetsByRole[DisplayRole::Display]->geometry());
            pDisplayWidget->setWindowFlags(mWidgetsByRole[DisplayRole::Display]->windowFlags());
        }

        mWidgetsByRole[DisplayRole::Display] = pDisplayWidget;

        if (mScreensByRole.contains(DisplayRole::Display))
        {
            mWidgetsByRole[DisplayRole::Display]->setGeometry(mScreensByRole[DisplayRole::Display]->geometry());
            UBPlatformUtils::showFullScreen(mWidgetsByRole[DisplayRole::Display]);
        }
    }
}


void UBDisplayManager::setPreviousDisplaysWidgets(QList<UBBoardView*> pPreviousViews)
{
    for (int i = 0; i < pPreviousViews.size(); ++i)
    {
        mWidgetsByRole[static_cast<DisplayRole>(int(DisplayRole::Previous1) + i)] = pPreviousViews[i];
    }
}

QWidget* UBDisplayManager::widget(DisplayRole role)
{
        return mWidgetsByRole.value(role, nullptr);
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
    if(mWidgetsByRole.contains(DisplayRole::Desktop) && hasControl())
    {
        mWidgetsByRole[DisplayRole::Desktop]->hide();
        mWidgetsByRole[DisplayRole::Desktop]->setGeometry(mScreensByRole[DisplayRole::Control]->geometry());
    }
    if (mWidgetsByRole.contains(DisplayRole::Control) && hasControl())
    {
        mWidgetsByRole[DisplayRole::Control]->showNormal();
        mWidgetsByRole[DisplayRole::Control]->setGeometry(mScreensByRole[DisplayRole::Control]->geometry());
        UBPlatformUtils::showFullScreen(mWidgetsByRole[DisplayRole::Control]);
    }

    if (mWidgetsByRole.contains(DisplayRole::Display) && hasDisplay())
    {
        mWidgetsByRole[DisplayRole::Display]->showNormal();
        mWidgetsByRole[DisplayRole::Display]->setGeometry(mScreensByRole[DisplayRole::Display]->geometry());
        UBPlatformUtils::showFullScreen(mWidgetsByRole[DisplayRole::Display]);
    }
    else if(mWidgetsByRole.contains(DisplayRole::Display))
    {
        mWidgetsByRole[DisplayRole::Display]->hide();
    }

    for (int i = 0; i < 5; ++i)
    {
        DisplayRole role = static_cast<DisplayRole>(int(DisplayRole::Previous1) + i);

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

    if (mWidgetsByRole.contains(DisplayRole::Control) && hasControl())
        mWidgetsByRole[DisplayRole::Control]->activateWindow();
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
        bool isControlScreen = screen == mScreensByRole[DisplayRole::Control];
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


void UBDisplayManager::addOrRemoveScreen(QScreen *screen)
{
    Q_UNUSED(screen);
    // adjustment must be delayed, because OS also tries to position the widgets
    QTimer::singleShot(3000, [this](){ adjustScreens(0); } );
}


void UBDisplayManager::setUseMultiScreen(bool pUse)
{
    mUseMultiScreen = pUse;
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

QRect UBDisplayManager::screenGeometry(DisplayRole role) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);
    return screen ? screen->geometry() : QRect();
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

QPixmap UBDisplayManager::grab(DisplayRole role, QRect rect) const
{
    QScreen* screen = mScreensByRole.value(role, nullptr);

    if (screen)
    {
        // see https://doc.qt.io/qt-6.2/qtwidgets-desktop-screenshot-example.html
        // for using window id 0
        return screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    }

    return QPixmap();
}

QPixmap UBDisplayManager::grabGlobal(QRect rect) const
{
    QScreen* screen = QGuiApplication::screenAt(rect.topLeft());

    if (screen) {
        rect.translate(-screen->geometry().topLeft());
        return screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    }

    return QPixmap();
}

