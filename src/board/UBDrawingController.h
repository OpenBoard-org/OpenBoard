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




#ifndef UBDRAWINGCONTROLLER_H_
#define UBDRAWINGCONTROLLER_H_

#include <QtCore>

#include "core/UB.h"

class UBAbstractDrawRuler;

class UBDrawingController : public QObject
{
    Q_OBJECT;

    private:
        UBDrawingController(QObject * parent = 0);
        virtual ~UBDrawingController();

    public:

        static UBDrawingController* drawingController();
        static void destroy();

        int stylusTool();
        int latestDrawingTool();

        bool isDrawingTool();

        int currentToolWidthIndex();
        qreal currentToolWidth();
        int currentToolColorIndex();
        QColor currentToolColor();
        QColor toolColor(bool onDarkBackground);

        void setPenColor(bool onDarkBackground, const QColor& color, int pIndex);
        void setMarkerColor(bool onDarkBackground, const QColor& color, int pIndex);
        void setMarkerAlpha(qreal alpha);

        UBAbstractDrawRuler* mActiveRuler;

        void setInDesktopMode(bool mode){
            mIsDesktopMode = mode;
        }

        bool isInDesktopMode(){
            return mIsDesktopMode;
        }

    public slots:

        void setStylusTool(int tool);
        void setLineWidthIndex(int index);
        void setColorIndex(int index);
        void setEraserWidthIndex(int index);

    signals:
        void stylusToolChanged(int tool, int previousTool = -1);
        void colorPaletteChanged();

        void lineWidthIndexChanged(int index);
        void colorIndexChanged(int index);

    private:
        UBStylusTool::Enum mStylusTool;
        UBStylusTool::Enum mLatestDrawingTool;
        bool mIsDesktopMode;

        static UBDrawingController* sDrawingController;

    private slots:

        void penToolSelected(bool checked);
        void eraserToolSelected(bool checked);
        void markerToolSelected(bool checked);
        void selectorToolSelected(bool checked);
        void playToolSelected(bool checked);
        void handToolSelected(bool checked);
        void zoomInToolSelected(bool checked);
        void zoomOutToolSelected(bool checked);
        void pointerToolSelected(bool checked);
        void lineToolSelected(bool checked);
        void textToolSelected(bool checked);
        void captureToolSelected(bool checked);
};

#endif /* UBDRAWINGCONTROLLER_H_ */
