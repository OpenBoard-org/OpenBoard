/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef UBDRAWINGFILLPROPERTIESPALETTE_H
#define UBDRAWINGFILLPROPERTIESPALETTE_H

#include "UBAbstractSubPalette.h"

class UBDrawingFillPropertiesPalette : public UBAbstractSubPalette
{
    Q_OBJECT

public:
    UBDrawingFillPropertiesPalette(Qt::Orientation orient, QWidget *parent = 0);
    virtual ~UBDrawingFillPropertiesPalette();

private slots:
    void onBtnSelectFillFirstColor();
    void onBtnSelectFillSecondColor();
    void onBtnColorTransparent();
    void onBtnColorFull();
    void onBtnColorGradient();
    void onBtnFillStyleDense();
    void onBtnFillStyleDiag();

private:
    UBColorPickerButton * mBtnColorPicker;
    UBColorPickerButton * mBtnColor2Picker;
    QButtonGroup* mButtonGroupColorStyle;

};

#endif // UBDRAWINGFILLPROPERTIESPALETTE_H
