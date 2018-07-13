/*
 * Copyright (C) 2010-2013 Groupement d'Intt Public pour lEducation Numrique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankor.
 *
 * Open-Sankor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL projects
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankor  If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBColorPicker.h"
#include "UBDrawingStrokePropertiesPalette.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"
#include "UBMainWindow.h"
/*#include "UBGlobals.h"*/


UBDrawingStrokePropertiesPalette::UBDrawingStrokePropertiesPalette(Qt::Orientation orient, QWidget *parent)
    :UBAbstractSubPalette(orient, parent)
{
    hide();    

    // Color button
    mBtnColorPicker = new UBColorPickerButton(this);
    layout()->addWidget(mBtnColorPicker);
    connect(mBtnColorPicker, SIGNAL(clicked()), this, SLOT(onBtnSelectStrokeColor()));

    //layout "thickness"
    QHBoxLayout* thicknessLayout = new QHBoxLayout();
    thicknessLayout->setSpacing(0);

    // Thickness buttons
    UBActionPaletteButton* btnThickness1 = new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyThickness0, this);
    btnThickness1->setStyleSheet(styleSheetLeftGroupedButton);
    mListBtnStrokeThickness.append(btnThickness1);
    mMapBtnStrokeThickness[btnThickness1] = Fine;
    UBApplication::mainWindow->actionStrokePropertyThickness0->setChecked(true);

    UBActionPaletteButton* btnThickness2 = new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyThickness1, this);
    btnThickness2->setStyleSheet(styleSheetCenterGroupedButton);
    mListBtnStrokeThickness.append(btnThickness2);
    mMapBtnStrokeThickness[btnThickness2] = Medium;

    UBActionPaletteButton* btnThickness3 = new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyThickness2, this);
    mListBtnStrokeThickness.append(btnThickness3);
    btnThickness3->setStyleSheet(styleSheetRightGroupedButton);
    mMapBtnStrokeThickness[btnThickness3] = Large;

    //group thickness buttons
    mButtonGroupStrokeThickness = new QButtonGroup(this);
    foreach(UBActionPaletteButton* button, mListBtnStrokeThickness)
    {
        mButtonGroupStrokeThickness->addButton(button);
        thicknessLayout->addWidget(button);
        connect(button, SIGNAL(clicked()), this, SLOT(onBtnSelectThickness()));
    }

    //layout "Style"
    QHBoxLayout* styleLayout = new QHBoxLayout();
    styleLayout->setSpacing(0);

    // Style buttons
    UBActionPaletteButton* btnSolidLine = new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyStyleSolidLine, this);
    mMapBtnStrokeStyle[btnSolidLine] = Qt::SolidLine;
    mListBtnStrokeStyle.append(btnSolidLine);
    btnSolidLine->setStyleSheet(styleSheetLeftGroupedButton);
    UBApplication::mainWindow->actionStrokePropertyStyleSolidLine->setChecked(true);

    UBActionPaletteButton* btnDotLine = new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyStyleDotLine, this);
    mMapBtnStrokeStyle[btnDotLine] = Qt::DotLine;
    mListBtnStrokeStyle.append(btnDotLine);
    btnDotLine->setStyleSheet(styleSheetCenterGroupedButton);

    UBActionPaletteButton* btnCustomDash = new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyStyleCustomDash, this);
    mMapBtnStrokeStyle[btnCustomDash] = Qt::CustomDashLine;
    mListBtnStrokeStyle.append(btnCustomDash);
    btnCustomDash->setStyleSheet(styleSheetRightGroupedButton);

    //group style buttons
    mButtonGroupStrokeStyle = new QButtonGroup(this);
    foreach(UBActionPaletteButton* button, mListBtnStrokeStyle)
    {
        mButtonGroupStrokeStyle->addButton(button);
        styleLayout->addWidget(button);
        connect(button, SIGNAL(clicked()), this, SLOT(onBtnSelectStyle()));
    }

    //group layouts to main layout
    QHBoxLayout* mainLayout = dynamic_cast<QHBoxLayout*>(layout());
    if (mainLayout)
    {
        mainLayout->addLayout(thicknessLayout);
        mainLayout->addLayout(styleLayout);
    }

    adjustSizeAndPosition();
}



UBDrawingStrokePropertiesPalette::~UBDrawingStrokePropertiesPalette()
{
    /*DELETEPTR(mBtnColorPicker);*/
    if(NULL != mBtnColorPicker){
        delete mBtnColorPicker;
        mBtnColorPicker = NULL;
    }

    foreach(UBActionPaletteButton * button, mListBtnStrokeThickness){
        /*DELETEPTR(button);*/
        if(NULL != button){
            delete button;
            button = NULL;
        }
    }
    /*DELETEPTR(mButtonGroupStrokeThickness);*/
    if(NULL != mButtonGroupStrokeThickness){
        delete mButtonGroupStrokeThickness;
        mButtonGroupStrokeThickness = NULL;
    }

    foreach(UBActionPaletteButton * button, mListBtnStrokeStyle){
        /*DELETEPTR(button);*/
        if(NULL != button){
            delete button;
            button = NULL;
        }
    }
    /*DELETEPTR(mButtonGroupStrokeStyle);*/
    if(NULL != mButtonGroupStrokeStyle){
        delete mButtonGroupStrokeStyle;
        mButtonGroupStrokeStyle = NULL;
    }
}

void UBDrawingStrokePropertiesPalette::onBtnSelectStrokeColor()
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

        UBApplication::boardController->shapeFactory().setStrokeColor(selectedColor);
        mBtnColorPicker->setColor(selectedColor); // udpate Color icon in palette.
    }
}

void UBDrawingStrokePropertiesPalette::onBtnSelectThickness()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>(sender());
    if (button && mMapBtnStrokeThickness.contains(button))
    {
        int thickness = mMapBtnStrokeThickness.value(button);
        UBApplication::boardController->shapeFactory().setThickness(thickness);
        //qWarning() << "thickness: " << thickness;
    }
}

void UBDrawingStrokePropertiesPalette::onBtnSelectStyle()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>(sender());
    if (button && mMapBtnStrokeStyle.contains(button))
    {
        Qt::PenStyle style = mMapBtnStrokeStyle.value(button);
        UBApplication::boardController->shapeFactory().setStrokeStyle(style);
    }
}

