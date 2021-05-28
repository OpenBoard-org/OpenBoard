/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browserwindow.h"
#include "downloadmanagerwidget.h"
#include "tabwidget.h"
#include "webview.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWebEngineProfile>

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"

BrowserWindow::BrowserWindow(QWidget *parent, QWebEngineProfile *profile, bool forDevTools)
    : QWidget(parent)
    , m_profile(profile)
    , m_tabWidget(new TabWidget(profile, this))
    , m_progressBar(nullptr)
    , m_historyBackAction(nullptr)
    , m_historyForwardAction(nullptr)
    , m_stopAction(nullptr)
    , m_reloadAction(nullptr)
    , m_stopReloadAction(nullptr)
    , m_urlLineEdit(nullptr)
    , m_favAction(nullptr)
    , m_InspectorWindow(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setFocusPolicy(Qt::ClickFocus);

    QWidget *centralWidget = this; //new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    if (!forDevTools) {
        m_progressBar = new QProgressBar(this);

        // FIXME toolbar handling as in WBBrowserWindow
//        QToolBar *toolbar = createToolBar(this);
//        layout->addWidget(toolbar);
//        addToolBar(toolbar);
//        menuBar()->addMenu(createFileMenu(m_tabWidget));
//        menuBar()->addMenu(createEditMenu());
//        menuBar()->addMenu(createViewMenu(toolbar));
//        menuBar()->addMenu(createWindowMenu(m_tabWidget));
//        menuBar()->addMenu(createHelpMenu());
    }

    if (!forDevTools) {
//        addToolBarBreak();

        m_progressBar->setMaximumHeight(1);
        m_progressBar->setTextVisible(false);
        m_progressBar->setStyleSheet(QStringLiteral("QProgressBar {border: 0px} QProgressBar::chunk {background-color: #da4453}"));

        layout->addWidget(m_progressBar);
    }

    layout->addWidget(m_tabWidget);
    centralWidget->setLayout(layout);
//    setCentralWidget(centralWidget);



}

void BrowserWindow::init()
{
    connect(m_tabWidget, &TabWidget::titleChanged, this, &BrowserWindow::handleWebViewTitleChanged);
        // FIXME would be nice to have
//        connect(m_tabWidget, &TabWidget::linkHovered, [this](const QString& url) {
//            statusBar()->showMessage(url);
//        });
    connect(m_tabWidget, &TabWidget::loadProgress, this, &BrowserWindow::handleWebViewLoadProgress);
    connect(m_tabWidget, &TabWidget::webActionEnabledChanged, this, &BrowserWindow::handleWebActionEnabledChanged);
    connect(m_tabWidget, &TabWidget::urlChanged, [this](const QUrl &url) {
        m_urlLineEdit->setText(url.toDisplayString());
    });
    connect(m_tabWidget, &TabWidget::favIconChanged, m_favAction, &QAction::setIcon);
    connect(m_tabWidget, &TabWidget::devToolsRequested, this, &BrowserWindow::handleDevToolsRequested);
    connect(m_urlLineEdit, &QLineEdit::returnPressed, this, &BrowserWindow::handleReturnPressed);

    QAction *focusUrlLineEditAction = new QAction(this);
    addAction(focusUrlLineEditAction);
    focusUrlLineEditAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(focusUrlLineEditAction, &QAction::triggered, this, [this] () {
        m_urlLineEdit->setFocus(Qt::ShortcutFocusReason);
    });

    handleWebViewTitleChanged(QString());
    connect(m_tabWidget, &TabWidget::currentChanged, [this](int index){
        QWidget* current = m_tabWidget->widget(index);

        emit activeViewChange(current);
        emit activeViewPageChanged();
    });
    connect(m_tabWidget, &TabWidget::tabClosing, this, &BrowserWindow::handleTabClosing);
    m_tabWidget->createTab();
}

QSize BrowserWindow::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * qreal(0.9);
    return size;
}

