/*
 * Copyright (C) 2010-2014 Groupement d'Intrt Public pour l'Education Numrique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankor.
 *
 * Open-Sankor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankor.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "UBColorPicker.h"
#include "UBAlignObjectPalette.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"
#include "UBMainWindow.h"
#include "UBAbstractSubPalette.h"
/*#include "UBGlobals.h"*/


UBAlignObjectPalette::UBAlignObjectPalette(Qt::Orientation orient, QWidget *parent)
    :UBAbstractSubPalette(orient, parent)
{
    hide();

    UBActionPaletteButton *btnAlignToLeft = new UBActionPaletteButton(UBApplication::mainWindow->actionAlignToLeft, this);
    UBActionPaletteButton *btnAlignVerticalCenter = new UBActionPaletteButton(UBApplication::mainWindow->actionVerticalCenter, this);
    UBActionPaletteButton *btnAlignToRight = new UBActionPaletteButton(UBApplication::mainWindow->actionAlignToRight, this);

    UBActionPaletteButton *btnAlignToTop = new UBActionPaletteButton(UBApplication::mainWindow->actionAlignToTop, this);
    UBActionPaletteButton *btnAlignHorizontalCenter = new UBActionPaletteButton(UBApplication::mainWindow->actionHorizontalCenter, this);
    UBActionPaletteButton *btnAlignToBottom = new UBActionPaletteButton(UBApplication::mainWindow->actionAlignToBottom, this);

    layout()->addWidget(btnAlignToLeft);
    layout()->addWidget(btnAlignVerticalCenter);
    layout()->addWidget(btnAlignToRight);

    layout()->addWidget(btnAlignToTop);
    layout()->addWidget(btnAlignHorizontalCenter);
    layout()->addWidget(btnAlignToBottom);

    connect(btnAlignToLeft, SIGNAL(clicked()), this, SLOT(onBtnAlignToLeft()));
    connect(btnAlignVerticalCenter, SIGNAL(clicked()), this, SLOT(onBtnAlignVerticalCenter()));
    connect(btnAlignToRight, SIGNAL(clicked()), this, SLOT(onBtnAlignToRight()));

    connect(btnAlignToTop, SIGNAL(clicked()), this, SLOT(onBtnAlignToTop()));
    connect(btnAlignHorizontalCenter, SIGNAL(clicked()), this, SLOT(onBtnAlignHorizontalCenter()));
    connect(btnAlignToBottom, SIGNAL(clicked()), this, SLOT(onBtnAlignToBottom()));

    adjustSizeAndPosition();
}



UBAlignObjectPalette::~UBAlignObjectPalette()
{

}

void UBAlignObjectPalette::onBtnAlignToLeft()
{
    mAlignObjectManager.alignToLeft();
}

void UBAlignObjectPalette::onBtnAlignVerticalCenter()
{
    mAlignObjectManager.verticalAlign();
}
void UBAlignObjectPalette::onBtnAlignToRight()
{
    mAlignObjectManager.alignToRight();
}
void UBAlignObjectPalette::onBtnAlignToTop()
{
    mAlignObjectManager.alignToTop();
}
void UBAlignObjectPalette::onBtnAlignHorizontalCenter()
{
    mAlignObjectManager.horizontalAlign();
}

void UBAlignObjectPalette::onBtnAlignToBottom()
{
    mAlignObjectManager.alignToBottom();
}

