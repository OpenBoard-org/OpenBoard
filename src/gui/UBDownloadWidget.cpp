/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QDebug>
#include <QHeaderView>
#include <QStyleOptionProgressBarV2>
#include <QApplication>

#include "UBDownloadWidget.h"
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
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());
    resize(400, 300);

    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpTree = new QTreeWidget(this);
    mpTree->setRootIsDecorated(false);
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
        mpItem->setText(0, actualDL.at(i).name);
        mpItem->setData(0, Qt::UserRole, QVariant(actualDL.at(i).id));
        mpTree->addTopLevelItem(mpItem);
        mpItem = new QTreeWidgetItem(mpTree);
        mpItem->setData(0, Qt::UserRole, actualDL.at(i).currentSize);
        mpItem->setData(0, Qt::UserRole + 1, actualDL.at(i).totalSize);
        mpItem->setData(0, Qt::UserRole + 2, actualDL.at(i).id);
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
    qDebug() << "Pending downloads size: " << pendingDL.size();
    for(int i=0; i<pendingDL.size(); i++)
    {
        mpItem = new QTreeWidgetItem(mpTree);
        mpItem->setText(0, pendingDL.at(i).name);
        mpItem->setData(0, Qt::UserRole, QVariant(pendingDL.at(i).id));
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
                QModelIndex currentIndex = model->index(i, 0);
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

// ---------------------------------------------------------------------------------------------
UBDownloadProgressDelegate::UBDownloadProgressDelegate(QObject *parent):QItemDelegate(parent)
{

}

void UBDownloadProgressDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionProgressBarV2 opt;
    opt.rect = option.rect;
    opt.minimum = 0;
    opt.maximum = index.data(Qt::UserRole + 1).toInt();
    opt.progress = index.data(Qt::UserRole).toInt();

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &opt, painter, 0);
}
