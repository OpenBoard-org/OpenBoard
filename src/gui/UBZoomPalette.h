/*
 * UBZoomPalette.h
 *
 *  Created on: Mar 25, 2009
 *      Author: julienbachmann
 */

#ifndef UBZOOMPALETTE_H_
#define UBZOOMPALETTE_H_

#include "UBFloatingPalette.h"

class QPushButton;
class UBBoardController;

class UBZoomPalette : public UBFloatingPalette
{
    Q_OBJECT;

    public:
        UBZoomPalette(QWidget *parent);
        virtual ~UBZoomPalette();

    public slots:
        void hide();

    private:
        UBBoardController* mBoardController;
        QPushButton *mCurrentZoomButton;
        QPushButton *mHundredButton;
        QPushButton *mShowAllButton;

        bool mIsExpanded;

    private slots:
        void refreshPalette();
        void showHideExtraButton();
        void goHundred();

};

#endif /* UBZOOMPALETTE_H_ */
