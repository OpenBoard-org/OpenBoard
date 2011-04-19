
#ifndef UBAPPLICATIONCONTROLLER_H_
#define UBAPPLICATIONCONTROLLER_H_

#include <QtGui>

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

class UBApplicationController : public QObject
{
    Q_OBJECT;

    public:

        UBApplicationController(UBBoardView *pControlView, UBBoardView *pDisplayView,
                UBMainWindow *pMainWindow, QObject* parent = 0);
        virtual ~UBApplicationController();

        int initialHScroll() { return mInitialHScroll; }
        int initialVScroll() { return mInitialVScroll; }

        void adaptToolBar();
        void adjustDisplayView();
        void adjustPreviousViews(int pActiveSceneIndex, UBDocumentProxy *pActiveDocument);

        void blackout();

        void initScreenLayout();

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
            Board = 0, Internet, Document, Tutorial, ParaschoolEditor
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

        void showTutorial();
        void showSankoreEditor();

        void checkUpdateRequest();
        void checkUpdateAtLaunch();
        void downloadJsonFinished(QNetworkReply* pReply);

    protected:

        UBDesktopAnnotationController *mUninoteController;

        UBMainWindow *mMainWindow;

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
};

#endif /* UBAPPLICATIONCONTROLLER_H_ */
