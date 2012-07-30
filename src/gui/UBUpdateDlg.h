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
#ifndef UBUPDATEDLG_H
#define UBUPDATEDLG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>

class UBUpdateDlg : public QDialog
{
    Q_OBJECT

public:
    UBUpdateDlg(QWidget *parent = 0, int nbFiles = 0, const QString& bkpPath = "");
    ~UBUpdateDlg();
    QString backupPath();

public slots:
    void onFilesUpdated(bool bResult);

signals:
    void updateFiles();

private slots:
    void onBrowse();
    void onUpdate();
    void transitioningFile(QString fileName);

private:
    QVBoxLayout* mMainLayout;

    QLabel* mNbFilesLabel;
    QLabel* mBkpLabel;
    QLineEdit* mBkpPath;
    QPushButton* mBrowseBttn;
    QDialogButtonBox* mpDlgBttn;
    QVBoxLayout* mLayout;
    QHBoxLayout* mHLayout;


    QStackedWidget* mStackedWidget;
    QWidget* mDialogWidget;
    QWidget* mProgressWidget;
    QHBoxLayout* mProgressLayout;
    QLabel* mProgressLabel;
};

#endif // UBUPDATEDLG_H
