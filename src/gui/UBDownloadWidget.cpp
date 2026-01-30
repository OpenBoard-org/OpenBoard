/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include <QDebug>
#include <QHeaderView>
#include <QStyleOptionProgressBar>
#include <QApplication>

#include "UBDownloadWidget.h"

#include "globals/UBGlobals.h"

#include "core/UBApplication.h"
#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the widget object name
 */
UBDownloadWidget::UBDownloadWidget(QWidget *parent, const char *name):QWidget(parent)
  , mpLayout(NULL)
  , mpBttnLayout(NULL)
  , mpTree(NULL)
  , mpCancelBttn(NULL)
  , mpItem(NULL)
{
    setObjectName(name);
    setWindowTitle(tr("Downloading files"));
    SET_STYLE_SHEET();
    resize(400, 300);

    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpTree = new QTreeWidget(this);
    mpTree->setRootIsDecorated(false);
    mpTree->setColumnCount(2);
    mpTree->header()->setStretchLastSection(false);
    mpTree->header()->setSectionResizeMode(eItemColumn_Desc, QHeaderView::Stretch);
    mpTree->header()->setSectionResizeMode(eItemColumn_Close, QHeaderView::Custom);
    mpTree->resizeColumnToContents(eItemColumn_Close);
    mpTree->header()->close();
    mpLayout->addWidget(mpTree, 1);

    mpBttnLayout = new QHBoxLayout();
    mpBttnLayout->addStretch(1);
    mpCancelBttn = new QPushButton(tr("Cancel"), this);
    mpCancelBttn->setObjectName("DockPaletteWidgetButton");
    mpBttnLayout->addWidget(mpCancelBttn, 0);
    mpLayout->addLayout(mpBttnLayout);

    connect(UBDownloadManager::downloadManager(), SIGNAL(fileAddedToDownload()), this, SLOT(onFileAddedToDownload()));
    connect(UBDownloadManager::downloadManager(), SIGNAL(downloadUpdated(int,qint64,qint64)), this, SLOT(onDownloadUpdated(int,qint64,qint64)));
    connect(UBDownloadManager::downloadManager(), SIGNAL(downloadFinished(int)), this, SLOT(onDownloadFinished(int)));
    connect(mpCancelBttn, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
    connect(mpTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onItemClicked(QTreeWidgetItem*,int)));
}

/**
 * \brief Destructor
 */
UBDownloadWidget::~UBDownloadWidget()
{
    if(NULL != mpCancelBttn)
    {
        delete mpCancelBttn;
        mpCancelBttn = NULL;
    }
    if(NULL != mpTree)
    {
        delete mpTree;
        mpTree = NULL;
    }
    if(NULL != mpBttnLayout)
    {
        delete mpBttnLayout;
        mpBttnLayout = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

/**
 * \brief Refresh the tree of downloaded files
 */
void UBDownloadWidget::onFileAddedToDownload()
{
    if(NULL != mpTree)
    {
        mpTree->clear();
        addCurrentDownloads();
        addPendingDownloads();
    }
}

/**
 * \brief Add the current downloads
 */
void UBDownloadWidget::addCurrentDownloads()
{
    QVector<sDownloadFileDesc> actualDL = UBDownloadManager::downloadManager()->currentDownloads();
    qDebug() << "Actual downloads size: " << actualDL.size();
    for(int i=0; i<actualDL.size();i++)
    {
        mpItem = new QTreeWidgetItem(mpTree);
        mpItem->setText(eItemColumn_Desc, actualDL.at(i).name);
        mpItem->setData(eItemColumn_Desc, Qt::UserRole, QVariant(actualDL.at(i).id));
        mpItem->setIcon(eItemColumn_Close, QIcon(":images/close.svg"));
        mpTree->addTopLevelItem(mpItem);
        mpItem = new QTreeWidgetItem(mpTree);
        mpItem->setData(eItemColumn_Desc, Qt::UserRole, actualDL.at(i).currentSize);
        mpItem->setData(eItemColumn_Desc, Qt::UserRole + 1, actualDL.at(i).totalSize);
        mpItem->setData(eItemColumn_Desc, Qt::UserRole + 2, actualDL.at(i).id);
        mpTree->addTopLevelItem(mpItem);
        mpTree->setItemDelegateForRow(((i+1)*2)-1, &mProgressBarDelegate);
    }
}

/**
 * \brief Add the pending downloads
 */
void UBDownloadWidget::addPendingDownloads()
{
    QVector<sDownloadFileDesc> pendingDL = UBDownloadManager::downloadManager()->pendingDownloads();
    for(int i=0; i<pendingDL.size(); i++)
    {
        mpItem = new QTreeWidgetItem(mpTree);
        mpItem->setText(eItemColumn_Desc, pendingDL.at(i).name);
        mpItem->setData(eItemColumn_Desc, Qt::UserRole, QVariant(pendingDL.at(i).id));
        mpTree->addTopLevelItem(mpItem);
    }
}

/**
 * \brief Update the progress bar
 * @param id as the downloaded file id
 * @param crnt as the current transfered size
 * @param total as the total size of the file
 */
void UBDownloadWidget::onDownloadUpdated(int id, qint64 crnt, qint64 total)
{
    if(NULL != mpTree)
    {
        QAbstractItemModel* model = mpTree->model();
        if(NULL != model)
        {
            for(int i=0; i< model->rowCount(); i++)
            {
                QModelIndex currentIndex = model->index(i, eItemColumn_Desc);
                if(id == currentIndex.data(Qt::UserRole + 2))
                {
                    // We found the right item, now we update the progress bar
                    model->setData(currentIndex, crnt, Qt::UserRole);
                    model->setData(currentIndex, total, Qt::UserRole + 1);
                    break;
                }
            }
        }
    }
}

/**
 * \brief Handles the download finish notification
 * @param id as the downloaded file id
 */
void UBDownloadWidget::onDownloadFinished(int id)
{
    Q_UNUSED(id);
    // Refresh the file's list
    onFileAddedToDownload();
}

/**
 * \brief Handles the Cancel button action
 */
void UBDownloadWidget::onCancelClicked()
{
    UBDownloadManager::downloadManager()->cancelDownloads();
}

/**
 * \brief Handles the item click notification
 * @param pItem as the item clicked
 * @param col as the column containing the item clicked
 */
void UBDownloadWidget::onItemClicked(QTreeWidgetItem *pItem, int col)
{
    if( eItemColumn_Close == col
            && "" != pItem->text(eItemColumn_Desc)){

        // Stop the download of the clicked item and remove it from the list
        UBDownloadManager::downloadManager()->cancelDownload(pItem->data(eItemColumn_Desc, Qt::UserRole).toInt());
    }
}

// ---------------------------------------------------------------------------------------------
UBDownloadProgressDelegate::UBDownloadProgressDelegate(QObject *parent):QItemDelegate(parent)
{

}

void UBDownloadProgressDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionProgressBar opt;
    opt.rect = option.rect;
    opt.minimum = 0;
    opt.maximum = index.data(Qt::UserRole + 1).toInt();
    opt.progress = index.data(Qt::UserRole).toInt();

    if(0 == index.column()){
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &opt, painter, 0);
    }
}
