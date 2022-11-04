/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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
{
    mUseMultiScreen = UBSettings::settings()->appUseMultiscreen->get().toBool();

    initScreenIndexes();

    connect(qApp, &QGuiApplication::screenAdded, this, &UBDisplayManager::addOrRemoveScreen);
    connect(qApp, &QGuiApplication::screenRemoved, this, &UBDisplayManager::addOrRemoveScreen);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &UBDisplayManager::addOrRemoveScreen);

    connect(UBSettings::settings()->appScreenList, &UBSetting::changed, this, &UBDisplayManager::adjustScreens);
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
        QRect iGeomentry = screens[i]->geometry();
        bool removed = false;

        for (int j = 0; j < i; ++j)
        {
            QRect jGeometry = screens[j]->geometry();

            if (iGeomentry.contains(jGeometry) || jGeometry.contains(iGeomentry))
            {
                screens.removeAt(i);
                removed = true;
                break;
            }
        }

        if (!removed)
        {
            ++i;
        }
    }

    if (screens != mAvailableScreens)
    {
        qDebug() << "availableScreens" << screens;
        mAvailableScreens = screens;
        emit availableScreenCountChanged(screens.count());
    }

    /*
     * Emitting availableScreenCountChanged updated the screen configuration
     * in the UBPreferencesManager, so we now get the screen list valid for
     * the new monitor configuration.
     */
}

void UBDisplayManager::initScreensByRole()
{
    mScreensByRole.clear();
    bool swapScreens = UBSettings::settings()->swapControlAndDisplayScreens->get().toBool();

    mScreensByRole[ScreenRole::Control] = mAvailableScreens[0];

    if (mAvailableScreens.count() > 1)
    {
        QScreen* controlScreen = mAvailableScreens[0];
        QScreen* displayScreen = mAvailableScreens[1];

        if (swapScreens)
        {
            std::swap(controlScreen, displayScreen);
        }

        mScreensByRole[ScreenRole::Control] = controlScreen;

        mScreensByRole[ScreenRole::Display] = displayScreen;

        ScreenRole role(ScreenRole::Previous1);

        for (int i = 2; i < mAvailableScreens.count(); ++i)
        {
            mScreensByRole[role++] = mAvailableScreens[i];
        }
    }

    // Desktop screen is same as Control screen
    mScreensByRole[ScreenRole::Desktop] = mScreensByRole[ScreenRole::Control];
}


void UBDisplayManager::assignRoles()
{
    QVariant appScreenList = UBSettings::settings()->appScreenList->get();
    QStringList screenList = appScreenList.toStringList();
    qDebug() << "assignRoles using screen list" << screenList;

    if (!appScreenList.isValid())
    {
        // no entry in configuration files
        // "old" configuration mode
        initScreensByRole();
    }
    else
    {
        mScreensByRole.clear();
        // converting an empty entry to a QStringList creates a list with one empty QString
        if (appScreenList.toStringList().isEmpty() || appScreenList.toStringList().at(0).isEmpty())
        {
            screenList.clear();

            // explicitly stored an empty list, take all available screens
            for (int index = 1; index <= mAvailableScreens.count(); ++index)
            {
                screenList << QString::number(index);
            }
        }

        // "new" configuration mode using list of screen indexes
        // convert to index list and check: are all numbers in range?
        QList<int> indexList;

        for (QString& entry : screenList)
        {
            int index = entry.toInt();

            if (index < 1 || index > mAvailableScreens.size())
            {
                qDebug() << "Warning: Screen index out of range, skipped:" << index;
            }
            else
            {
                indexList << index - 1;
            }
        }

        // configure control screen, by default use primary screen
        QScreen* controlScreen = mAvailableScreens[0];

        if (!indexList.empty() && mAvailableScreens[indexList[0]])
        {
            controlScreen = mAvailableScreens[indexList[0]];
        }

        mScreensByRole[ScreenRole::Control] = controlScreen;

        // configure display screen
        if (mUseMultiScreen && indexList.count() > 1)
        {
            QScreen* displayScreen = mAvailableScreens[indexList[1]];

            if (displayScreen)
            {
                mScreensByRole[ScreenRole::Display] = displayScreen;
            }
        }

        // configure previous screens
        ScreenRole role = ScreenRole::Previous1;

        for (int i = 2; i < indexList.count(); ++i)
        {
            QScreen* previousScreen = mAvailableScreens[indexList[i]];

            if (previousScreen)
            {
                mScreensByRole[role++] = previousScreen;
            }
        }

        // Desktop screen is same as Control screen
        mScreensByRole[ScreenRole::Desktop] = mScreensByRole[ScreenRole::Control];
    }

    emit screenRolesAssigned();
}

