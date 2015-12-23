/*
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




#include "UBDocumentPublisher.h"


UBProxyLoginDlg::UBProxyLoginDlg(QWidget *parent, const char *name):QDialog(parent)
  , mpLayout(NULL)
  , mpUserLayout(NULL)
  , mpPasswordLayout(NULL)
  , mpButtons(NULL)
  , mpUserLabel(NULL)
  , mpPasswordLabel(NULL)
  , mpUsername(NULL)
  , mpPassword(NULL)
{
    setObjectName(name);
    setFixedSize(400, 150);
    setWindowTitle(tr("Proxy Login"));

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);
    mpUserLayout = new QHBoxLayout();
    mpLayout->addLayout(mpUserLayout);
    mpPasswordLayout = new QHBoxLayout();
    mpLayout->addLayout(mpPasswordLayout);

    mpUserLabel = new QLabel(tr("Username:"), this);
    mpUsername = new QLineEdit(this);
    mpUserLayout->addWidget(mpUserLabel, 0);
    mpUserLayout->addWidget(mpUsername, 1);

    mpPasswordLabel = new QLabel(tr("Password:"), this);
    mpPassword = new QLineEdit(this);
    mpPasswordLayout->addWidget(mpPasswordLabel, 0);
    mpPasswordLayout->addWidget(mpPassword, 1);

    mpButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mpLayout->addWidget(mpButtons);

    connect(mpButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mpButtons, SIGNAL(rejected()), this, SLOT(reject()));

}

UBProxyLoginDlg::~UBProxyLoginDlg()
{
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
    if(NULL != mpButtons)
    {
        delete mpButtons;
        mpButtons = NULL;
    }
    if(NULL != mpUserLabel)
    {
        delete mpUserLabel;
        mpUserLabel = NULL;
    }
    if(NULL != mpPasswordLabel)
    {
        delete mpPasswordLabel;
        mpPasswordLabel = NULL;
    }
    if(NULL != mpUsername)
    {
        delete mpUsername;
        mpUsername = NULL;
    }
    if(NULL != mpPassword)
    {
        delete mpPassword;
        mpPassword = NULL;
    }
}

// ---------------------------------------------------------
UBPublicationDlg::UBPublicationDlg(QWidget *parent, const char *name):QDialog(parent)
  , mpLayout(NULL)
  , mpTitleLayout(NULL)
  , mpTitleLabel(NULL)
  , mpTitle(NULL)
  , mpDescLabel(NULL)
  , mpDescription(NULL)
  , mpButtons(NULL)
{
    setObjectName(name);
    setWindowTitle(tr("Publish document on the web"));

    resize(500, 300);

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpTitleLabel = new QLabel(tr("Title:"), this);
    mpTitle = new QLineEdit(this);
    mpTitleLayout = new QHBoxLayout();
    mpTitleLayout->addWidget(mpTitleLabel, 0);
    mpTitleLayout->addWidget(mpTitle, 1);
    mpLayout->addLayout(mpTitleLayout, 0);

    mpDescLabel = new QLabel(tr("Description:"), this);
    mpLayout->addWidget(mpDescLabel, 0);

    mpDescription = new QTextEdit(this);
    mpLayout->addWidget(mpDescription, 1);

    mpButtons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, Qt::Horizontal, this);
    mpButtons->button(QDialogButtonBox::Ok)->setText(tr("Publish"));
    mpLayout->addWidget(mpButtons);

    mpButtons->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(mpButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mpButtons, SIGNAL(rejected()), this, SLOT(reject()));
    connect(mpTitle, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged()));
    connect(mpDescription, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

UBPublicationDlg::~UBPublicationDlg()
{
    if(NULL != mpTitleLabel)
    {
        delete mpTitleLabel;
        mpTitleLabel = NULL;
    }
    if(NULL != mpTitle)
    {
        delete mpTitle;
        mpTitle = NULL;
    }
    if(NULL != mpDescLabel)
    {
        delete mpDescLabel;
        mpDescLabel = NULL;
    }
    if(NULL != mpDescription)
    {
        delete mpDescription;
        mpDescription = NULL;
    }
    if(NULL != mpButtons)
    {
        delete mpButtons;
        mpButtons = NULL;
    }
    if(NULL != mpTitleLayout)
    {
        delete mpTitleLayout;
        mpTitleLayout = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

void UBPublicationDlg::onTextChanged()
{
    bool bPublishButtonState = false;
    if(mpTitle->text() != "" && mpDescription->document()->toPlainText() != "")
    {
        bPublishButtonState = true;
    }
    else
    {
        bPublishButtonState = false;
    }

    mpButtons->button(QDialogButtonBox::Ok)->setEnabled(bPublishButtonState);
}