// FIXME remove, no menus at all
QMenu *BrowserWindow::createFileMenu(TabWidget *tabWidget)
{
    QMenu *fileMenu = new QMenu(tr("&File"));
    fileMenu->addAction(tr("&New Window"), this, &BrowserWindow::handleNewWindowTriggered, QKeySequence::New);
    fileMenu->addAction(tr("New &Incognito Window"), this, &BrowserWindow::handleNewIncognitoWindowTriggered);

    QAction *newTabAction = new QAction(tr("New &Tab"), this);
    newTabAction->setShortcuts(QKeySequence::AddTab);
    connect(newTabAction, &QAction::triggered, this, [this]() {
        m_tabWidget->createTab();
        m_urlLineEdit->setFocus();
    });
    fileMenu->addAction(newTabAction);

    fileMenu->addAction(tr("&Open File..."), this, &BrowserWindow::handleFileOpenTriggered, QKeySequence::Open);
    fileMenu->addSeparator();

    QAction *closeTabAction = new QAction(tr("&Close Tab"), this);
    closeTabAction->setShortcuts(QKeySequence::Close);
    connect(closeTabAction, &QAction::triggered, [tabWidget]() {
        tabWidget->closeTab(tabWidget->currentIndex());
    });
    fileMenu->addAction(closeTabAction);

    QAction *closeAction = new QAction(tr("&Quit"),this);
    closeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(closeAction);

//    connect(fileMenu, &QMenu::aboutToShow, [this, closeAction]() {
//        if (m_browser->windows().count() == 1)
//            closeAction->setText(tr("&Quit"));
//        else
//            closeAction->setText(tr("&Close Window"));
//    });
    return fileMenu;
}

QMenu *BrowserWindow::createEditMenu()
{
    QMenu *editMenu = new QMenu(tr("&Edit"));
    QAction *findAction = editMenu->addAction(tr("&Find"));
    findAction->setShortcuts(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &BrowserWindow::handleFindActionTriggered);

    QAction *findNextAction = editMenu->addAction(tr("Find &Next"));
    findNextAction->setShortcut(QKeySequence::FindNext);
    connect(findNextAction, &QAction::triggered, [this]() {
        if (!currentTab() || m_lastSearch.isEmpty())
            return;
        currentTab()->findText(m_lastSearch);
    });

    QAction *findPreviousAction = editMenu->addAction(tr("Find &Previous"));
    findPreviousAction->setShortcut(QKeySequence::FindPrevious);
    connect(findPreviousAction, &QAction::triggered, [this]() {
        if (!currentTab() || m_lastSearch.isEmpty())
            return;
        currentTab()->findText(m_lastSearch, QWebEnginePage::FindBackward);
    });

    return editMenu;
}

QMenu *BrowserWindow::createViewMenu(QToolBar *toolbar)
{
    QMenu *viewMenu = new QMenu(tr("&View"));
    m_stopAction = viewMenu->addAction(tr("&Stop"));
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Period));
    shortcuts.append(Qt::Key_Escape);
    m_stopAction->setShortcuts(shortcuts);
    connect(m_stopAction, &QAction::triggered, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Stop);
    });

    m_reloadAction = viewMenu->addAction(tr("Reload Page"));
    m_reloadAction->setShortcuts(QKeySequence::Refresh);
    connect(m_reloadAction, &QAction::triggered, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Reload);
    });

    QAction *zoomIn = viewMenu->addAction(tr("Zoom &In"));
    zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    connect(zoomIn, &QAction::triggered, [this]() {
        if (currentTab())
            currentTab()->setZoomFactor(currentTab()->zoomFactor() + 0.1);
    });

    QAction *zoomOut = viewMenu->addAction(tr("Zoom &Out"));
    zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    connect(zoomOut, &QAction::triggered, [this]() {
        if (currentTab())
            currentTab()->setZoomFactor(currentTab()->zoomFactor() - 0.1);
    });

    QAction *resetZoom = viewMenu->addAction(tr("Reset &Zoom"));
    resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(resetZoom, &QAction::triggered, [this]() {
        if (currentTab())
            currentTab()->setZoomFactor(1.0);
    });


    viewMenu->addSeparator();
    QAction *viewToolbarAction = new QAction(tr("Hide Toolbar"),this);
    viewToolbarAction->setShortcut(tr("Ctrl+|"));
    connect(viewToolbarAction, &QAction::triggered, [toolbar,viewToolbarAction]() {
        if (toolbar->isVisible()) {
            viewToolbarAction->setText(tr("Show Toolbar"));
            toolbar->close();
        } else {
            viewToolbarAction->setText(tr("Hide Toolbar"));
            toolbar->show();
        }
    });
    viewMenu->addAction(viewToolbarAction);

    QAction *viewStatusbarAction = new QAction(tr("Hide Status Bar"), this);
    viewStatusbarAction->setShortcut(tr("Ctrl+/"));
