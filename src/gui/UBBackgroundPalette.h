#ifndef UBBACKGROUNDPALETTE_H
#define UBBACKGROUNDPALETTE_H

#include "gui/UBActionPalette.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"

// forward
class UBBackgroundManager;

class UBBackgroundPalette : public UBActionPalette
{
    Q_OBJECT

    public:
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
        void toggleBackgroundColor(bool checked);
        void changeBackground(bool checked);

    protected:
        virtual void updateLayout();
        void init();
        void createActions();
        void updateActions();

    private:
        QVBoxLayout* mVLayout;
        QHBoxLayout* mTopLayout;
        QHBoxLayout* mBottomLayout;

        QSlider* mSlider;
        QLabel* mSliderLabel;
        QLabel* mLightDarkModeLabel;
        UBActionPaletteButton* mResetDefaultGridSizeButton;
        UBActionPaletteButton* mLightDarkModeSwitch;

        bool mIsDark{false};
        const UBBackgroundRuling* mBackground{nullptr};
};

#endif // UBBACKGROUNDPALETTE_H
