#ifndef UBDRAWINGARROWSPROPERTIESPALETTE_H
#define UBDRAWINGARROWSPROPERTIESPALETTE_H

#include "UBAbstractSubPalette.h"

class UBDrawingArrowsPropertiesPalette : public UBAbstractSubPalette
{
    Q_OBJECT

public:
    UBDrawingArrowsPropertiesPalette(Qt::Orientation orient, QWidget *parent = 0);
    virtual ~UBDrawingArrowsPropertiesPalette();

private slots:
    void onBtnStartArrowStyleNone();
    void onBtnStartArrowStyleArrow();
    void onBtnStartArrowStyleRound();
    void onBtnEndArrowStyleNone();
    void onBtnEndArrowStyleArrow();
    void onBtnEndArrowStyleRound();

private:
    QButtonGroup * mButtonGroupStartArrowStyle;
    QButtonGroup * mButtonGroupEndArrowStyle;

};

#endif // UBDRAWINGARROWSPROPERTIESPALETTE_H
