/*
 * UNWindow.h
 *
 *  Created on: Jan 9, 2009
 *      Author: julienbachmann
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
        void pointerClick();
        void arrowClicked();

#ifdef Q_WS_X11
        void refreshMask();
#endif

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
