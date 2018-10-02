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
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>

#include "core/UBSettings.h"

#include "UBOpenSankoreImporterWidget.h"

UBOpenSankoreImporterWidget::UBOpenSankoreImporterWidget(QWidget *parent):
    UBFloatingPalette(Qt::TopRightCorner,parent)
{
    setBackgroundBrush(QBrush(Qt::white));

    setObjectName("UBOpenSankoreImporterWidget");
    setFixedSize(700,450);
    setStyleSheet("QWidget#UBOpenSankoreImporterWidget { background-color : red; }");

    QVBoxLayout* mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(20,38,20,20);
    setLayout(mLayout);

    QLabel* title = new QLabel(this);
    title->setStyleSheet("font-size : 18px; font-weight : bold;");
    title->setText(tr("Open-Sankore Documents Detected"));
    mLayout->addWidget(title);
    mLayout->addSpacing(20);

    QTextEdit* helpText = new QTextEdit(this);
    helpText->setText(tr("Open-Sankoré documents are present on your computer. It is possible to import them to OpenBoard by pressing the “Proceed” button to launch the importer application."));
    helpText->setAcceptDrops(false);
    helpText->setReadOnly(true);
    helpText->setStyleSheet("border : none;");
    mLayout->addWidget(helpText);

    mDisplayOnNextRestart = new QCheckBox(this);
    mDisplayOnNextRestart->setText(tr("Show this panel next time"));
    mDisplayOnNextRestart->setChecked(true);
    connect(mDisplayOnNextRestart,SIGNAL(clicked(bool)),this,SLOT(onNextRestartCheckBoxClicked(bool)));
    mLayout->addWidget(mDisplayOnNextRestart);
    mLayout->addSpacing(100);

    QTextEdit* warningText = new QTextEdit(this);
    warningText->setText(tr("You can always access the OpenBoard Document Importer through the Preferences panel in the About tab. Warning, if you have already imported your Open-Sankore datas, you might loose your current OpenBoard documents."));
    warningText->setReadOnly(true);
    warningText->setStyleSheet("border : none;");
    mLayout->addWidget(warningText);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* mCancelButton = new QPushButton(this);
    mCancelButton->setText(tr("Cancel"));
    buttonLayout->addWidget(mCancelButton);
    buttonLayout->addStretch();
    connect(mCancelButton,SIGNAL(clicked()),this,SLOT(close()));

    mProceedButton = new QPushButton(this);
    mProceedButton->setText(tr("Proceed"));
    buttonLayout->addWidget(mProceedButton);

    mLayout->addLayout(buttonLayout);


    show();
}

void UBOpenSankoreImporterWidget::onNextRestartCheckBoxClicked(bool clicked)
{
    UBSettings::settings()->appLookForOpenSankoreInstall->setBool(clicked);
}

void UBOpenSankoreImporterWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    adjustSizeAndPosition();
    move((parentWidget()->width() - width()) / 2, (parentWidget()->height() - height()) / 5);
}


int UBOpenSankoreImporterWidget::border()
{
    return 10;
}