int UBDisplayManager::numScreens()
{
    return mAvailableScreens.count();
}


int UBDisplayManager::numPreviousViews()
{
    int previousViews = 0;

    for (ScreenRole role = ScreenRole::Previous1; role <= ScreenRole::Previous5; ++role)
    {
        if (screen(role))
        {
            ++previousViews;
        }
    }

    return previousViews;
}


void UBDisplayManager::setControlWidget(QWidget* pControlWidget)
{
    if (pControlWidget)
    {
        mWidgetsByRole[ScreenRole::Control] = pControlWidget;
    }
}

void UBDisplayManager::setDesktopWidget(QWidget* pDesktopWidget )
{
    if (pDesktopWidget)
    {
        mWidgetsByRole[ScreenRole::Desktop] = pDesktopWidget;
    }
}

void UBDisplayManager::setDisplayWidget(QWidget* pDisplayWidget)
{
    if (pDisplayWidget && pDisplayWidget != widget((ScreenRole::Display)))
    {
        if (widget(ScreenRole::Display))
        {
            widget(ScreenRole::Display)->hide();
            pDisplayWidget->setGeometry(widget(ScreenRole::Display)->geometry());
            pDisplayWidget->setWindowFlags(widget(ScreenRole::Display)->windowFlags());
        }

        mWidgetsByRole[ScreenRole::Display] = pDisplayWidget;

        if (screen(ScreenRole::Display))
        {
            pDisplayWidget->setGeometry(screen(ScreenRole::Display)->geometry());
            UBPlatformUtils::showFullScreen(pDisplayWidget);
        }
    }
}


void UBDisplayManager::setPreviousDisplaysWidgets(QList<UBBoardView*> pPreviousViews)
{
    ScreenRole role(ScreenRole::Previous1);

    for (int i = 0; i < pPreviousViews.size(); ++i)
    {
        mWidgetsByRole[role++] = pPreviousViews[i];
    }
}

QWidget* UBDisplayManager::widget(ScreenRole role) const
{
    return mWidgetsByRole.value(role);
}

QScreen* UBDisplayManager::screen(ScreenRole role) const
{
    return mScreensByRole.value(role);
}

QList<QScreen *> UBDisplayManager::availableScreens() const
{
    return mAvailableScreens;
}

void UBDisplayManager::adjustScreens()
{
    assignRoles();
    positionScreens();
}

void UBDisplayManager::positionScreens()
{
    qDebug() << "positionScreens";
    if (widget(ScreenRole::Desktop) && hasControl())
    {
        widget(ScreenRole::Desktop)->hide();
        widget(ScreenRole::Desktop)->setGeometry(screen(ScreenRole::Control)->geometry());
    }

    if (widget(ScreenRole::Control) && hasControl())
    {
        QWidget* controlWidget = widget(ScreenRole::Control);

        // Sometimes moving control screen to the left won't operate...
        // found out that resetting the geometry solves theses cases better than "showNormal() workaround"
        controlWidget->setGeometry(QRect());

        QRect geometry = screenGeometry(ScreenRole::Control);

        if (UBSettings::settings()->appRunInWindow->get().toBool())
        {
            if (controlWidget->property("isInitialized").toBool())
            {
                // reuse previous size and relative position
                QRect previousGeometry = controlWidget->geometry();
                QScreen* previousScreen = QGuiApplication::screenAt(previousGeometry.topLeft());

                if (previousScreen)
                {
                    QRect previousScreenGeometry = QGuiApplication::screenAt(previousGeometry.topLeft())->geometry();
                    QPoint offset = previousGeometry.topLeft() - previousScreenGeometry.topLeft();
                    geometry.setSize(previousGeometry.size());
                    geometry.moveTo(geometry.topLeft() + offset);

                    // make sure widget fits to screen
                    geometry = screenGeometry(ScreenRole::Control).intersected(geometry);
                }
            }
            else
            {
                // calculate an initial geometry for window mode
                geometry.setSize(geometry.size() - QSize(150, 150));
                geometry.translate(50, 50);
                controlWidget->setProperty("isInitialized", true);
            }
        }

        qDebug() << "control geometry" << geometry;
        controlWidget->setGeometry(geometry);
        UBPlatformUtils::showFullScreen(controlWidget);
    }

    if (widget(ScreenRole::Display) && hasDisplay() && mUseMultiScreen)
    {
        qDebug() << "display geometry" << screenGeometry(ScreenRole::Display);
        widget(ScreenRole::Display)->showNormal();
        widget(ScreenRole::Display)->setGeometry(screenGeometry(ScreenRole::Display));
        UBPlatformUtils::showFullScreen(widget(ScreenRole::Display));
    }
    else if (widget(ScreenRole::Display))
    {
        widget(ScreenRole::Display)->hide();
    }

    for (ScreenRole role = ScreenRole::Previous1; role <= ScreenRole::Previous5; ++role)
    {
        if (widget(role))
        {
            if (screen(role)) {
                QWidget* previous = widget(role);
                qDebug() << "previous display geometry" << screenGeometry(role);
                previous->showNormal();
                previous->setGeometry(screenGeometry(role));
                UBPlatformUtils::showFullScreen(previous);
            }
            else
            {
                widget(role)->hide();
            }
        }
    }

    if (widget(ScreenRole::Control) && hasControl())
    {
        widget(ScreenRole::Control)->activateWindow();
    }

    emit screenLayoutChanged();
}


