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

#include "webpage.h"
#include "webpopupwindow.h"
#include "webview.h"
#include <QAction>
#include <QIcon>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWindow>

WebPopupWindow::WebPopupWindow(QWebEngineProfile *profile)
    : m_urlLineEdit(new QLineEdit(this))
    , m_favAction(new QAction(this))
    , m_view(new WebView(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(m_urlLineEdit);
    layout->addWidget(m_view);

    m_view->setPage(new WebPage(profile, m_view));
    m_view->setFocus();

    m_urlLineEdit->setReadOnly(true);
    m_urlLineEdit->addAction(m_favAction, QLineEdit::LeadingPosition);

    connect(m_view, &WebView::titleChanged, this, &QWidget::setWindowTitle);
    connect(m_view, &WebView::urlChanged, [this](const QUrl &url) {
        m_urlLineEdit->setText(url.toDisplayString());
    });
    connect(m_view, &WebView::favIconChanged, m_favAction, &QAction::setIcon);
    connect(m_view->page(), &WebPage::geometryChangeRequested, this, &WebPopupWindow::handleGeometryChangeRequested);
    connect(m_view->page(), &WebPage::windowCloseRequested, this, &QWidget::close);
}

WebView *WebPopupWindow::view() const
{
    return m_view;
}

void WebPopupWindow::handleGeometryChangeRequested(const QRect &newGeometry)
{
    if (QWindow *window = windowHandle())
        setGeometry(newGeometry.marginsRemoved(window->frameMargins()));
    show();
    m_view->setFocus();
}
