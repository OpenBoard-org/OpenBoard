#ifndef UBABSTRACTCOLORSUBPALETTE_H
#define UBABSTRACTCOLORSUBPALETTE_H

#include "UBActionPalette.h"
#include <QAction>

class UBAbstractColorSubPalette : public UBActionPalette
{
    public :
        UBAbstractColorSubPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);
        UBAbstractColorSubPalette(Qt::Orientation orient, QWidget *parent = 0 );

        virtual void togglePalette();
        QAction* mainAction() { return actions().at(mMainAction); }

        UBActionPaletteButton * actionPaletteButtonParent() const {return mActionPaletteButtonParent;}
        void setActionPaletteButtonParent(UBActionPaletteButton * button){mActionPaletteButtonParent = button;}

        virtual void triggerAction(QAction * action);

    protected :
        int mMainAction;

        UBActionPaletteButton * mActionPaletteButtonParent; // button that opened this subPalette.

        // QWidget interface
protected:
        virtual void focusOutEvent(QFocusEvent *);
        virtual void mouseMoveEvent(QMouseEvent *);

        // EV-7 - CFA - 20140127 : ...
        //unable to use Uniboard.css for custom classes, even with overiding paintEvent as Qt recommands...
        //use .css and setObjectName should be preferable, but no more time, and not enough knowledge...
        static const QString styleSheetLeftGroupedButton;
        static const QString styleSheetCenterGroupedButton;
        static const QString styleSheetRightGroupedButton;
};

class UBColorPickerButton : public QToolButton
{
    public:
        UBColorPickerButton(QWidget * parent = 0);

        QColor color() const {return mColor;}
        void setColor(QColor color) {mColor = color;}

        static const int iconSize = 32;

    // QWidget interface
    protected:
        virtual void paintEvent(QPaintEvent * pe);

    private:
        QColor mColor;

        static const int margin_left = 2;
        static const int margin_top = 5;
        static const int width = 20;
        static const int height = 16;
};


#endif // UBABSTRACTCOLORSUBPALETTE_H
