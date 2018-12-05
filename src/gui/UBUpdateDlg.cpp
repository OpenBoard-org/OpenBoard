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




#include <QFileDialog>

#include "UBUpdateDlg.h"
#include "core/UBApplication.h"
#include "UBMainWindow.h"

#include "core/memcheck.h"

UBUpdateDlg::UBUpdateDlg(QWidget *parent, int nbFiles, const QString& bkpPath)
    : QDialog(parent)
    , mMainLayout(NULL)
    , mNbFilesLabel(NULL)
    , mBkpLabel(NULL)
    , mBkpPath(NULL)
    , mBrowseBttn(NULL)
    , mpDlgBttn(NULL)
    , mLayout(NULL)
    , mHLayout(NULL)
    , mStackedWidget(NULL)
    , mDialogWidget(NULL)
    , mProgressWidget(NULL)
    , mProgressLayout(NULL)
    , mProgressLabel(NULL)

{
    mDialogWidget = new QWidget(this);
    mProgressWidget = new QWidget(this);

    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->addWidget(mDialogWidget);
    mStackedWidget->addWidget(mProgressWidget);

    setFixedSize(450, 110);
    setModal(true);
    setWindowTitle(tr("Document updater"));
    mLayout = new QVBoxLayout();
    setLayout(mLayout);

    QString str = QString::number(nbFiles);
    str.append(tr(" files require an update."));
    mNbFilesLabel = new QLabel(mDialogWidget);
    mNbFilesLabel->setText(str);

    mLayout->addWidget(mNbFilesLabel);

    mBkpLabel = new QLabel(mDialogWidget);
    mBkpLabel->setText(tr("Backup path: "));

    mBkpPath = new QLineEdit(mDialogWidget);
    mBkpPath->setText(bkpPath);

    mBrowseBttn = new QPushButton(mDialogWidget);
    mBrowseBttn->setText(tr("Browse"));

    mHLayout = new QHBoxLayout();
    mHLayout->addWidget(mBkpLabel);
    mHLayout->addWidget(mBkpPath, 1);
    mHLayout->addWidget(mBrowseBttn);

    mLayout->addLayout(mHLayout);

    mpDlgBttn = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, mDialogWidget);
    mLayout->addWidget(mpDlgBttn);

    mpDlgBttn->button(QDialogButtonBox::Ok)->setText(tr("Update"));
    mpDlgBttn->button(QDialogButtonBox::Cancel)->setText(tr("Remind me later"));

    QObject::connect(mBrowseBttn, SIGNAL(clicked()), this, SLOT(onBrowse()));
    QObject::connect(mpDlgBttn, SIGNAL(accepted()), this, SLOT(onUpdate()));
    QObject::connect(mpDlgBttn, SIGNAL(rejected()), this, SLOT(reject()));
    mDialogWidget->setLayout(mLayout);
    mStackedWidget->setCurrentWidget(mDialogWidget);

    mMainLayout = new QVBoxLayout();
    this->setLayout(mMainLayout);
    mMainLayout->addWidget(mStackedWidget);
}

UBUpdateDlg::~UBUpdateDlg()
{
    if (NULL != mpDlgBttn)
    {
        delete mpDlgBttn;
        mpDlgBttn = NULL;
    }

    if (mNbFilesLabel) {
        delete mNbFilesLabel;
        mNbFilesLabel = NULL;
    }

    if (mBkpLabel) {
        delete mBkpLabel;
        mBkpLabel = NULL;
    }

    if (mBkpPath) {
        delete mBkpPath;
        mBkpPath = NULL;
    }

    if (mBrowseBttn) {
        delete mBrowseBttn;
        mBrowseBttn = NULL;
    }

    if (mProgressLabel) {
        delete mProgressLabel;
        mProgressLabel = NULL;
    }

    if (mHLayout) {
        delete mHLayout;
        mHLayout = NULL;
    }

    if (mLayout) {
        delete mLayout;
        mLayout = NULL;
    }

    if (mProgressLayout) {
        delete mProgressLayout;
        mProgressLayout = NULL;
    }

    if (mDialogWidget) {
        delete mDialogWidget;
        mDialogWidget = NULL;
    }

    if (mProgressWidget) {
        delete mProgressWidget;
        mProgressWidget = NULL;
    }

    if (mStackedWidget) {
        delete mStackedWidget;
        mStackedWidget = NULL;
    }

    if (mMainLayout) {
        delete mMainLayout;
        mMainLayout = NULL;
    }
}

void UBUpdateDlg::onBrowse()
{
    QString qsSelectedDir;
    qsSelectedDir = QFileDialog::getExistingDirectory(this, tr("Select a backup folder"), mBkpPath->text());
    mBkpPath->setText(qsSelectedDir);
}

void UBUpdateDlg::onUpdate()
{
    mProgressLabel = new QLabel(mProgressWidget);
    mProgressLabel->setText(tr("Please wait the import process will start soon..."));
    mProgressLayout = new QHBoxLayout();
    mProgressLayout->addWidget(mProgressLabel);
    mProgressWidget->setLayout(mProgressLayout);
    mStackedWidget->setCurrentWidget(mProgressWidget);
    emit updateFiles();
}

void UBUpdateDlg::onFilesUpdated(bool bResult)
{
    this->hide();
    QString qsMsg;

    if (bResult) {
        qsMsg = tr("Files update successful!\nPlease reboot the application to access the updated documents.");
    }
    else {
        qsMsg = tr("An error occured during the update. The files have not been affected.");
    }
    UBApplication::mainWindow->information(tr("Files update results"), qsMsg);
}

QString UBUpdateDlg::backupPath()
{
    return mBkpPath->text();
}

void UBUpdateDlg::transitioningFile(QString fileName)
{
    mProgressLabel->setText(tr("Updating file ") + fileName);
}