void UBDisplayManager::blackout()
{
    for (auto& screen : mScreensByRole)
    {
        UBBlackoutWidget* blackoutWidget = new UBBlackoutWidget(); //deleted in UBDisplayManager::unBlackout
        Ui::BlackoutWidget* blackoutUi = new Ui::BlackoutWidget();
        blackoutUi->setupUi(blackoutWidget);

        connect(blackoutUi->iconButton, SIGNAL(pressed()), blackoutWidget, SLOT(doActivity()));
        connect(blackoutWidget, SIGNAL(activity()), this, SLOT(unBlackout()));

        // display Uniboard logo on main screen
        bool isControlScreen = screen == mScreensByRole[ScreenRole::Control];
        blackoutUi->iconButton->setVisible(isControlScreen);
        blackoutUi->labelClickToReturn->setVisible(isControlScreen);

        blackoutWidget->setGeometry(screen->geometry());

        mBlackoutWidgets << blackoutWidget;
        mBlackoutUiList << blackoutUi;
    }

    UBPlatformUtils::fadeDisplayOut();

    for (UBBlackoutWidget* blackoutWidget : qAsConst(mBlackoutWidgets))
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

    qDeleteAll(mBlackoutUiList);
    mBlackoutUiList.clear();

    UBPlatformUtils::fadeDisplayIn();

    UBApplication::boardController->freezeW3CWidgets(false);
}

void UBDisplayManager::addOrRemoveScreen(QScreen *screen)
{
    Q_UNUSED(screen);

    initScreenIndexes();
    assignRoles();

    // positioning must be delayed, because OS also tries to position the widgets
    QTimer::singleShot(3000, this, [this](){
        positionScreens();
    });
}

void UBDisplayManager::setUseMultiScreen(bool pUse)
{
    mUseMultiScreen = pUse;
}

QSize UBDisplayManager::screenSize(ScreenRole role) const
{
    QScreen* scr = screen(role);
    return scr ? scr->size() : QSize();
}

QSize UBDisplayManager::availableScreenSize(ScreenRole role) const
{
    QScreen* scr = screen(role);
    return scr ? scr->availableSize() : QSize();
}

QRect UBDisplayManager::screenGeometry(ScreenRole role) const
{
    QScreen* scr = screen(role);
    return scr ? scr->geometry() : QRect();
}

qreal UBDisplayManager::physicalDpi(ScreenRole role) const
{
    QScreen* scr = screen(role);
    return scr ? scr->physicalDotsPerInch() : 96.;
}

qreal UBDisplayManager::logicalDpi(ScreenRole role) const
{
    QScreen* scr = screen(role);
    return scr ? scr->logicalDotsPerInch() : 96.;
}

QPixmap UBDisplayManager::grab(ScreenRole role, QRect rect) const
{
    QScreen* scr = screen(role);

    if (scr)
    {
        // see https://doc.qt.io/qt-6.2/qtwidgets-desktop-screenshot-example.html
        // for using window id 0
        return scr->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    }

    return QPixmap();
}

QPixmap UBDisplayManager::grabGlobal(QRect rect) const
{
    QScreen* screen = QGuiApplication::screenAt(rect.topLeft());

    if (screen)
    {
        rect.translate(-screen->geometry().topLeft());
        return screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    }

    return QPixmap();
}


ScreenRole &operator++(ScreenRole &role)
{
    role = ScreenRole(int(role) + 1);
    return role;
}

ScreenRole operator++(ScreenRole &role, int)
{
    ScreenRole old = role;  // copy old value
    ++role;                 // prefix increment
    return old;             // return old value
}
