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




#include "UBApplication.h"

#include "UBDisplayManager.h"
#include "gui/UBMainWindow.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBBoardView.h"
#include "frameworks/UBPlatformUtils.h"

#include <QtGui>
#include <QFontDatabase>
#include <QStyleFactory>
#include "UBApplicationController.h"

#include "ui_mainWindow.h"

UBApplicationController* UBApplication::applicationController = 0;
UBBoardController* UBApplication::boardController = 0;
UBMainWindow* UBApplication::mainWindow = 0;

#if defined(Q_OS_OSX) || defined(Q_OS_LINUX)
bool bIsMinimized = false;
#endif //defined(Q_OS_OSX) || defined(Q_OS_LINUX)

UBApplication::UBApplication(const QString &id, int &argc, char **argv) : QApplication(argc, argv)
{

    installEventFilter(this);
}


UBApplication::~UBApplication()
{
    if (boardController) delete boardController;
    boardController = NULL;

    delete mainWindow;
    mainWindow = 0;

    if (applicationController) delete applicationController;
    applicationController = NULL;
}

int UBApplication::exec(const QString& pFileToImport)
{
    mainWindow = new UBMainWindow(0, Qt::FramelessWindowHint); // deleted by application destructor
    mainWindow->setAttribute(Qt::WA_NativeWindow, true);

    mainWindow->actionCopy->setShortcuts(QKeySequence::Copy);
    mainWindow->actionPaste->setShortcuts(QKeySequence::Paste);
    mainWindow->actionCut->setShortcuts(QKeySequence::Cut);

    connect(mainWindow->actionBoard, SIGNAL(triggered()), this, SLOT(showBoard()));
    //connect(mainWindow->actionWeb, SIGNAL(triggered()), this, SLOT(showInternet()));
    //connect(mainWindow->actionWeb, SIGNAL(triggered()), this, SLOT(stopScript()));
    connect(mainWindow->actionDocument, SIGNAL(triggered()), this, SLOT(showDocument()));
    //connect(mainWindow->actionDocument, SIGNAL(triggered()), this, SLOT(stopScript()));
    //connect(mainWindow->actionQuit, SIGNAL(triggered()), this, SLOT(closing()));
    //connect(mainWindow, SIGNAL(closeEvent_Signal(QCloseEvent*)), this, SLOT(closeEvent(QCloseEvent*)));

    boardController = new UBBoardController(mainWindow);
    boardController->init();
    applicationController = new UBApplicationController(boardController->controlView(),
                                                        boardController->displayView(),
                                                        mainWindow,
                                                        nullptr,
                                                        nullptr);
                                                        //boardController->paletteManager()->rightPalette());

    connect(applicationController, SIGNAL(mainModeChanged(UBApplicationController::MainMode)),
            boardController->paletteManager(), SLOT(slot_changeMainMode(UBApplicationController::MainMode)));

    connect(applicationController, SIGNAL(desktopMode(bool)),
            boardController->paletteManager(), SLOT(slot_changeDesktopMode(bool)));

    connect(applicationController, SIGNAL(mainModeChanged(UBApplicationController::MainMode))
          , boardController,       SLOT(appMainModeChanged(UBApplicationController::MainMode)));

    connect(mainWindow->actionDesktop, SIGNAL(triggered(bool)), applicationController, SLOT(showDesktop(bool)));
    //connect(mainWindow->actionDesktop, SIGNAL(triggered(bool)), this, SLOT(stopScript()));
#if defined(Q_OS_OSX) || defined(Q_OS_LINUX)
    connect(mainWindow->actionHideApplication, SIGNAL(triggered()), this, SLOT(showMinimized()));
#else
    connect(mainWindow->actionHideApplication, SIGNAL(triggered()), mainWindow, SLOT(showMinimized()));
#endif

    connect(mainWindow->actionCheckUpdate, SIGNAL(triggered()), applicationController, SLOT(checkUpdateRequest()));
    connect(mainWindow->actionMultiScreen, SIGNAL(triggered(bool)), applicationController, SLOT(useMultiScreen(bool)));
    connect(mainWindow->actionWidePageSize, SIGNAL(triggered(bool)), boardController, SLOT(setWidePageSize(bool)));
    connect(mainWindow->actionRegularPageSize, SIGNAL(triggered(bool)), boardController, SLOT(setRegularPageSize(bool)));

    connect(mainWindow->actionCut, SIGNAL(triggered()), applicationController, SLOT(actionCut()));
    connect(mainWindow->actionCopy, SIGNAL(triggered()), applicationController, SLOT(actionCopy()));
    connect(mainWindow->actionPaste, SIGNAL(triggered()), applicationController, SLOT(actionPaste()));

    bool const bUseMultiScreen = true;
    applicationController->initScreenLayout(bUseMultiScreen);
    boardController->setupLayout();

    if (pFileToImport.length() > 0)
        UBApplication::applicationController->importFile(pFileToImport);

    //if (UBSettings::settings()->appStartMode->get().toInt())
    applicationController->showDesktop();
    //else
    //    applicationController->showBoard();

    onScreenCountChanged(1);
    return QApplication::exec();
}

void UBApplication::onScreenCountChanged(int newCount)
{
    Q_UNUSED(newCount);
    UBDisplayManager displayManager;
    mainWindow->actionMultiScreen->setEnabled(displayManager.numScreens() > 1);
}

void UBApplication::showMinimized()
{
#ifdef Q_OS_OSX
    mainWindow->hide();
    bIsMinimized = true;
#elif defined(Q_OS_LINUX)
    mainWindow->showMinimized();
    bIsMinimized = true;
#endif

}
void UBApplication::showBoard()
{
    applicationController->showBoard();
}
void UBApplication::showDocument()
{
    applicationController->showDocument();
}
bool UBApplication::eventFilter(QObject *obj, QEvent *event)
{
    bool result = QObject::eventFilter(obj, event);

    if (event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *fileToOpenEvent = static_cast<QFileOpenEvent *>(event);

        UBPlatformUtils::setFrontProcess();

        applicationController->importFile(fileToOpenEvent->file());
    }

    if (event->type() == QEvent::TabletLeaveProximity)
    {
        if (boardController && boardController->controlView())
            boardController->controlView()->forcedTabletRelease();
    }


    if (event->type() == QEvent::ApplicationActivate)
    {
        boardController->controlView()->setMultiselection(false);

#if defined(Q_OS_OSX)
        if (bIsMinimized) {
            if (mainWindow->isHidden())
                mainWindow->show();
            bIsMinimized = false;
        }
#elif defined(Q_OS_LINUX)
        if (bIsMinimized) {
            bIsMinimized = false;
            UBPlatformUtils::showFullScreen(mainWindow);
        }
#endif
    }

    return result;
}
