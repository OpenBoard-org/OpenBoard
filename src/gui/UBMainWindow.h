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




#ifndef UBMAINWINDOW_H_
#define UBMAINWINDOW_H_

#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>
#include "UBDownloadWidget.h"

class QStackedLayout;

#include "ui_mainWindow.h"

class UBMainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
    public:

        UBMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
        virtual ~UBMainWindow();

        void addBoardWidget(QWidget *pWidget);
        void switchToBoardWidget();

        void addWebWidget(QWidget *pWidget);
        void switchToWebWidget();

        void addDocumentsWidget(QWidget *pWidget);
        void switchToDocumentsWidget();

        bool yesNoQuestion(QString windowTitle, QString text);
        void warning(QString windowTitle, QString text);
        void information(QString windowTitle, QString text);

        void showDownloadWidget();
        void hideDownloadWidget();

    signals:
        void closeEvent_Signal( QCloseEvent *event );

    public slots:
        void onExportDone();

    protected:
        void oneButtonMessageBox(QString windowTitle, QString text, QMessageBox::Icon type);

        virtual void keyPressEvent(QKeyEvent *event);
        virtual void closeEvent (QCloseEvent *event);

        virtual QMenu* createPopupMenu ()
        {
            // no pop up on toolbar
            return 0;
        }

        QStackedLayout* mStackedLayout;

        QWidget *mBoardWidget;
        QWidget *mWebWidget;
        QWidget *mDocumentsWidget;

private:
// work around for handling tablet events on MAC OS with Qt 4.8.0 and above
#if defined(Q_OS_OSX)
        bool event(QEvent *event);
#endif
        UBDownloadWidget* mpDownloadWidget;
};

#endif /* UBMAINWINDOW_H_ */
