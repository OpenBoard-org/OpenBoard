
#ifndef UBBOARDCONTROLLER_H_
#define UBBOARDCONTROLLER_H_

#include <QtGui>

#include <QObject>

class UBMainWindow;
class UBApplication;
class UBBoardView;

class UBDocumentController;
class UBLibraryController;
class UBMessageWindow;
class UBGraphicsScene;
class UBDocumentProxy;
class UBBlackoutWidget;
class UBToolWidget;
class UBVersion;
class UBSoftwareUpdate;
class UBSoftwareUpdateDialog;
class UBGraphicsVideoItem;
class UBGraphicsAudioItem;
class UBGraphicsWidgetItem;
class UBBoardPaletteManager;


class UBBoardController : public QObject
{
    Q_OBJECT;

    public:
        UBBoardController(UBMainWindow *mainWindow);
        virtual ~UBBoardController();

        void init();
        void setupLayout();

        UBDocumentProxy* activeDocument() const;
        UBGraphicsScene* activeScene() const;
        int activeSceneIndex() const;

        QSize displayViewport();
        QSize controlViewport();
        QRectF controlGeometry();

        void closing();

        UBDocumentProxy* activeDocument()
        {
            return mActiveDocument;
        }

        QWidget* controlContainer()
        {
            return mControlContainer;
        }

        UBBoardView* controlView()
        {
            return mControlView;
        }

        UBBoardView* displayView()
        {
            return mDisplayView;
        }

        void setPenColorOnDarkBackground(const QColor& pColor)
        {
            if (mPenColorOnDarkBackground == pColor)
                return;

            mPenColorOnDarkBackground = pColor;
            emit penColorChanged();
        }

        void setPenColorOnLightBackground(const QColor& pColor)
        {
            if (mPenColorOnLightBackground == pColor)
                return;

            mPenColorOnLightBackground = pColor;
            emit penColorChanged();
        }

        void setMarkerColorOnDarkBackground(const QColor& pColor)
        {
            mMarkerColorOnDarkBackground = pColor;
        }

        void setMarkerColorOnLightBackground(const QColor& pColor)
        {
            mMarkerColorOnLightBackground = pColor;
        }

        QColor penColorOnDarkBackground()
        {
            return mPenColorOnDarkBackground;
        }

        QColor penColorOnLightBackground()
        {
            return mPenColorOnLightBackground;
        }

        QColor markerColorOnDarkBackground()
        {
            return mMarkerColorOnDarkBackground;
        }

        QColor markerColorOnLightBackground()
        {
            return mMarkerColorOnLightBackground;
        }

        UBLibraryController* libraryController()
        {
            return mLibraryController;
        }

        qreal systemScaleFactor()
        {
            return mSystemScaleFactor;
        }

        qreal currentZoom();

        void persistCurrentScene();

        void showNewVersionAvailable(bool automatic, const UBVersion &installedVersion, const UBSoftwareUpdate &softwareUpdate);

        void setBoxing(QRect displayRect);

        void setToolbarTexts();

        static QUrl expandWidgetToTempDir(const QByteArray& pZipedData, const QString& pExtension = QString("wgt"));

        void setPageSize(QSize newSize);

        UBBoardPaletteManager *paletteManager()
        {
            return mPaletteManager;
        }

    public slots:

        void setActiveDocumentScene(UBDocumentProxy* pDocumentProxy, int pSceneIndex = 0);

        void showDocumentsDialog();
        void showLibraryDialog(bool show);

        void showKeyboard(bool show);

        void togglePodcast(bool checked);

        void blackout();

        void addScene();
        void addScene(UBDocumentProxy* proxy, int sceneIndex, bool replaceActiveIfEmpty = false);
        void addScene(UBGraphicsScene* scene, bool replaceActiveIfEmpty = false);
        void duplicateScene();
        void importPage();

        void clearScene();
        void clearSceneItems();
        void clearSceneAnnotation();

        void zoomIn(QPointF scenePoint = QPointF(0,0));
        void zoomOut(QPointF scenePoint = QPointF(0,0));
        void zoomRestore();
        void centerRestore();
        void centerOn(QPointF scenePoint = QPointF(0,0));

