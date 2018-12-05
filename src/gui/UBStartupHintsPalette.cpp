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



#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QCheckBox>
#include <QWebFrame>
#include "UBStartupHintsPalette.h"

#include "globals/UBGlobals.h"
#include "core/UBSettings.h"





UBStartupHintsPalette::UBStartupHintsPalette(QWidget *parent) :
    UBFloatingPalette(Qt::TopRightCorner,parent)
{
    setObjectName("UBStartupHintsPalette");
    if(UBSettings::settings()->appStartupHintsEnabled->get().toBool()){
        setFixedSize(700,450);
        mLayout = new QVBoxLayout();
        mLayout->setContentsMargins(10,28,10,10);
        setLayout(mLayout);
        QString url = UBSettings::settings()->applicationStartupHintsDirectory() + "/index.html";
        mpWebView = new QWebView(this);
        mpSankoreAPI = new UBWidgetUniboardAPI(0);
        mpWebView->page()->mainFrame()->addToJavaScriptWindowObject("sankore", mpSankoreAPI);
        connect(mpWebView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
        mpWebView->setUrl(QUrl::fromLocalFile(url));
        mpWebView->setAcceptDrops(false);
        mLayout->addWidget(mpWebView);
        mButtonLayout = new QHBoxLayout();
        mLayout->addLayout(mButtonLayout);
        mShowNextTime = new QCheckBox(tr("Visible next time"),this);
        mShowNextTime->setCheckState(Qt::Checked);
        connect(mShowNextTime,SIGNAL(stateChanged(int)),this,SLOT(onShowNextTimeStateChanged(int)));
        mButtonLayout->addStretch();
        mButtonLayout->addWidget(mShowNextTime);
    }
    else
        hide();
}

UBStartupHintsPalette::~UBStartupHintsPalette()
{
//    DELETEPTR(mButtonLayout);
//    DELETEPTR(mLayout);
}

void UBStartupHintsPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, QPixmap(":/images/close.svg"));
}


void UBStartupHintsPalette::close()
{
    hide();
}


void UBStartupHintsPalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        close();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}

void UBStartupHintsPalette::onShowNextTimeStateChanged(int state)
{
    UBSettings::settings()->appStartupHintsEnabled->setBool(state == Qt::Checked);
}

void UBStartupHintsPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    adjustSizeAndPosition();
    move((parentWidget()->width() - width()) / 2, (parentWidget()->height() - height()) / 5);
}


int UBStartupHintsPalette::border()
{
    return 40;
}

void UBStartupHintsPalette::javaScriptWindowObjectCleared()
{
    mpWebView->page()->mainFrame()->addToJavaScriptWindowObject("sankore", mpSankoreAPI);
}