//    connect(viewStatusbarAction, &QAction::triggered, [this, viewStatusbarAction]() {
//        if (statusBar()->isVisible()) {
//            viewStatusbarAction->setText(tr("Show Status Bar"));
//            statusBar()->close();
//        } else {
//            viewStatusbarAction->setText(tr("Hide Status Bar"));
//            statusBar()->show();
//        }
//    });
    viewMenu->addAction(viewStatusbarAction);
    return viewMenu;
}

QMenu *BrowserWindow::createWindowMenu(TabWidget *tabWidget)
{
    QMenu *menu = new QMenu(tr("&Window"));

    QAction *nextTabAction = new QAction(tr("Show Next Tab"), this);
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Less));
    nextTabAction->setShortcuts(shortcuts);
    connect(nextTabAction, &QAction::triggered, tabWidget, &TabWidget::nextTab);

    QAction *previousTabAction = new QAction(tr("Show Previous Tab"), this);
    shortcuts.clear();
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Greater));
    previousTabAction->setShortcuts(shortcuts);
    connect(previousTabAction, &QAction::triggered, tabWidget, &TabWidget::previousTab);

    connect(menu, &QMenu::aboutToShow, [this, menu, nextTabAction, previousTabAction]() {
        menu->clear();
        menu->addAction(nextTabAction);
        menu->addAction(previousTabAction);
        menu->addSeparator();

//        QVector<BrowserWindow*> windows = m_browser->windows();
//        int index(-1);
//        for (auto window : windows) {
//            QAction *action = menu->addAction(window->windowTitle(), this, &BrowserWindow::handleShowWindowTriggered);
//            action->setData(++index);
//            action->setCheckable(true);
//            if (window == this)
//                action->setChecked(true);
//        }
    });
    return menu;
}

QMenu *BrowserWindow::createHelpMenu()
{
    QMenu *helpMenu = new QMenu(tr("&Help"));
    helpMenu->addAction(tr("About &Qt"), qApp, QApplication::aboutQt);
    return helpMenu;
}

QToolBar *BrowserWindow::createToolBar(QWidget *parent)
{
    QToolBar *navigationBar = new QToolBar(tr("Navigation"));
    navigationBar->setMovable(false);
    navigationBar->toggleViewAction()->setEnabled(false);

// FIXME remove m_historyBackAction
//    m_historyBackAction = new QAction(this);
//    QList<QKeySequence> backShortcuts = QKeySequence::keyBindings(QKeySequence::Back);
//    for (auto it = backShortcuts.begin(); it != backShortcuts.end();) {
//        // Chromium already handles navigate on backspace when appropriate.
//        if ((*it)[0] == Qt::Key_Backspace)
//            it = backShortcuts.erase(it);
//        else
//            ++it;
//    }
//    // For some reason Qt doesn't bind the dedicated Back key to Back.
//    backShortcuts.append(QKeySequence(Qt::Key_Back));
//    m_historyBackAction->setShortcuts(backShortcuts);
//    m_historyBackAction->setIconVisibleInMenu(false);
//    m_historyBackAction->setIcon(QIcon(QStringLiteral(":go-previous.png")));
//    m_historyBackAction->setToolTip(tr("Go back in history"));
//    connect(m_historyBackAction, &QAction::triggered, [this]() {
//        m_tabWidget->triggerWebPageAction(QWebEnginePage::Back);
//    });
//    navigationBar->addAction(m_historyBackAction);

// FIXME remove m_historyForwardAction
//    m_historyForwardAction = new QAction(this);
//    QList<QKeySequence> fwdShortcuts = QKeySequence::keyBindings(QKeySequence::Forward);
//    for (auto it = fwdShortcuts.begin(); it != fwdShortcuts.end();) {
//        if (((*it)[0] & Qt::Key_unknown) == Qt::Key_Backspace)
//            it = fwdShortcuts.erase(it);
//        else
//            ++it;
//    }
//    fwdShortcuts.append(QKeySequence(Qt::Key_Forward));
//    m_historyForwardAction->setShortcuts(fwdShortcuts);
//    m_historyForwardAction->setIconVisibleInMenu(false);
//    m_historyForwardAction->setIcon(QIcon(QStringLiteral(":go-next.png")));
//    m_historyForwardAction->setToolTip(tr("Go forward in history"));
//    connect(m_historyForwardAction, &QAction::triggered, [this]() {
//        m_tabWidget->triggerWebPageAction(QWebEnginePage::Forward);
//    });
//    navigationBar->addAction(m_historyForwardAction);

    // FIXME remove m_stopReloadAction
//    m_stopReloadAction = new QAction(this);
//    connect(m_stopReloadAction, &QAction::triggered, [this]() {
//        m_tabWidget->triggerWebPageAction(QWebEnginePage::WebAction(m_stopReloadAction->data().toInt()));
//    });
//    navigationBar->addAction(m_stopReloadAction);

    m_urlLineEdit = new QLineEdit(parent);
    m_favAction = new QAction(parent);
    m_urlLineEdit->addAction(m_favAction, QLineEdit::LeadingPosition);
    m_urlLineEdit->setClearButtonEnabled(true);
    navigationBar->addWidget(m_urlLineEdit);

    auto downloadsAction = new QAction(parent);
    downloadsAction->setIcon(QIcon(QStringLiteral(":go-bottom.png")));
    downloadsAction->setToolTip(tr("Show downloads"));
    navigationBar->addAction(downloadsAction);
    connect(downloadsAction, &QAction::triggered, [this]() {
        DownloadManagerWidget* downloadManager = findChild<DownloadManagerWidget*>();

        if (downloadManager)
        {
            downloadManager->show();
        }
    });

    return navigationBar;
}

