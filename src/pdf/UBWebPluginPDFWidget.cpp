/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include <QtGui>

#include "UBWebPluginPDFWidget.h"
#include "XPDFRenderer.h"


#include "core/memcheck.h"

UBWebPluginPDFWidget::UBWebPluginPDFWidget(const QUrl &url, QWidget *parent)
    : UBWebPluginWidget(url, parent)
    , mPreviousPageButton(this)
    , mNextPageButton(this)
    , mPreviousPageAction(0)
    , mNextPageAction(0)
    , mRenderer(0)
    , mScale(1.5)
    , mPageNumber(1)
{
    QIcon previousPageIcon;
    QIcon nextPageIcon;
    previousPageIcon.addFile(":/images/toolbar/previousPage.png", QSize(32, 32), QIcon::Normal);
    nextPageIcon.addFile(":/images/toolbar/nextPage.png", QSize(32, 32), QIcon::Normal);
    mPreviousPageAction = new QAction(previousPageIcon, QString(), this);
    mNextPageAction = new QAction(nextPageIcon, QString(), this);

    connect(mPreviousPageAction, SIGNAL(triggered()), this, SLOT(previousPage()));
    connect(mNextPageAction, SIGNAL(triggered()), this, SLOT(nextPage()));

    mPreviousPageButton.setFixedSize(32, 32);
    mNextPageButton.setFixedSize(32, 32);

    mPreviousPageButton.setDefaultAction(mPreviousPageAction);
    mNextPageButton.setDefaultAction(mNextPageAction);
    mNextPageButton.move(mPreviousPageButton.x() + mPreviousPageButton.width() + 10, mNextPageButton.y());
}

UBWebPluginPDFWidget::~UBWebPluginPDFWidget()
{
    if (mRenderer)
    {
        delete mRenderer;
    }
}

void UBWebPluginPDFWidget::handleFile(const QString &filePath)
{
    mRenderer = new XPDFRenderer(filePath);
}

void UBWebPluginPDFWidget::keyReleaseEvent(QKeyEvent *keyEvent)
{
    // TOOD: why is it not called?
    switch (keyEvent->key())
    {
        case Qt::Key_Right:
            if (nextPage())
            {
                keyEvent->accept();
            }
            break;
        case Qt::Key_Left:
            if (previousPage())
            {
                keyEvent->accept();
            }
            break;
        default:
            break;
    }
}

bool UBWebPluginPDFWidget::previousPage()
{
    if (mPageNumber > 1)
    {
        mPageNumber--;
        update();
        return true;
    }
    return false;
}

bool UBWebPluginPDFWidget::nextPage()
{
    if (mPageNumber < mRenderer->pageCount())
    {
        mPageNumber++;
        update();
        return true;
    }
    return false;
}

void UBWebPluginPDFWidget::zoomIn()
{
    mScale *= 1.41;
    update();
}

void UBWebPluginPDFWidget::zoomOut()
{
    mScale /= 1.41;
    update();
}

void UBWebPluginPDFWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (!mRenderer)
    {
        // PDF not yet downloaded
        return;
    }

    if (!mRenderer->isValid())
    {
        qWarning("UBWebPluginPDFWidget::paintEvent: Invalid renderer");
        return;
    }

    QSizeF pageSize = mRenderer->pageSizeF(mPageNumber);

    painter.translate((geometry().width() - (pageSize.width() * mScale)) / 2, 0);
    painter.scale(mScale, mScale);
    mRenderer->render(&painter, mPageNumber, event->rect());
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawRect(0, 0, pageSize.width(), pageSize.height());
}

QString UBWebPluginPDFWidget::title() const
{
    if (mRenderer)
    {
        return mRenderer->title();
    }
    else
    {
        return UBWebPluginWidget::title();
    }
}
