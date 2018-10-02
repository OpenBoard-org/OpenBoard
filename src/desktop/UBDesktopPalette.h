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




#ifndef UBUNINOTESWINDOW_H_
#define UBUNINOTESWINDOW_H_
#include <QtGui>
#include <QShowEvent>
#include <QHideEvent>

#include "gui/UBActionPalette.h"
#include "gui/UBRightPalette.h"

/**
 * The uninotes window. This window is controlled by UBUninotesWindowController.
 */
class UBDesktopPalette : public UBActionPalette
{
    Q_OBJECT

    public:
        UBDesktopPalette(QWidget *parent, UBRightPalette* rightPalette);
        virtual ~UBDesktopPalette();

        void disappearForCapture();
        void appear();
        QPoint buttonPos(QAction* action);

    signals:
        void uniboardClick();
        void customClick();
        void windowClick();
        void screenClick();

//#ifdef Q_OS_LINUX //TODO: check why this produces an error on linux if uncommented
        void refreshMask();
//#endif

    public slots:

        void showHideClick(bool checked);
        void updateShowHideState(bool pShowEnabled);
        void setShowHideButtonVisible(bool visible);
        void setDisplaySelectButtonVisible(bool show);
        void minimizeMe(eMinimizedLocation location);
        void maximizeMe();
        void parentResized();

protected:
        void showEvent(QShowEvent *event);
        void hideEvent(QHideEvent *event);

        virtual int getParentRightOffset();

private:
        QAction *mShowHideAction;
        QAction *mDisplaySelectAction;
        QAction *mMaximizeAction;
        QAction *mActionUniboard;
        QAction *mActionCustomSelect;
        QAction* mActionTest;


        UBRightPalette* rightPalette;
        void adjustPosition();


signals:
        void stylusToolChanged(int tool);

};

#endif /* UBUNINOTESWINDOW_H_ */