void BrowserWindow::handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled)
{
    // FIXME
//    switch (action) {
//    case QWebEnginePage::Back:
//        m_historyBackAction->setEnabled(enabled);
//        break;
//    case QWebEnginePage::Forward:
//        m_historyForwardAction->setEnabled(enabled);
//        break;
//    case QWebEnginePage::Reload:
//        m_reloadAction->setEnabled(enabled);
//        break;
//    case QWebEnginePage::Stop:
//        m_stopAction->setEnabled(enabled);
//        break;
//    default:
//        qWarning("Unhandled webActionChanged signal");
//    }
}

void BrowserWindow::handleWebViewTitleChanged(const QString &title)
{
    QString suffix = m_profile->isOffTheRecord()
        ? tr("Qt Simple Browser (Incognito)")
        : tr("Qt Simple Browser");

    if (title.isEmpty())
        setWindowTitle(suffix);
    else
        setWindowTitle(title + " - " + suffix);
}

void BrowserWindow::handleNewWindowTriggered()
{
    // FIXME never used
//    BrowserWindow *window = m_browser->createWindow();
//    window->m_urlLineEdit->setFocus();
}

void BrowserWindow::handleNewIncognitoWindowTriggered()
{
    // FIXME never used
//    BrowserWindow *window = m_browser->createWindow(/* offTheRecord: */ true);
//    window->m_urlLineEdit->setFocus();
}

void BrowserWindow::handleFileOpenTriggered()
{
    QUrl url = QFileDialog::getOpenFileUrl(this, tr("Open Web Resource"), QString(),
                                                tr("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)"));
    if (url.isEmpty())
        return;
    currentTab()->setUrl(url);
}