        void zoom(const qreal ratio, QPointF scenePoint);
        void handScroll(qreal dx, qreal dy);

        void previousScene();
        void nextScene();
        void firstScene();
        void lastScene();

        void downloadURL(const QUrl& url, const QPointF& pPos = QPointF(0.0, 0.0),
                const QSize& pSize = QSize(), bool isBackground = false);

        void downloadFinished(bool pSuccess, QUrl sourceUrl, QString pHeader,
                QByteArray pData, QPointF pPos, QSize pSize, bool isBackground = false);

        void changeBackground(bool isDark, bool isCrossed);

        void setToolCursor(int tool);

        void showMessage(const QString& message, bool showSpinningWheel);
        void hideMessage();

        void setDisabled(bool disable);

        void setColorIndex(int pColorIndex);

        UBToolWidget* addTool(const QUrl& toolUrl, QPointF scenePos);
        UBToolWidget* addTool(const QUrl& toolUrl);
        void removeTool(UBToolWidget* toolWidget);

        void hide();
        void show();

        void setWidePageSize(bool checked);
        void setRegularPageSize(bool checked);

        void stylusToolChanged(int tool);

        void grabScene(const QRectF& pSceneRect);

        void controlViewHidden();
        void controlViewShown();

        UBGraphicsVideoItem* addVideo(const QUrl& pUrl, bool startPlay, const QPointF& pos);
        UBGraphicsAudioItem* addAudio(const QUrl& pUrl, bool startPlay, const QPointF& pos);

        void cut();
        void copy();
        void paste();

        void processMimeData(const QMimeData* pMimeData, const QPointF& pPos);

        void moveGraphicsWidgetToControlView(UBGraphicsWidgetItem* graphicWidget);
        void moveToolWidgetToScene(UBToolWidget* toolWidget);

        void addItem();

    signals:

        void activeSceneWillBePersisted();
        void activeSceneWillChange();
        void activeSceneChanged();

        void activeDocumentChanged();

        void zoomChanged(qreal pZoomFactor);
        void systemScaleFactorChanged(qreal pSystemScaleFactor);
        void penColorChanged();
        void controlViewportChanged();

        void backgroundChanged();

    protected:

        void setupViews();
        void setupToolbar();
        void connectToolbar();
        void initToolbarTexts();

        void updateActionStates();
        void updateSystemScaleFactor();

        QString truncate(QString text, int maxWidth);

    protected slots:

        void selectionChanged();

        void undoRedoStateChange(bool canUndo);

        void documentSceneChanged(UBDocumentProxy* proxy, int pIndex);

    private:

        UBMainWindow *mMainWindow;

        UBDocumentProxy* mActiveDocument;
        UBGraphicsScene* mActiveScene;
        int mActiveSceneIndex;

        UBLibraryController* mLibraryController;
        UBBoardPaletteManager *mPaletteManager;

        UBSoftwareUpdateDialog *mSoftwareUpdateDialog;

        UBMessageWindow *mMessageWindow;

        UBBoardView *mControlView;
        UBBoardView *mDisplayView;

        QWidget *mControlContainer;
        QHBoxLayout *mControlLayout;

        qreal mZoomFactor;

        bool mIsClosing;

        QColor mPenColorOnDarkBackground;
        QColor mPenColorOnLightBackground;
        QColor mMarkerColorOnDarkBackground;
        QColor mMarkerColorOnLightBackground;

        QList<UBToolWidget*> mTools;

        qreal mSystemScaleFactor;

        void updatePageSizeState();
        void saveViewState();
        void adjustDisplayViews();

        bool mCleanupDone;

        QMap<QAction*, QPair<QString, QString> > mActionTexts;

    private slots:

        void stylusToolDoubleClicked(int tool);

        void boardViewResized(QResizeEvent* event);

        void documentWillBeDeleted(UBDocumentProxy* pProxy);

        void updateBackgroundActionsState(bool isDark, bool isCrossed);
        void updateBackgroundState();

        void colorPaletteChanged();

        void libraryDialogClosed(int ret);

        void lastWindowClosed();

};


#endif /* UBBOARDCONTROLLER_H_ */
