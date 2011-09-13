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

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "board/UBBoardController.h"

#include "core/memcheck.h"

UBMainWindow::UBMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , mBoardWidget(0)
    , mWebWidget(0)
    , mDocumentsWidget(0)
{
    Ui::MainWindow::setupUi(this);

    QWidget* centralWidget = new QWidget(this);
    mStackedLayout = new QStackedLayout(centralWidget);
    setCentralWidget(centralWidget);

#ifdef Q_WS_MAC
    actionPreferences->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Comma));
    actionQuit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
#elif defined(Q_WS_WIN)
    actionPreferences->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Return));
    // this code, because it unusable, system key combination can`t be triggered, even we add it manually
    actionQuit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
#else
    actionQuit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
#endif
}

UBMainWindow::~UBMainWindow()
{
    // NOOP
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
    qDebug() << "add to StackedLayout size height: " << pWidget->height() << " width: " << pWidget->width();
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

    /*
    if (event->key() == Qt::Key_B && !event->isAccepted())
    {
        UBApplication::applicationController->blackout();
        event->accept();
    }
    */
}

void UBMainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    emit closeEvent_Signal(event);
}

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
    messageBox.setWindowFlags(Qt::Dialog);
    messageBox.setWindowTitle(windowTitle);
    messageBox.setText(text);
    QPushButton* yesButton = messageBox.addButton(tr("Yes"),QMessageBox::YesRole);
    messageBox.addButton(tr("No"),QMessageBox::NoRole);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.exec();

    return messageBox.clickedButton() == yesButton;
}
