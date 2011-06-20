/*
 * UNWindowController.h
 *
 *  Created on: Jan 15, 2009
 *      Author: julienbachmann
 */

#ifndef UBUNINOTESWINDOWCONTROLLER_H_
#define UBUNINOTESWINDOWCONTROLLER_H_

#include <QtGui>
#include <QTime>
#include <QTimer>

#include "gui/UBLibPalette.h"

class UBDesktopPalette;
class UBDesktopToolsPalette;
class UBBoardView;
class UBGraphicsScene;
class UBDesktopPenPalette;
class UBDesktopMarkerPalette;
class UBDesktopEraserPalette;
class UBActionPalette;
class UBKeyboardPalette;
class UBMainWindow;

#define PROPERTY_PALETTE_TIMER      1000

/**
 * The uninotes controller. This object allocate a uninotes window and implements all the actions corresponding to
 * the uninotes button:
 * - Go to Uninotes
 * - Custom capture
 * - Window capture
 * - Screen capture
 */
class UBDesktopAnnotationController : public QObject
{
    Q_OBJECT;

    public:
        UBDesktopAnnotationController(QObject *parent = 0);
        virtual ~UBDesktopAnnotationController();
        void showWindow();
        void hideWindow();

        UBDesktopPalette *desktopPalette();
        UBBoardView *drawingView();

    public slots:

        void screenLayoutChanged();
        void goToUniboard();
        void customCapture();
        void windowCapture();
        void screenCapture();
        void updateShowHideState(bool pEnabled);

        void close();

        void stylusToolChanged(int tool);
        void updateBackground();

    signals:
        /**
         * This signal is emitted once the screenshot has been performed. This signal is also emitted when user
         * click on go to uniboard button. In this case pCapturedPixmap is an empty pixmap.
         * @param pCapturedPixmap QPixmap corresponding to the capture.
         */
        void imageCaptured(const QPixmap& pCapturedPixmap, bool pageMode);
        void restoreUniboard();

    protected:
        QPixmap getScreenPixmap();

        UBBoardView* mTransparentDrawingView;       
        UBGraphicsScene* mTransparentDrawingScene;

    private slots:
        void desktopToolsActionToogled(bool);
        void desktopPenActionToggled(bool checked);
        void desktopMarkerActionToggled(bool checked);
        void desktopEraserActionToggled(bool checked);
        void eraseDesktopAnnotations();
        void penActionPressed();
        void markerActionPressed();
        void eraserActionPressed();
        void penActionReleased();
        void markerActionReleased();
        void eraserActionReleased();
        void onDesktopPaletteMaximized();
        void onDesktopPaletteMinimize();
        void onTransparentWidgetResized();

    private:
        void setAssociatedPalettePosition(UBActionPalette* palette, const QString& actionName);
        void togglePropertyPalette(UBActionPalette* palette);

        UBDesktopPalette *mDesktopPalette;
        UBDesktopToolsPalette *mDesktopToolsPalette;
        UBDesktopPenPalette* mDesktopPenPalette;
        UBDesktopMarkerPalette* mDesktopMarkerPalette;
        UBDesktopEraserPalette* mDesktopEraserPalette;

        UBLibPalette* mLibPalette;

        QTime mPenHoldTimer;
        QTime mMarkerHoldTimer;
        QTime mEraserHoldTimer;
        QTimer mHoldTimerPen;
        QTimer mHoldTimerMarker;
        QTimer mHoldTimerEraser;

        bool mWindowPositionInitialized;
        bool mIsFullyTransparent;
        bool mDesktopToolsPalettePositioned;
        bool mPendingPenButtonPressed;
        bool mPendingMarkerButtonPressed;
        bool mPendingEraserButtonPressed;
        bool mbArrowClicked;

        int mBoardStylusTool;
        int mDesktopStylusTool;

};

#endif /* UBUNINOTESWINDOWCONTROLLER_H_ */
