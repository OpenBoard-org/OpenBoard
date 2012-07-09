/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#ifndef UBDOWNLOADWIDGET_H
#define UBDOWNLOADWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QItemDelegate>

#include "core/UBDownloadManager.h"

typedef enum{
    eItemColumn_Desc,
    eItemColumn_Close
}eItemColumn;

class UBDownloadProgressDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    UBDownloadProgressDelegate(QObject* parent=0);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class UBDownloadWidget : public QWidget
{
    Q_OBJECT
public:
    UBDownloadWidget(QWidget* parent=0, const char* name="UBDownloadWidget");
    ~UBDownloadWidget();

private slots:
    void onFileAddedToDownload();
    void onDownloadUpdated(int id, qint64 crnt, qint64 total);
    void onDownloadFinished(int id);
    void onCancelClicked();
    void onItemClicked(QTreeWidgetItem* pItem, int col);

private:
    void addCurrentDownloads();
    void addPendingDownloads();

    /** The general layout of this widget */
    QVBoxLayout* mpLayout;
    /** The button layout */
    QHBoxLayout* mpBttnLayout;
    /** The treeview that will display the files list */
    QTreeWidget* mpTree;
    /** The 'Cancel' button */
    QPushButton* mpCancelBttn;
    /** A temporary tree widget item */
    QTreeWidgetItem* mpItem;
    /** The delegate that will draw the progressbars */
    UBDownloadProgressDelegate mProgressBarDelegate;
};

#endif // UBDOWNLOADWIDGET_H
