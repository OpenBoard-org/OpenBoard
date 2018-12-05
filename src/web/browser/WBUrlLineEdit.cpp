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




/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "WBUrlLineEdit.h"

#include "WBSearchLineEdit.h"
#include "WBWebView.h"

#include "globals/UBGlobals.h"

#include <QtGui>
#include <QCompleter>
#include <QLineEdit>
#include <QLabel>
#include <QApplication>

#include "core/memcheck.h"

WBExLineEdit::WBExLineEdit(QWidget *parent)
    : QWidget(parent)
    , mLeftWidget(0)
    , mLineEdit(new QLineEdit(this))
    , mClearButton(0)
{
    setFocusPolicy(mLineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
    setSizePolicy(mLineEdit->sizePolicy());
    setBackgroundRole(mLineEdit->backgroundRole());
    setMouseTracking(true);
    setAcceptDrops(true);
    setAttribute(Qt::WA_MacShowFocusRect, true);
    QPalette p = mLineEdit->palette();
    setPalette(p);

    // line edit
    mLineEdit->setFrame(false);
    mLineEdit->setObjectName("ubWebBrowserLineEdit");
    mLineEdit->setFocusProxy(this);
    mLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    QPalette clearPalette = mLineEdit->palette();
    clearPalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    mLineEdit->setPalette(clearPalette);

    // clearButton
    mClearButton = new WBClearButton(this);
    connect(mClearButton, SIGNAL(clicked()), mLineEdit, SLOT(clear()));
    connect(mLineEdit, SIGNAL(textChanged(const QString&)), mClearButton, SLOT(textChanged(const QString&)));

    mClearButton->hide();
}

void WBExLineEdit::setLeftWidget(QWidget *widget)
{
    delete mLeftWidget;
    mLeftWidget = widget;

    updateGeometries();
}

QWidget *WBExLineEdit::leftWidget() const
{
    return mLeftWidget;
}

void WBExLineEdit::resizeEvent(QResizeEvent *event)
{
    Q_ASSERT(mLeftWidget);
    updateGeometries();
    QWidget::resizeEvent(event);
}

void WBExLineEdit::updateGeometries()
{
    QStyleOptionFrameV2 panel;
    initStyleOption(&panel);

    QRect rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

    int width = rect.width();

    mLeftWidget->setGeometry(0, -4, 32 ,32);

    mLineEdit->setGeometry(32, 0,
                            width - this->height() - 32, this->height());

    mClearButton->setGeometry(this->width() - this->height() - 2 , 0,
            this->height(), this->height());

}

void WBExLineEdit::initStyleOption(QStyleOptionFrameV2 *option) const
{
    option->initFrom(this);
    option->rect = contentsRect();
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;
    if (mLineEdit->isReadOnly())
        option->state |= QStyle::State_ReadOnly;
#ifdef QT_KEYPAD_NAVIGATION
    if (hasEditFocus())
        option->state |= QStyle::State_HasEditFocus;
#endif
    option->features = QStyleOptionFrameV2::None;
}

QSize WBExLineEdit::sizeHint() const
{
    mLineEdit->setFrame(true);
    QSize size = mLineEdit->sizeHint();
    mLineEdit->setFrame(false);
    return size;
}

void WBExLineEdit::focusInEvent(QFocusEvent *event)
{
    mLineEdit->event(event);
    QWidget::focusInEvent(event);
}

void WBExLineEdit::focusOutEvent(QFocusEvent *event)
{
    mLineEdit->event(event);

    if (mLineEdit->completer()) {
        connect(mLineEdit->completer(), SIGNAL(activated(QString)),
                         mLineEdit, SLOT(setText(QString)));
        connect(mLineEdit->completer(), SIGNAL(highlighted(QString)),
                         mLineEdit, SLOT(_q_completionHighlighted(QString)));
    }
    QWidget::focusOutEvent(event);
}

void WBExLineEdit::keyPressEvent(QKeyEvent *event)
{
    mLineEdit->event(event);
    QWidget::keyPressEvent(event);
}

bool WBExLineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
        return mLineEdit->event(event);

    return QWidget::event(event);
}


void WBExLineEdit::paintEvent(QPaintEvent *)
{
    // No default painting
}


QVariant WBExLineEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
    return mLineEdit->inputMethodQuery(property);
}


void WBExLineEdit::inputMethodEvent(QInputMethodEvent *e)
{
    mLineEdit->event(e);
}


void WBExLineEdit::setVisible(bool pVisible)
{
    QWidget::setVisible(pVisible);
    mLineEdit->setVisible(pVisible);
    mClearButton->setVisible(pVisible);
    mLeftWidget->setVisible(pVisible);
}


class WBUrlIconLabel : public QLabel
{

public:
    WBUrlIconLabel(QWidget *parent);

    WBWebView *m_webView;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QPoint m_dragStartPos;

};

WBUrlIconLabel::WBUrlIconLabel(QWidget *parent)
    : QLabel(parent)
    , m_webView(0)
{
    setMinimumWidth(16);
    setMinimumHeight(16);
}

void WBUrlIconLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStartPos = event->pos();
    QLabel::mousePressEvent(event);
}

void WBUrlIconLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton
        && (event->pos() - m_dragStartPos).manhattanLength() > QApplication::startDragDistance()
         && m_webView) {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(QString::fromUtf8(m_webView->url().toEncoded()));
        QList<QUrl> urls;
        urls.append(m_webView->url());
        mimeData->setUrls(urls);
        drag->setMimeData(mimeData);
        drag->exec();
    }
}

WBUrlLineEdit::WBUrlLineEdit(QWidget *parent)
    : WBExLineEdit(parent)
    , mWebView(0)
{
    setLeftWidget(new QWidget(this));
}


void WBUrlLineEdit::setWebView(WBWebView *pWebView)
{
    mWebView = pWebView;

    connect(pWebView, SIGNAL(urlChanged(const QUrl &)),
        this, SLOT(webViewUrlChanged(const QUrl &)));

    connect(pWebView, SIGNAL(loadProgress(int)),
        this, SLOT(update()));
}


void WBUrlLineEdit::webViewUrlChanged(const QUrl &url)
{
    mLineEdit->setText(url.toString());
    mLineEdit->setCursorPosition(0);
}


void WBUrlLineEdit::focusOutEvent(QFocusEvent *event)
{
    if (mLineEdit->text().isEmpty() && mWebView)
        mLineEdit->setText(QString::fromUtf8(mWebView->url().toEncoded()));
    WBExLineEdit::focusOutEvent(event);
}


