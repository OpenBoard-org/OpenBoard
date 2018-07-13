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



#include "UBShapesPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/memcheck.h"

#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"

#include "board/UBBoardPaletteManager.h"

UBShapesPalette::UBShapesPalette(Qt::Orientation orient, QWidget *parent )
    : UBAbstractSubPalette(parent, orient)
{
    hide();

    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionEllipse;
    actions << UBApplication::mainWindow->actionCircle;
    actions << UBApplication::mainWindow->actionRectangle;
    actions << UBApplication::mainWindow->actionSquare;

    setActions(actions);
    groupActions();

    layout()->setSpacing(0);
    mButtons.at(0)->setStyleSheet(styleSheetLeftGroupedButton);
    mButtons.at(1)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(2)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(3)->setStyleSheet(styleSheetRightGroupedButton);

    adjustSizeAndPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
}

UBShapesPalette::~UBShapesPalette()
{

}

void UBShapesPalette::buttonClicked()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>(sender());
    if (button)
    {
        QAction * action = button->defaultAction();

        if (action)
        {
            triggerAction(action);

            // Change the Action shown in the DrawingPalette :
            foreach (QAction* a, actionPaletteButtonParent()->actions()) {
                actionPaletteButtonParent()->removeAction(a); // Remove all older actions, in order to let only one action associated to the button.
            }
            actionPaletteButtonParent()->setDefaultAction(action); // Associate the new Action to the Button.
        } 
    }

    hide();
}

void UBShapesPalette::triggerAction(QAction* action)
{
    if (action)
    {
        if (action == UBApplication::mainWindow->actionEllipse){
            UBApplication::boardController->shapeFactory().createEllipse(true);
        }
        else if (action == UBApplication::mainWindow->actionCircle){
            UBApplication::boardController->shapeFactory().createCircle(true);
        }
        else if (action == UBApplication::mainWindow->actionRectangle){
            UBApplication::boardController->shapeFactory().createRectangle(true);
        }
        else if (action == UBApplication::mainWindow->actionSquare){
            UBApplication::boardController->shapeFactory().createSquare(true);
        }
    }
}



