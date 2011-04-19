/*
 * UBDrawingController.h
 *
 *  Created on: 17 feb. 2010
 *      Author: Luc
 */

#ifndef UBDRAWINGCONTROLLER_H_
#define UBDRAWINGCONTROLLER_H_

#include <QtCore>

#include "core/UB.h"

class UBDrawingController : public QObject
{
    Q_OBJECT;

    private:
        UBDrawingController(QObject * parent = 0);
        virtual ~UBDrawingController();

    public:

        static UBDrawingController* drawingController();

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

    public slots:

        void setStylusTool(int tool);
        void setLineWidthIndex(int index);
        void setColorIndex(int index);
        void setEraserWidthIndex(int index);

    signals:
        void stylusToolChanged(int tool);
        void colorPaletteChanged();

        void lineWidthIndexChanged(int index);
        void colorIndexChanged(int index);

    private:
        UBStylusTool::Enum mStylusTool;
        UBStylusTool::Enum mLatestDrawingTool;

        static UBDrawingController* sDrawingController;

    private slots:

        void penToolSelected(bool checked);
        void eraserToolSelected(bool checked);
        void markerToolSelected(bool checked);
        void selectorToolSelected(bool checked);
        void handToolSelected(bool checked);
        void zoomInToolSelected(bool checked);
        void zoomOutToolSelected(bool checked);
        void pointerToolSelected(bool checked);
        void lineToolSelected(bool checked);
        void textToolSelected(bool checked);
        void captureToolSelected(bool checked);
};

#endif /* UBDRAWINGCONTROLLER_H_ */
