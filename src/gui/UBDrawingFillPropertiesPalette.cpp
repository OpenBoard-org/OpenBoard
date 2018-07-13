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


#include "UBDrawingFillPropertiesPalette.h"

#include "core/UBApplication.h"
#include "UBMainWindow.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"

UBDrawingFillPropertiesPalette::UBDrawingFillPropertiesPalette(Qt::Orientation orient, QWidget *parent)
    :UBAbstractSubPalette(orient, parent)
{
    hide();

    //Alpha button
    UBActionPaletteButton* btnAlphaColor = new UBActionPaletteButton(UBApplication::mainWindow->actionColorStyleTransparent, this);
    layout()->addWidget(btnAlphaColor);

    // ColorPicker button
    mBtnColorPicker = new UBColorPickerButton(this);
    mBtnColorPicker->setToolTip(tr("Select and set filling color"));
    mBtnColorPicker->setColor(UBApplication::boardController->shapeFactory().fillFirstColor());
    layout()->addWidget(mBtnColorPicker);
    connect(mBtnColorPicker, SIGNAL(clicked()), this, SLOT(onBtnSelectFillFirstColor()));    

    //layout "ColorStyle"
    QHBoxLayout* colorStyleLayout = new QHBoxLayout();
    colorStyleLayout->setSpacing(0);

    // ColorType buttons
    UBActionPaletteButton* btnFullColor = new UBActionPaletteButton(UBApplication::mainWindow->actionColorStyleFull, this);
    btnFullColor->setStyleSheet(styleSheetLeftGroupedButton);
    colorStyleLayout->addWidget(btnFullColor);
    UBApplication::mainWindow->actionColorStyleFull->setChecked(true);

    UBActionPaletteButton* btnFillStyleDense = new UBActionPaletteButton(UBApplication::mainWindow->actionFillStyleDense, this);
    btnFillStyleDense->setStyleSheet(styleSheetCenterGroupedButton);
    colorStyleLayout->addWidget(btnFillStyleDense);

    UBActionPaletteButton* btnFillStyleDiag = new UBActionPaletteButton(UBApplication::mainWindow->actionFillStyleDiag, this);
    btnFillStyleDiag->setStyleSheet(styleSheetCenterGroupedButton);
    colorStyleLayout->addWidget(btnFillStyleDiag);

    UBActionPaletteButton* btnGradientColor = new UBActionPaletteButton(UBApplication::mainWindow->actionColorStyleGradient, this);
    btnGradientColor->setStyleSheet(styleSheetRightGroupedButton);
    colorStyleLayout->addWidget(btnGradientColor);

    //group color style buttons
    mButtonGroupColorStyle = new QButtonGroup(this);
    mButtonGroupColorStyle->addButton(btnFillStyleDiag);
    mButtonGroupColorStyle->addButton(btnFillStyleDense);
    mButtonGroupColorStyle->addButton(btnFullColor);
    mButtonGroupColorStyle->addButton(btnGradientColor);

    connect(btnAlphaColor, SIGNAL(clicked()), this, SLOT(onBtnColorTransparent()));
    connect(btnFillStyleDense, SIGNAL(clicked()), this, SLOT(onBtnFillStyleDense()));
    connect(btnFillStyleDiag, SIGNAL(clicked()), this, SLOT(onBtnFillStyleDiag()));
    connect(btnFullColor, SIGNAL(clicked()), this, SLOT(onBtnColorFull()));
    connect(btnGradientColor, SIGNAL(clicked()), this, SLOT(onBtnColorGradient()));

    //group layouts to main layout
    QHBoxLayout* mainLayout = dynamic_cast<QHBoxLayout*>(layout());
    if (mainLayout)
    {
        mainLayout->addLayout(colorStyleLayout);
    }


    // ColorPicker button
    mBtnColor2Picker = new UBColorPickerButton(this);
    mBtnColor2Picker->setToolTip(tr("Select and set second filling color"));
    mBtnColor2Picker->setColor(UBApplication::boardController->shapeFactory().fillSecondColor());
    mBtnColor2Picker->setEnabled(false);
    layout()->addWidget(mBtnColor2Picker);
    connect(mBtnColor2Picker, SIGNAL(clicked()), this, SLOT(onBtnSelectFillSecondColor()));

    adjustSizeAndPosition();
}

UBDrawingFillPropertiesPalette::~UBDrawingFillPropertiesPalette()
{

}

void UBDrawingFillPropertiesPalette::onBtnColorTransparent()
{
    mBtnColor2Picker->setEnabled(false);
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Transparent);
    UBApplication::boardController->shapeFactory().setFillingFirstColor(Qt::transparent);
    mBtnColorPicker->setColor(Qt::transparent);
    mBtnColorPicker->repaint();
}

void UBDrawingFillPropertiesPalette::onBtnFillStyleDense()
{
    mBtnColor2Picker->setEnabled(false);
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Dense);
    UBApplication::boardController->shapeFactory().setFillingFirstColor(mBtnColorPicker->color());
}

void UBDrawingFillPropertiesPalette::onBtnFillStyleDiag()
{
    mBtnColor2Picker->setEnabled(false);
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Diag);
    UBApplication::boardController->shapeFactory().setFillingFirstColor(mBtnColorPicker->color());
}


void UBDrawingFillPropertiesPalette::onBtnColorFull()
{
    mBtnColor2Picker->setEnabled(false);
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Full);
    UBApplication::boardController->shapeFactory().setFillingFirstColor(mBtnColorPicker->color());
}

void UBDrawingFillPropertiesPalette::onBtnColorGradient()
{
    mBtnColor2Picker->setEnabled(true);
    UBApplication::boardController->shapeFactory().setFillType(UBShapeFactory::Gradient);    
    UBApplication::boardController->shapeFactory().updateFillingPropertyOnSelectedItems();
}

void UBDrawingFillPropertiesPalette::onBtnSelectFillFirstColor()
{
    QColorDialog colorPicker(this);
    colorPicker.setOption(QColorDialog::ShowAlphaChannel);

#ifdef Q_WS_MAC
    colorPicker.setOption(QColorDialog::DontUseNativeDialog);
#endif

    if ( colorPicker.exec() )
    {
        QColor selectedColor = colorPicker.selectedColor();
        if (selectedColor.alpha() == 0)
            selectedColor = Qt::transparent;

        UBApplication::boardController->shapeFactory().setFillingFirstColor(selectedColor);
        mBtnColorPicker->setColor(selectedColor); // udpate Color icon in palette.        
        if (UBApplication::boardController->shapeFactory().fillType() == UBShapeFactory::Transparent)
        {
            UBApplication::mainWindow->actionColorStyleFull->setChecked(true);
            onBtnColorFull();
        }
    }
}

void UBDrawingFillPropertiesPalette::onBtnSelectFillSecondColor()
{

    QColorDialog colorPicker(this);
    colorPicker.setOption(QColorDialog::ShowAlphaChannel);

#ifdef Q_WS_MAC
    colorPicker.setOption(QColorDialog::DontUseNativeDialog);
#endif

    if ( colorPicker.exec() )
    {
        QColor selectedColor = colorPicker.selectedColor();
        if (selectedColor.alpha() == 0)
            selectedColor = Qt::transparent;

        UBApplication::boardController->shapeFactory().setFillingSecondColor(selectedColor);
        mBtnColor2Picker->setColor(selectedColor); // udpate Color icon in palette.
    }
}
