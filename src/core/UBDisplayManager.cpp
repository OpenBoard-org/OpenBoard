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

#include "UBDisplayManager.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"

#include "board/UBBoardView.h"

#include "gui/UBBlackoutWidget.h"

#include "ui_blackoutWidget.h"

#include "core/memcheck.h"

UBDisplayManager::UBDisplayManager(QObject *parent)
    : QObject(parent)
    , mControlScreenIndex(-1)
    , mDisplayScreenIndex(-1)
    , mControlWidget(0)
    , mDisplayWidget(0)
{
    mDesktop = qApp->desktop();

    mUseMultiScreen = true;

    initScreenIndexes();

    connect(mDesktop, SIGNAL(resized(int)), this, SLOT(adjustScreens(int)));
    connect(mDesktop, SIGNAL(workAreaResized(int)), this, SLOT(adjustScreens(int)));
}


void UBDisplayManager::initScreenIndexes()
{
    int screenCount = 1;//= numScreens();

    mScreenIndexesRoles.clear();

    if (screenCount > 0)
    {
        mControlScreenIndex = mDesktop->primaryScreen();

        //qDebug() <<  "The primary screen: " << mControlScreenIndex << mDesktop->screenGeometry(mControlScreenIndex);

        mScreenIndexesRoles << Control;
    }
    else
    {
        mControlScreenIndex = -1;
    }

    if (screenCount > 1)
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


UBDisplayManager::~UBDisplayManager()
{
    // NOOP
}


int UBDisplayManager::numScreens()
{
    if (mUseMultiScreen)
    {
        return mDesktop->numScreens();
    }
    else
    {
        return 1;
    }
}


int UBDisplayManager::numPreviousViews()
{
    return mPreviousScreenIndexes.size();
}


void UBDisplayManager::setAsControl(QWidget* pControlWidget )
{
    if(hasControl() && pControlWidget && (pControlWidget != mControlWidget))
    {
        mControlWidget = pControlWidget;
        mControlWidget->hide();
        mControlWidget->setGeometry(mDesktop->screenGeometry(mControlScreenIndex));
        mControlWidget->showFullScreen();
		// !!!! Should be included into Windows after QT recompilation
#ifdef Q_WS_MAC
//        mControlWidget->setAttribute(Qt::WA_MacNoShadow);
#endif
    }
}


void UBDisplayManager::setAsDisplay(QWidget* pDisplayWidget)
{
    if(hasDisplay() && pDisplayWidget && (pDisplayWidget != mDisplayWidget))
    {
        mDisplayWidget = pDisplayWidget;
        mDisplayWidget->hide();
        mDisplayWidget->setGeometry(mDesktop->screenGeometry(mDisplayScreenIndex));
        mDisplayWidget->showFullScreen();
		// !!!! Should be included into Windows after QT recompilation
#ifdef Q_WS_MAC
//        mDisplayWidget->setAttribute(Qt::WA_MacNoShadow);
#endif
    }
}


void UBDisplayManager::setAsPreviousDisplays(QList<UBBoardView*> pPreviousViews)
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

void UBDisplayManager::adjustScreens(int screen)
{
    Q_UNUSED(screen);

    initScreenIndexes();

    positionScreens();

    emit screenLayoutChanged();
}


void UBDisplayManager::positionScreens()
{

    if (mControlWidget && mControlScreenIndex > -1)
    {
        mControlWidget->setGeometry(mDesktop->screenGeometry(mControlScreenIndex));
        mControlWidget->showFullScreen();
    }

    if (mDisplayWidget && mDisplayScreenIndex > -1)
    {
        mDisplayWidget->setGeometry(mDesktop->screenGeometry(mDisplayScreenIndex));
        mDisplayWidget->showFullScreen();
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
            previous->showFullScreen();
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
        blackoutWidget->showFullScreen();
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
}


void UBDisplayManager::setRoleToScreen(DisplayRole role, int screenIndex)
{
    Q_UNUSED(role);
    Q_UNUSED(screenIndex);
}


void UBDisplayManager::setUseMultiScreen(bool pUse)
{
    mUseMultiScreen = pUse;
    adjustScreens(0);
}

