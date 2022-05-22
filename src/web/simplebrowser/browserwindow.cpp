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
#include "WBHistory.h"

#include <QApplication>
#include <QCloseEvent>
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
    , m_tabWidget(new TabWidget(profile, this))
    , m_progressBar(nullptr)
    , m_urlLineEdit(nullptr)
    , m_favAction(nullptr)
    , m_inspectorWindow(nullptr)
    , m_historyManager(nullptr)
    , m_offTheRecord(profile->isOffTheRecord())
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setFocusPolicy(Qt::ClickFocus);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    if (!forDevTools) {
        m_progressBar = new QProgressBar(this);
        m_progressBar->setMaximumHeight(1);
        m_progressBar->setTextVisible(false);
        m_progressBar->setStyleSheet(QStringLiteral("QProgressBar {border: 0px} QProgressBar::chunk {background-color: #da4453}"));

        layout->addWidget(m_progressBar);
    }

    layout->addWidget(m_tabWidget);
    setLayout(layout);
}

void BrowserWindow::init()
{
    m_statusBar = new QStatusBar(this);
    m_statusBar->setWindowFlag(Qt::ToolTip);
    m_statusBar->setVisible(true);

    connect(m_tabWidget, &TabWidget::titleChanged, this, &BrowserWindow::handleWebViewTitleChanged);
    connect(m_tabWidget, &TabWidget::linkHovered, [this](const QString& url) {
        m_statusBar->setVisible(!url.isEmpty());
        m_statusBar->showMessage(url);
    });
    connect(m_tabWidget, &TabWidget::loadProgress, this, &BrowserWindow::handleWebViewLoadProgress);
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
    QSize size = UBApplication::displayManager->screenSize(ScreenRole::Control);
    return size * qreal(0.9);
}

QToolBar *BrowserWindow::createToolBar(QWidget *parent)
{
    QToolBar *navigationBar = new QToolBar(tr("Navigation"));
    navigationBar->setMovable(false);
    navigationBar->toggleViewAction()->setEnabled(false);

    m_urlLineEdit = new QLineEdit(parent);
    m_favAction = new QAction(parent);
    m_urlLineEdit->addAction(m_favAction, QLineEdit::LeadingPosition);
    m_urlLineEdit->setClearButtonEnabled(true);
    navigationBar->addWidget(m_urlLineEdit);

    // add QCompleter
    WBHistoryCompletionModel *completionModel = new WBHistoryCompletionModel(this);
    completionModel->setSourceModel(historyManager()->historyFilterModel());
    m_lineEditCompleter = new QCompleter(completionModel, this);
    m_lineEditCompleter->setFilterMode(Qt::MatchContains);
    // Should this be in Qt by default?
    QAbstractItemView *popup = m_lineEditCompleter->popup();
    QListView *listView = qobject_cast<QListView*>(popup);

    if (listView)
    {
        listView->setUniformItemSizes(true);
    }

    m_urlLineEdit->setCompleter(m_lineEditCompleter);

    auto downloadsAction = new QAction(parent);
    downloadsAction->setIcon(QIcon(QStringLiteral(":webbrowser/go-bottom.png")));
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

void BrowserWindow::handleWebViewTitleChanged(const QString &title)
{
    if (title.isEmpty())
        setWindowTitle("");
    else
        setWindowTitle(title);

    if (!title.isEmpty() && currentTab() && !m_offTheRecord)
    {
        historyManager()->updateHistoryItem(currentTab()->url(), title);
    }
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void BrowserWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    QRect geo = geometry();
    geo.moveTopLeft(parentWidget()->mapToGlobal(geo.topLeft()));
    QRect statusGeo(geo.left(), geo.bottom() - 20, geo.width() / 3, 20);
    m_statusBar->setGeometry(statusGeo);
}

void BrowserWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_statusBar->show();
}

void BrowserWindow::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_statusBar->hide();
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

WBHistoryManager *BrowserWindow::historyManager()
{
    if (!m_historyManager) {
        m_historyManager = new WBHistoryManager(this);
    }

    return m_historyManager;
}

void BrowserWindow::handleWebViewLoadProgress(int progress)
{
    static QIcon stopIcon(QStringLiteral(":process-stop.png"));
    static QIcon reloadIcon(QStringLiteral(":view-refresh.png"));

    if (0 < progress && progress < 100) {
        m_progressBar->setValue(progress);
    } else {
        m_progressBar->setValue(0);
    }
}

void BrowserWindow::handleDevToolsRequested(QWebEnginePage *source)
{
    // use same mechanism as for the widget apps
    if (m_inspectorWindow)
    {
        QWebEngineView *inspector = qobject_cast<QWebEngineView*>(m_inspectorWindow->centralWidget());

        if (inspector && inspector->page()->inspectedPage() != source) {
            source->setDevToolsPage(inspector->page());
        }

        m_inspectorWindow->activateWindow();
    }
    else
    {
        QRect controlGeometry = UBApplication::displayManager->screenGeometry(ScreenRole::Control);
        QRect inspectorGeometry(controlGeometry.left() + 50, controlGeometry.top() + 50, controlGeometry.width() / 2, controlGeometry.height() / 2);

        m_inspectorWindow = new QMainWindow();
        m_inspectorWindow->setAttribute(Qt::WA_DeleteOnClose, true);
        m_inspectorWindow->setFocusPolicy(Qt::ClickFocus);

        QWebEngineView *inspector = new QWebEngineView();
        m_inspectorWindow->setCentralWidget(inspector);
        m_inspectorWindow->setGeometry(inspectorGeometry);
        m_inspectorWindow->show();

        source->setDevToolsPage(inspector->page());
        source->triggerAction(QWebEnginePage::InspectElement);

        connect(m_inspectorWindow, &QObject::destroyed, [this](){
            m_inspectorWindow = nullptr;
        });
    }
}

void BrowserWindow::handleTabClosing(WebView* webView)
{
    // close inspector window if it is attached to the source page
    if (webView && m_inspectorWindow)
    {
        QWebEnginePage* source = webView->page();
        QWebEngineView *inspector = qobject_cast<QWebEngineView*>(m_inspectorWindow->centralWidget());

        if (inspector && inspector->page()->inspectedPage() == source)
        {
            m_inspectorWindow->close();
            m_inspectorWindow->deleteLater();
            m_inspectorWindow = nullptr;
        }
    }
}

void BrowserWindow::handleReturnPressed()
{
    QString input = m_urlLineEdit->text().trimmed();

    // get first word
    static const QRegularExpression whitespace("\\s");
    int wordEnd = input.indexOf(whitespace);
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
        // search if first word does not contain a dot
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
