#ifndef UBCANVASPALETTE_H
#define UBCANVASPALETTE_H


#include <QButtonGroup>

#include "gui/UBActionPalette.h"


class UBCanvasPalette : public UBActionPalette
{
    Q_OBJECT

    public:
        UBCanvasPalette(QWidget *parent = 0, int id = 0, Qt::Orientation orient = Qt::Vertical);
        virtual ~UBCanvasPalette();

        void initPosition();
        void movePalette(int id, int w, int h);

        QButtonGroup * buttonGroup(){return mButtonGroup;}

        QAction *actionCPen;
        QAction *actionCEraser;
        QAction *actionCSave;
        QAction *actionCEraseRegion;
        int idPalette;

    public slots:
        void canvasToolDoubleClicked();
        void bgClicked(bool id);
        void eraserToolSelected(bool checked);
        void penToolSelected(bool checked);
        void saveToolSelected(bool checked);
        void eraseRegionToolSelected(bool checked);

    private:
        int mLastSelectedId;

    signals:
        void canvasToolDoubleClicked(int tool);
        void penSelected(int id);
        void eraserSelected(int id);
        void saveSelected(int id);
        void eraseRegionSelected(int id);

};

#endif // UBCANVASPALETTE_H
