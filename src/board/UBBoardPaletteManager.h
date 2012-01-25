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

#ifndef UBBOARDPALETTEMANAGER_H_
#define UBBOARDPALETTEMANAGER_H_

#include <QtGui>
#include <QtWebKit>

#include "web/UBRoutedMouseEventWebView.h"
#include "gui/UBLeftPalette.h"
#include "gui/UBRightPalette.h"
#include "gui/UBPageNavigationWidget.h"
#include "gui/UBLibWidget.h"
#include "gui/UBCachePropertiesWidget.h"
#include "gui/UBTeacherBarWidget.h"
#include "gui/UBDockDownloadWidget.h"
#include "core/UBApplicationController.h"


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
class UBApplicationController;

class UBBoardPaletteManager : public QObject
{
    Q_OBJECT

    public:
        UBBoardPaletteManager(QWidget* container, UBBoardController* controller);
        virtual ~UBBoardPaletteManager();

        void setupLayout();
        UBLeftPalette* leftPalette(){return mLeftPalette;}
        UBRightPalette* rightPalette(){return mRightPalette;}
        void showVirtualKeyboard(bool show = true);
        void initPalettesPosAtStartup();
        void connectToDocumentController();
        void refreshPalettes();

        UBKeyboardPalette *mKeyboardPalette;
//        UBRightPalette* createDesktopRightPalette(QWidget* parent);

        void processPalettersWidget(UBDockPalette *paletter, eUBDockPaletteWidgetMode mode);
        void changeMode(eUBDockPaletteWidgetMode newMode, bool isInit = false);
        void startDownloads();
        void stopDownloads();
        QRect GetFreeRectGlobalCoords() const;
		void ForceTeacherBarToSaveData();
		void ForceTeacherBarToLoadData();

    signals:
        void connectToDocController();
        void signal_changeMode(eUBDockPaletteWidgetMode newMode);

    public slots:

        void activeSceneChanged();
        void containerResized();
        void addItem(const QUrl& pUrl);
        void addItem(const QPixmap& pPixmap, const QPointF& p = QPointF(0.0, 0.0), qreal scale = 1.0, const QUrl& sourceUrl = QUrl());

//        void slot_changeMode(eUBDockPaletteWidgetMode newMode);
        void slot_changeMainMode(UBApplicationController::MainMode);
        void slot_changeDesktopMode(bool);

    private:

        void setupPalettes();
        void connectPalettes();
        void positionFreeDisplayPalette();
        void setupDockPaletteWidgets();

        QWidget* mContainer;
        UBBoardController *mBoardControler;

        UBStylusPalette *mStylusPalette;

        UBZoomPalette *mZoomPalette;

        /** The left dock palette */
        UBLeftPalette* mLeftPalette;
        /** The right dock palette */
        UBRightPalette* mRightPalette;

//         // HACK: here we duplicate the right palette for the desktop mode
//         //       we MUST refactor the architecture in order to use only one
//         //       right palette!
//         UBRightPalette* mDesktopRightPalette;

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

        /** The page navigator widget */
        UBPageNavigationWidget* mpPageNavigWidget;
        /** The library widget */
        UBLibWidget* mpLibWidget;
        /** The cache properties widget */
        UBCachePropertiesWidget* mpCachePropWidget;
        /** The teacher bar widget */
        UBTeacherBarWidget* mpTeacherBarWidget;
        /** The download widget */
        UBDockDownloadWidget* mpDownloadWidget;
        // HACK: here we duplicate the lib widget for the desktop mode
        //       we MUST refactor the architecture in order to use only one
        //       lib widget!
        UBLibWidget* mpDesktopLibWidget;

        bool mDownloadInProgress;

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

        void purchaseLinkActivated(const QString&);

        void linkClicked(const QUrl& url);

        void zoomButtonPressed();
        void zoomButtonReleased();
        void panButtonPressed();
        void panButtonReleased();

        void changeStylusPaletteOrientation(QVariant var);
};

#endif /* UBBOARDPALETTEMANAGER_H_ */