void BrowserWindow::handleFindActionTriggered()
{
    if (!currentTab())
        return;
    bool ok = false;
    QString search = QInputDialog::getText(this, tr("Find"),
                                           tr("Find:"), QLineEdit::Normal,
                                           m_lastSearch, &ok);
    if (ok && !search.isEmpty()) {
        m_lastSearch = search;
        currentTab()->findText(m_lastSearch, 0, [this](bool found) {
//            if (!found)
//                statusBar()->showMessage(tr("\"%1\" not found.").arg(m_lastSearch));
        });
    }
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    if (m_tabWidget->count() > 1) {
        int ret = QMessageBox::warning(this, tr("Confirm close"),
                                       tr("Are you sure you want to close the window ?\n"
                                          "There are %1 tabs open.").arg(m_tabWidget->count()),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    event->accept();
//    deleteLater(); deleted by parent
}

void BrowserWindow::zoomIn()
{
    WebView* webView = m_tabWidget->currentWebView();

    if (!webView)
        return;

    webView->setZoomFactor(webView->zoomFactor() + 0.1);
}

void BrowserWindow::zoomOut()
{
    WebView* webView = m_tabWidget->currentWebView();

    if (!webView)
        return;

    webView->setZoomFactor(webView->zoomFactor() - 0.1);
}

TabWidget *BrowserWindow::tabWidget() const
{
    return m_tabWidget;
}

WebView *BrowserWindow::currentTab() const
{
    return m_tabWidget->currentWebView();
}

void BrowserWindow::handleWebViewLoadProgress(int progress)
{
    static QIcon stopIcon(QStringLiteral(":process-stop.png"));
    static QIcon reloadIcon(QStringLiteral(":view-refresh.png"));

    if (0 < progress && progress < 100) {
        if (m_stopReloadAction)
        {
            m_stopReloadAction->setData(QWebEnginePage::Stop);
            m_stopReloadAction->setIcon(stopIcon);
            m_stopReloadAction->setToolTip(tr("Stop loading the current page"));
        }
        m_progressBar->setValue(progress);
    } else {
        if (m_stopReloadAction)
        {
            m_stopReloadAction->setData(QWebEnginePage::Reload);
            m_stopReloadAction->setIcon(reloadIcon);
            m_stopReloadAction->setToolTip(tr("Reload the current page"));
        }
        m_progressBar->setValue(0);
    }
}

void BrowserWindow::handleShowWindowTriggered()
{
//    if (QAction *action = qobject_cast<QAction*>(sender())) {
//        int offset = action->data().toInt();
//        QVector<BrowserWindow*> windows = m_browser->windows();
//        windows.at(offset)->activateWindow();
//        windows.at(offset)->currentTab()->setFocus();
//    }
}

void BrowserWindow::handleDevToolsRequested(QWebEnginePage *source)
{
    // use same mechanism as for the widget apps
    if (m_InspectorWindow)
    {
        QWebEngineView *inspector = qobject_cast<QWebEngineView*>(m_InspectorWindow->centralWidget());

        if (inspector && inspector->page()->inspectedPage() != source) {
            source->setDevToolsPage(inspector->page());
        }

        m_InspectorWindow->activateWindow();
    }
    else
    {
        QRect controlGeometry = UBApplication::applicationController->displayManager()->controlGeometry();
        QRect inspectorGeometry(controlGeometry.left() + 50, controlGeometry.top() + 50, controlGeometry.width() / 2, controlGeometry.height() / 2);

        m_InspectorWindow = new QMainWindow();
        m_InspectorWindow->setAttribute(Qt::WA_DeleteOnClose, true);
        m_InspectorWindow->setFocusPolicy(Qt::ClickFocus);

        QWebEngineView *inspector = new QWebEngineView();
        m_InspectorWindow->setCentralWidget(inspector);
        m_InspectorWindow->setGeometry(inspectorGeometry);
        m_InspectorWindow->show();

        source->setDevToolsPage(inspector->page());
        source->triggerAction(QWebEnginePage::InspectElement);

        connect(m_InspectorWindow, &QObject::destroyed, [this](){
            m_InspectorWindow = nullptr;
        });
    }
}

void BrowserWindow::handleTabClosing(WebView* webView)
{
    // close inspector window if it is attached to the source page
    if (webView && m_InspectorWindow)
    {
        QWebEnginePage* source = webView->page();
        QWebEngineView *inspector = qobject_cast<QWebEngineView*>(m_InspectorWindow->centralWidget());

        if (inspector && inspector->page()->inspectedPage() == source)
        {
            m_InspectorWindow->close();
            m_InspectorWindow->deleteLater();
            m_InspectorWindow = nullptr;
        }
    }
}

void BrowserWindow::handleReturnPressed()
{
    QString input = m_urlLineEdit->text().trimmed();

    // get first word
    int wordEnd = input.indexOf(QRegExp("\\s"));
    QString firstWord = wordEnd < 0 ? input : input.left(wordEnd);
    bool search = false;

    if (firstWord.startsWith("?"))
    {
        // ? as marker for search
        search = true;
        input = input.mid(1);
    }
    else if (firstWord.indexOf(".") < 0)
    {
        // search if first work does not contain a dot
        search = true;
    }

    QUrl url = QUrl::fromUserInput(input);

    if (search || !url.isValid())
    {
        QString searchEngine = UBSettings::settings()->webSearchEngineUrl->get().toString();
        input = QUrl::toPercentEncoding(input);
        url = QUrl::fromUserInput(searchEngine.arg(input));
    }

    m_tabWidget->setUrl(url);
}
