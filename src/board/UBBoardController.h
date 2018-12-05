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




#ifndef UBBOARDCONTROLLER_H_
#define UBBOARDCONTROLLER_H_

#include <QtGui>

#include <QObject>
#include <QHBoxLayout>
#include <QUndoCommand>

#include "document/UBDocumentContainer.h"
#include "core/UBApplicationController.h"

class UBMainWindow;
class UBApplication;
class UBBoardView;

class UBDocumentController;
class UBMessageWindow;
class UBGraphicsScene;
class UBDocumentProxy;
class UBBlackoutWidget;
class UBToolWidget;
class UBVersion;
class UBSoftwareUpdate;
class UBSoftwareUpdateDialog;
class UBGraphicsMediaItem;
class UBGraphicsWidgetItem;
class UBBoardPaletteManager;
class UBItem;
class UBGraphicsItem;


class UBBoardController : public UBDocumentContainer
{
    Q_OBJECT

    public:
        enum SaveFlag {
            sf_none = 0x0,
            sf_showProgress = 0x1
        };
    Q_DECLARE_FLAGS(SaveFlags, SaveFlag)

    public:
        UBBoardController(UBMainWindow *mainWindow);
        virtual ~UBBoardController();

        void init();
        void setupLayout();

        UBGraphicsScene* activeScene() const;
        int activeSceneIndex() const;
        void setActiveSceneIndex(int i);
        QSize displayViewport();
        QSize controlViewport();
        QRectF controlGeometry();
        void closing();

        int currentPage();

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

        UBGraphicsScene* activeScene()
        {
            return mActiveScene;
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

        qreal systemScaleFactor()
        {
            return mSystemScaleFactor;
        }
        qreal currentZoom();
        void persistViewPositionOnCurrentScene();
        void persistCurrentScene(bool isAnAutomaticBackup = false, bool forceImmediateSave = false);
        void showNewVersionAvailable(bool automatic, const UBVersion &installedVersion, const UBSoftwareUpdate &softwareUpdate);
        void setBoxing(QRect displayRect);
        void setToolbarTexts();
        static QUrl expandWidgetToTempDir(const QByteArray& pZipedData, const QString& pExtension = QString("wgt"));

        void setPageSize(QSize newSize);
        UBBoardPaletteManager *paletteManager()
        {
            return mPaletteManager;
        }

        void notifyCache(bool visible);
        void notifyPageChanged();
        void displayMetaData(QMap<QString, QString> metadatas);

        void findUniquesItems(const QUndoCommand *parent, QSet<QGraphicsItem *> &itms);
        void ClearUndoStack();

        void setActiveDocumentScene(UBDocumentProxy* pDocumentProxy, int pSceneIndex = 0, bool forceReload = false, bool onImport = false);
        void setActiveDocumentScene(int pSceneIndex);

        void moveSceneToIndex(int source, int target);
        void duplicateScene(int index);
        UBGraphicsItem *duplicateItem(UBItem *item);
        void deleteScene(int index);

        bool cacheIsVisible() {return mCacheWidgetIsEnabled;}

        QString actionGroupText(){ return mActionGroupText;}
        QString actionUngroupText(){ return mActionUngroupText;}

    public slots:
        void showDocumentsDialog();
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
        void clearSceneBackground();
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
        void downloadURL(const QUrl& url, QString contentSourceUrl = QString(), const QPointF& pPos = QPointF(0.0, 0.0), const QSize& pSize = QSize(), bool isBackground = false, bool internalData = false);
        UBItem *downloadFinished(bool pSuccess, QUrl sourceUrl, QUrl contentUrl, QString pHeader,
                                 QByteArray pData, QPointF pPos, QSize pSize,
                                 bool isBackground = false, bool internalData = false);
        void changeBackground(bool isDark, UBPageBackground pageBackground);
        void setToolCursor(int tool);
        void showMessage(const QString& message, bool showSpinningWheel = false);
        void hideMessage();
        void setDisabled(bool disable);
        void setColorIndex(int pColorIndex);
        void removeTool(UBToolWidget* toolWidget);
        void hide();
        void show();
        void setWidePageSize(bool checked);
        void setRegularPageSize(bool checked);
        void stylusToolChanged(int tool);
        void grabScene(const QRectF& pSceneRect);
        UBGraphicsMediaItem* addVideo(const QUrl& pUrl, bool startPlay, const QPointF& pos, bool bUseSource = false);
        UBGraphicsMediaItem* addAudio(const QUrl& pUrl, bool startPlay, const QPointF& pos, bool bUseSource = false);
        UBGraphicsWidgetItem *addW3cWidget(const QUrl& pUrl, const QPointF& pos);
        void adjustDisplayViews();
        void cut();
        void copy();
        void paste();
        void processMimeData(const QMimeData* pMimeData, const QPointF& pPos);
        void moveGraphicsWidgetToControlView(UBGraphicsWidgetItem* graphicWidget);
        void moveToolWidgetToScene(UBToolWidget* toolWidget);
        void addItem();

        void freezeW3CWidgets(bool freeze);
        void freezeW3CWidget(QGraphicsItem* item, bool freeze);
        void startScript();
        void stopScript();

        void saveData(SaveFlags fls = sf_none);

        //void regenerateThumbnails();

    signals:
        void newPageAdded();
        void activeSceneChanged();
        void zoomChanged(qreal pZoomFactor);
        void penColorChanged();
        void controlViewportChanged();
        void backgroundChanged();
        void cacheEnabled();
        void documentReorganized(int index);
        void displayMetadata(QMap<QString, QString> metadata);
        void pageSelectionChanged(int index);
        void centerOnThumbnailRequired(int index);
        void npapiWidgetCreated(const QString &Url);

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

    private slots:
        void autosaveTimeout();
        void appMainModeChanged(UBApplicationController::MainMode);

    private:
        void initBackgroundGridSize();
        void updatePageSizeState();
        void saveViewState();
        int autosaveTimeoutFromSettings();

        UBMainWindow *mMainWindow;
        UBGraphicsScene* mActiveScene;
        int mActiveSceneIndex;
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
        qreal mSystemScaleFactor;
        bool mCleanupDone;
        QMap<QAction*, QPair<QString, QString> > mActionTexts;
        bool mCacheWidgetIsEnabled;
        QGraphicsItem* mLastCreatedItem;
        int mDeletingSceneIndex;
        int mMovingSceneIndex;
        QString mActionGroupText;
        QString mActionUngroupText;

        QTimer *mAutosaveTimer;

    private slots:
        void stylusToolDoubleClicked(int tool);
        void boardViewResized(QResizeEvent* event);
        void documentWillBeDeleted(UBDocumentProxy* pProxy);
        void updateBackgroundActionsState(bool isDark, UBPageBackground pageBackground);
        void colorPaletteChanged();
        void libraryDialogClosed(int ret);
        void lastWindowClosed();
        void onDownloadModalFinished();

};


#endif /* UBBOARDCONTROLLER_H_ */
