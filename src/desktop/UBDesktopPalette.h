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

#ifndef UBUNINOTESWINDOW_H_
#define UBUNINOTESWINDOW_H_
#include <QtGui>
#include <QShowEvent>
#include <QHideEvent>

#include "gui/UBActionPalette.h"

/**
 * The uninotes window. This window is controlled by UBUninotesWindowController.
 */
class UBDesktopPalette : public UBActionPalette
{
    Q_OBJECT;

    public:
        UBDesktopPalette(QWidget *parent = 0);
        virtual ~UBDesktopPalette();

        void disappearForCapture();
        void appear();
        QPoint buttonPos(QAction* action);

    signals:
        void uniboardClick();
        void customClick();
        void windowClick();
        void screenClick();
        void showVirtualKeyboard(bool);

    public slots:

        void showHideClick(bool checked);

        void updateShowHideState(bool pShowEnabled);
        void setShowHideButtonVisible(bool visible);
        void setDisplaySelectButtonVisible(bool show);
	void minimizeMe(eMinimizedLocation location);
        void maximizeMe();

protected:
        void showEvent(QShowEvent *event);
        void hideEvent(QHideEvent *event);

    private:
        QAction *mShowHideAction;
        QAction *mDisplaySelectAction;
        QAction *mMaximizeAction;
        QAction *mActionUniboard;
        QAction *mActionCustomSelect;
        QAction* mActionTest;

    signals:
        void stylusToolChanged(int tool);

};

#endif /* UBUNINOTESWINDOW_H_ */
