#ifndef UBBACKGROUNDPALETTE_H
#define UBBACKGROUNDPALETTE_H

#include "gui/UBActionPalette.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"

class UBBackgroundPalette : public UBActionPalette
{
    Q_OBJECT

    public:

        UBBackgroundPalette(QList<QAction*> actions, QWidget* parent = 0);
        UBBackgroundPalette(QWidget* parent = 0);

        void addAction(QAction *action);
        void setActions(QList<QAction *> actions);
        void clearLayout();


    public slots:
        void showEvent(QShowEvent* event);
        void backgroundChanged();
        void refresh();

    protected slots:
        void sliderValueChanged(int value);
        void defaultBackgroundGridSize();
        void toggleIntermediateLines(bool checked);

    protected:
        virtual void updateLayout();
        void init();


        QVBoxLayout* mVLayout;
        QHBoxLayout* mTopLayout;
        QHBoxLayout* mBottomLayout;

        QSlider* mSlider;
        QLabel* mSliderLabel;
        QLabel* mIntermediateLinesLabel;
        UBActionPaletteButton* mResetDefaultGridSizeButton;
        UBActionPaletteButton* mDrawIntermediateLinesCheckBox;

};

#endif // UBBACKGROUNDPALETTE_H
