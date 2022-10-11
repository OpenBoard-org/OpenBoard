/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBUNINOTESWINDOWCONTROLLER_H_
#define UBUNINOTESWINDOWCONTROLLER_H_

#include <QtGui>
#include <QTime>
#include <QTimer>

#include "gui/UBRightPalette.h"

class UBDesktopPalette;
class UBBoardView;
class UBGraphicsScene;
class UBDesktopPropertyPalette;
class UBDesktopPenPalette;
class UBDesktopMarkerPalette;
class UBDesktopEraserPalette;
class UBActionPalette;
class UBMainWindow;
class UBRightPalette;

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
        UBDesktopAnnotationController(QObject *parent, UBRightPalette* rightPalette);
        virtual ~UBDesktopAnnotationController();
        void showWindow();
        void hideWindow();

        UBDesktopPalette *desktopPalette();
        QPainterPath desktopPalettePath() const;
        UBBoardView *drawingView();

        void TransparentWidgetResized();


    public slots:

        void screenLayoutChanged();
        void customCapture();
        void screenCapture();
        void updateShowHideState(bool pEnabled);

        void stylusToolChanged(int tool);
        void updateBackground();

//         void showKeyboard(bool show);
//         void showKeyboard(); //X11 virtual keyboard working only needed

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
        void updateColors();
        void desktopPropertyActionToggled(UBDesktopPropertyPalette* palette, QPoint pos);

        void eraseDesktopAnnotations();

        void switchCursor(int tool);
        void onTransparentWidgetResized();
        void refreshMask();

        void hideOtherPalettes(QAction *);
        void togglePropertyPalette(QAction * action);
        void switchCursor(QAction * action);


    private:

        void setAssociatedPalettePosition(UBActionPalette* palette, QPoint pos);
        void updateMask(bool bTransparent);

        UBDesktopPalette *mDesktopPalette;
        //UBKeyboardPalette *mKeyboardPalette;
        UBDesktopPenPalette* mDesktopPenPalette;
        UBDesktopMarkerPalette* mDesktopMarkerPalette;
        UBDesktopEraserPalette* mDesktopEraserPalette;

        UBRightPalette* mRightPalette;

        bool mWindowPositionInitialized;
        bool mIsFullyTransparent;
        bool mDesktopToolsPalettePositioned;

        int mBoardStylusTool;
        int mDesktopStylusTool;

        QPixmap mMask;

};

#endif /* UBUNINOTESWINDOWCONTROLLER_H_ */
