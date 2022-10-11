/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBStylusPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/memcheck.h"

UBStylusPalette::UBStylusPalette(QWidget *parent, Qt::Orientation orient)
    : UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mLastSelectedId(-1)
{
    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionPen;
    actions << UBApplication::mainWindow->actionEraser;
    actions << UBApplication::mainWindow->actionMarker;
    actions << UBApplication::mainWindow->actionSelector;
    actions << UBApplication::mainWindow->actionPlay;

    actions << UBApplication::mainWindow->actionHand;
    actions << UBApplication::mainWindow->actionZoomIn;
    actions << UBApplication::mainWindow->actionZoomOut;

    actions << UBApplication::mainWindow->actionPointer;
    actions << UBApplication::mainWindow->actionLine;
    actions << UBApplication::mainWindow->actionText;
    actions << UBApplication::mainWindow->actionCapture;

    if(UBPlatformUtils::hasVirtualKeyboard())
        actions << UBApplication::mainWindow->actionVirtualKeyboard;

    setActions(actions);
    setButtonIconSize(QSize(42, 42));

    if(!UBPlatformUtils::hasVirtualKeyboard())
    {
        groupActions();
    }
    else
    {
        // VirtualKeyboard action is not in group
        // So, groupping all buttons, except last
        mButtonGroup = new QButtonGroup(this);
        for(int i=0; i < mButtons.size()-1; i++)
        {
            mButtonGroup->addButton(mButtons[i], i);
        }
        connect(mButtonGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked),
                this, &UBActionPalette::buttonGroupClicked);
    }

    adjustSizeAndPosition();

    initPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(doubleClicked()), this, SLOT(stylusToolDoubleClicked()));
    }

}

void UBStylusPalette::initPosition()
{
    QWidget* pParentW = parentWidget();
    if(!pParentW) return ;

    mCustomPosition = true;

    QPoint pos;
    int parentWidth = pParentW->width();
    int parentHeight = pParentW->height();

    if(UBSettings::settings()->appToolBarOrientationVertical->get().toBool()){
        int posX = border();
        int posY = (parentHeight / 2) - (height() / 2);
        pos.setX(posX);
        pos.setY(posY);
    }
    else {
        int posX = (parentWidth / 2) - (width() / 2);
        int posY = parentHeight - border() - height();
        pos.setX(posX);
        pos.setY(posY);
    }
    moveInsideParent(pos);
}

UBStylusPalette::~UBStylusPalette()
{

}

void UBStylusPalette::stylusToolDoubleClicked()
{
    emit stylusToolDoubleClicked(mButtonGroup->checkedId());
}


