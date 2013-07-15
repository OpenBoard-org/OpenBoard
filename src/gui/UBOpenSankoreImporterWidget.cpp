/* Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique    (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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
    setObjectName("UBOpenSankoreImporterWidget");
    setFixedSize(700,450);

    QVBoxLayout* mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(20,38,20,20);
    setLayout(mLayout);

    QLabel* title = new QLabel(this);
    title->setText(tr("Open-Sankoré data detected"));
    mLayout->addWidget(title);

    QTextEdit* helpText = new QTextEdit(this);
    helpText->setText(tr("Open-Sankoré directory is present on the disk. It's possible to import the Open-Sankoré documents into OpenBoard as the preferences. Pushing \"Proceed\" will close OpenBoard and run the importer application."));
    helpText->setAcceptDrops(false);
    helpText->setReadOnly(true);
    mLayout->addWidget(helpText);

    mDisplayOnNextRestart = new QCheckBox(this);
    mDisplayOnNextRestart->setText(tr("Show this panel next time"));
    mDisplayOnNextRestart->setChecked(true);
    connect(mDisplayOnNextRestart,SIGNAL(clicked(bool)),this,SLOT(onNextRestartCheckBoxClicked(bool)));
    mLayout->addStretch();
    mLayout->addWidget(mDisplayOnNextRestart);
    mLayout->addStretch();

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

