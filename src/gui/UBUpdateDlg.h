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
