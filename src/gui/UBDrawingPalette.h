#ifndef UBDRAWINGPALETTE_H
#define UBDRAWINGPALETTE_H

#include <QMap>
#include <QButtonGroup>

#include "UBAbstractSubPalette.h"
#include "UBShapesPalette.h"
#include <QActionGroup>
#include "UBRegularShapesPalette.h"

class UBDrawingPalette : public UBActionPalette
{
    Q_OBJECT

    public:

        UBDrawingPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);

        virtual ~UBDrawingPalette();

        void mouseMoveEvent(QMouseEvent *event);

        void initPosition();
        virtual void initSubPalettesPosition();
        QButtonGroup * buttonGroup(){return mButtonGroup;}

        void setVisible(bool checked);
        void updateSubPalettesPosition(const QPoint &delta);
        void hideSubPalettes();

        void stackUnder(QWidget *w);


    private slots:

        void buttonClicked();

    private:
        QMap<UBActionPaletteButton*, UBAbstractSubPalette*> mSubPalettes;

        QTime mActionButtonPressedTime;
        bool mPendingActionButtonPressed;

        UBActionPaletteButton *addActionButton(QAction *action);
public slots:
        void raise();

protected:
        UBActionPaletteButton *addButtonSubPalette(UBAbstractSubPalette *subPalette, QAction *action = 0);
};


#endif // UBDRAWINGPALETTE_H
