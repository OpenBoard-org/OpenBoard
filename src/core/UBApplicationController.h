/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBAPPLICATIONCONTROLLER_H_
#define UBAPPLICATIONCONTROLLER_H_

#include <QtGui>
#include <QFtp>

class UBBoardView;
class UBDocumentProxy;
class UBGraphicsScene;
class UBDesktopAnnotationController;
class UBScreenMirror;
class UBMainWindow;
class UBDisplayManager;
class UBVersion;
class UBSoftwareUpdate;
class QNetworkAccessManager;
class QNetworkReply;
class QHttp;
class UBRightPalette;
class UBOpenSankoreImporter;

class UBApplicationController : public QObject
{
    Q_OBJECT

    public:

        UBApplicationController(UBBoardView *pControlView, UBBoardView *pDisplayView, UBMainWindow *pMainWindow, QObject* parent, UBRightPalette* rightPalette);
        virtual ~UBApplicationController();

        int initialHScroll() { return mInitialHScroll; }
        int initialVScroll() { return mInitialVScroll; }

        void adaptToolBar();
        void adjustDisplayView();
        void adjustPreviousViews(int pActiveSceneIndex, UBDocumentProxy *pActiveDocument);

        void blackout();

        void initScreenLayout(bool useMultiscreen);

        void closing();

        void setMirrorSourceWidget(QWidget*);

        void mirroringEnabled(bool);

        void initViewState(int horizontalPosition, int verticalPosition);

        void showBoard();

        void showInternet();

        void showDocument();

        void showMessage(const QString& message, bool showSpinningWheel);

        void importFile(const QString& pFilePath);

        UBDisplayManager* displayManager()
        {
            return mDisplayManager;
        }

        UBDesktopAnnotationController* uninotesController()
        {
            return mUninoteController;
        }

        enum MainMode
        {
            Board = 0, Internet, Document, WebDocument
        };

        MainMode displayMode()
        {
            return mMainMode;
        }

        bool isCheckingForSoftwareUpdate() const;

        bool isShowingDesktop()
        {
            return mIsShowingDesktop;
        }

        QStringList widgetInlineJavaScripts();

    signals:
        void mainModeChanged(UBApplicationController::MainMode pMode);
        void desktopMode(bool displayed);

    public slots:

        /**
         * Add the pPixmap to the current scene and reactivate the board.
         * This Slot is connected with uninotes to manage the transition between board and uninotes.
         */
        void addCapturedPixmap(const QPixmap &pPixmap, bool pageMode, const QUrl& sourceUrl = QUrl());

        void addCapturedEmbedCode(const QString& embedCode);

        void screenLayoutChanged();

        // defaulting to false to match QAction triggered(bool checked = false)
        void showDesktop(bool dontSwitchFrontProcess = false);

        void hideDesktop();

        void useMultiScreen(bool use);

        void actionCut();
        void actionCopy();
        void actionPaste();

        void checkUpdateRequest();
        void checkAtLaunch();

    private slots:
        void updateRequestFinished(int id, bool error);

    protected:

        UBDesktopAnnotationController *mUninoteController;

        UBMainWindow *mMainWindow;

        UBOpenSankoreImporter *mOpenSankoreImporter;

        UBBoardView *mControlView;
        UBBoardView *mDisplayView;
        QList<UBBoardView*> mPreviousViews;

        UBGraphicsScene *mBlackScene;

        UBScreenMirror* mMirror;

        int mInitialHScroll, mInitialVScroll;

    private:

        MainMode mMainMode;

        UBDisplayManager *mDisplayManager;

        bool mAutomaticCheckForUpdates;
        bool mCheckingForUpdates;

        void setCheckingForUpdates(bool value);

        bool mIsShowingDesktop;

        bool isNoUpdateDisplayed;
        void checkUpdate ();
        QNetworkAccessManager *networkAccessManager;

        void downloadJsonFinished(QString updateString);
        QHttp* mHttp;
};

#endif /* UBAPPLICATIONCONTROLLER_H_ */
