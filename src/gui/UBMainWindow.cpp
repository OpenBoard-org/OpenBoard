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
#include <QToolTip>
#include <QStackedLayout>

#include "UBMainWindow.h"
#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "board/UBBoardController.h"
#include "core/UBDisplayManager.h"

// work around for handling tablet events on MAC OS with Qt 4.8.0 and above
#if defined(Q_OS_OSX)
#include "board/UBBoardView.h"
#endif

#include "core/memcheck.h"

UBMainWindow::UBMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , mBoardWidget(0)
    , mWebWidget(0)
    , mDocumentsWidget(0)
    , mpDownloadWidget(NULL)
{
    Ui::MainWindow::setupUi(this);

    mpDownloadWidget = new UBDownloadWidget();
    mpDownloadWidget->setWindowModality(Qt::ApplicationModal);

    //Setting tooltip colors staticly, since they look not quite well on different color themes
    QPalette toolTipPalette;
    toolTipPalette.setColor(QPalette::ToolTipBase, QColor("#FFFFDC"));
    toolTipPalette.setColor(QPalette::ToolTipText, Qt::black);
    QToolTip::setPalette(toolTipPalette);

    QWidget* centralWidget = new QWidget(this);
    mStackedLayout = new QStackedLayout(centralWidget);
    setCentralWidget(centralWidget);

#ifdef Q_OS_OSX
    actionPreferences->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Comma));
    actionQuit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
#elif defined(Q_OS_WIN)
    actionPreferences->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
    // this code, because it unusable, system key combination can`t be triggered, even we add it manually
    actionQuit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
#else
    actionQuit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
#endif
}

UBMainWindow::~UBMainWindow()
{
    if(NULL != mpDownloadWidget)
    {
        delete mpDownloadWidget;
        mpDownloadWidget = NULL;
    }
}

void UBMainWindow::addBoardWidget(QWidget *pWidget)
{
    if (!mBoardWidget)
    {
        mBoardWidget = pWidget;
        mStackedLayout->addWidget(mBoardWidget);
    }
}

void UBMainWindow::switchToBoardWidget()
{
    if (mBoardWidget)
    {
        mStackedLayout->setCurrentWidget(mBoardWidget);
    }
}

void UBMainWindow::addWebWidget(QWidget *pWidget)
{
    if (!mWebWidget)
    {
        mWebWidget = pWidget;
        mStackedLayout->addWidget(mWebWidget);
    }
}

void UBMainWindow::switchToWebWidget()
{
    qDebug() << "popped out from StackedLayout size height: " << mWebWidget->height() << " width: " << mWebWidget->width();
    if (mWebWidget)
    {
        mStackedLayout->setCurrentWidget(mWebWidget);
    }
}


void UBMainWindow::addDocumentsWidget(QWidget *pWidget)
{
    if (!mDocumentsWidget)
    {
        mDocumentsWidget = pWidget;
        mStackedLayout->addWidget(mDocumentsWidget);
    }
}

void UBMainWindow::switchToDocumentsWidget()
{
    if (mDocumentsWidget)
    {
        mStackedLayout->setCurrentWidget(mDocumentsWidget);
    }
}

void UBMainWindow::keyPressEvent(QKeyEvent *event)
{
    QMainWindow::keyPressEvent(event);
}

void UBMainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    emit closeEvent_Signal(event);
}

// work around for handling tablet events on MAC OS with Qt 4.8.0 and above
#if defined(Q_OS_OSX)
bool UBMainWindow::event(QEvent *event)
{
    bool bRes = QMainWindow::event(event);

    if (NULL != UBApplication::boardController)
    {
        UBBoardView *controlV = UBApplication::boardController->controlView();
        if (controlV && controlV->isVisible())
        {
            switch (event->type())
            {
            case QEvent::TabletEnterProximity:
            case QEvent::TabletLeaveProximity:
            case QEvent::TabletMove:
            case QEvent::TabletPress:
            case QEvent::TabletRelease:
                {
                    return controlV->directTabletEvent(event);
                }
            }
        }
    }
    return bRes;
}
#endif

void UBMainWindow::onExportDone()
{
    // HACK :  When opening the file save dialog during the document exportation,
    //         some buttons of the toolbar become disabled without any reason. We
    //         re-enable them here.
    actionExport->setEnabled(true);
    actionNewDocument->setEnabled(true);
    actionRename->setEnabled(true);
    actionDuplicate->setEnabled(true);
    actionDelete->setEnabled(true);
    actionOpen->setEnabled(true);
    actionDocumentAdd->setEnabled(true);
}

bool UBMainWindow::yesNoQuestion(QString windowTitle, QString text)
{
    QMessageBox messageBox;
    messageBox.setParent(this);
    messageBox.setWindowTitle(windowTitle);
    messageBox.setText(text);
    QPushButton* yesButton = messageBox.addButton(tr("Yes"),QMessageBox::YesRole);
    messageBox.addButton(tr("No"),QMessageBox::NoRole);
    messageBox.setIcon(QMessageBox::Question);

#ifdef Q_OS_LINUX
    // to avoid to be handled by x11. This allows us to keep to the back all the windows manager stuff like palette, toolbar ...
    messageBox.setWindowFlags(Qt::Dialog | Qt::X11BypassWindowManagerHint);
#else
    messageBox.setWindowFlags(Qt::Dialog);
#endif

    messageBox.exec();
    return messageBox.clickedButton() == yesButton;
}

void UBMainWindow::oneButtonMessageBox(QString windowTitle, QString text, QMessageBox::Icon type)
{
    QMessageBox messageBox;
    messageBox.setParent(this);
    messageBox.setWindowFlags(Qt::Dialog);
    messageBox.setWindowTitle(windowTitle);
    messageBox.setText(text);
    messageBox.addButton(tr("Ok"),QMessageBox::YesRole);
    messageBox.setIcon(type);
    messageBox.exec();
}

void UBMainWindow::warning(QString windowTitle, QString text)
{
    oneButtonMessageBox(windowTitle,text, QMessageBox::Warning);
}

void UBMainWindow::information(QString windowTitle, QString text)
{
    oneButtonMessageBox(windowTitle, text, QMessageBox::Information);
}

void UBMainWindow::showDownloadWidget()
{
    if(NULL != mpDownloadWidget)
    {
        mpDownloadWidget->show();
    }
}

void UBMainWindow::hideDownloadWidget()
{
    if(NULL != mpDownloadWidget)
    {
        mpDownloadWidget->hide();
    }
}
