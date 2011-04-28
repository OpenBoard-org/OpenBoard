/*
 * UBBoardPaletteManager.h
 *
 *  Created on: 3 nov. 2009
 *      Author: Luc
 */

#ifndef UBBOARDPALETTEMANAGER_H_
#define UBBOARDPALETTEMANAGER_H_

#include <QtGui>
#include <QtWebKit>

#include "web/UBRoutedMouseEventWebView.h"
#include "gui/UBNavigatorPalette.h"
#include "gui/UBLibPalette.h"

class UBStylusPalette;
class UBClockPalette;
class UBPageNumberPalette;
class UBZoomPalette;
class UBActionPalette;
class UBBoardController;
class UBFloatingPalette;
class UBServerXMLHttpRequest;
class UBKeyboardPalette;
class UBMainWindow;

class UBBoardPaletteManager : public QObject
{
    Q_OBJECT;

    public:
        UBBoardPaletteManager(QWidget* container, UBBoardController* controller);
        virtual ~UBBoardPaletteManager();

        void setupLayout();
		UBNavigatorPalette* navigatorPalette(){return mNavigPalette;}
		void showVirtualKeyboard(bool show = true);
        void initPalettesPosAtStartup();

    public slots:

        void activeSceneChanged();
        void containerResized();
        void addItem(const QUrl& pUrl);
        void addItem(const QPixmap& pPixmap, const QPointF& p = QPointF(0.0, 0.0), qreal scale = 1.0, const QUrl& sourceUrl = QUrl());

    private:

        void setupPalettes();
        void connectPalettes();
        void positionFreeDisplayPalette();

        QWidget* mContainer;
        UBBoardController *mBoardControler;

        UBStylusPalette *mStylusPalette;

        UBZoomPalette *mZoomPalette;
        UBKeyboardPalette *mKeyboardPalette;
        UBNavigatorPalette* mNavigPalette;
        UBLibPalette* mLibPalette;

        UBActionPalette *mBackgroundsPalette;
        UBActionPalette *mToolsPalette;
        UBActionPalette* mAddItemPalette;
        UBActionPalette* mErasePalette;
        UBActionPalette* mPagePalette;

        QUrl mItemUrl;
        QPixmap mPixmap;
        QPointF mPos;
        qreal mScaleFactor;

        QTime mPageButtonPressedTime;
        bool mPendingPageButtonPressed;

        QTime mZoomButtonPressedTime;
        bool mPendingZoomButtonPressed;

        QTime mPanButtonPressedTime;
        bool mPendingPanButtonPressed;

        QTime mEraseButtonPressedTime;
        bool mPendingEraseButtonPressed;

    private slots:

        void changeBackground();

        void toggleBackgroundPalette(bool checked);
        void backgroundPaletteClosed();

        void toggleStylusPalette(bool checked);
        void tooglePodcastPalette(bool checked);

        void erasePaletteButtonPressed();
        void erasePaletteButtonReleased();

        void toggleErasePalette(bool ckecked);
        void erasePaletteClosed();

        void togglePagePalette(bool ckecked);
        void pagePaletteClosed();

		void pagePaletteButtonPressed();
        void pagePaletteButtonReleased();

        void addItemToCurrentPage();
        void addItemToNewPage();
        void addItemToLibrary();
        void shareItemOnWeb();

        void purchaseLinkActivated(const QString&);

        void linkClicked(const QUrl& url);

        void zoomButtonPressed();
        void zoomButtonReleased();
        void panButtonPressed();
        void panButtonReleased();

        void changeStylusPaletteOrientation(QVariant var);
};

#endif /* UBBOARDPALETTEMANAGER_H_ */
