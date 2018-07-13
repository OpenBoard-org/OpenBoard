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

#include "UBRegularShapesPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/memcheck.h"

UBRegularShapesPalette::UBRegularShapesPalette(QWidget *parent, Qt::Orientation orient)
    : UBAbstractSubPalette(parent, orient)
{
    hide();

    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionRegularTriangle;
    actions << UBApplication::mainWindow->actionRegularSquare;
    actions << UBApplication::mainWindow->actionRegularPentagone;
    actions << UBApplication::mainWindow->actionRegularHexagone;
    actions << UBApplication::mainWindow->actionRegularHeptagone;
    actions << UBApplication::mainWindow->actionRegularOctogone;            

    setActions(actions);

    groupActions();

    layout()->setSpacing(0);
    mButtons.at(0)->setStyleSheet(styleSheetLeftGroupedButton);
    mButtons.at(1)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(2)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(3)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(4)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(5)->setStyleSheet(styleSheetRightGroupedButton);

    adjustSizeAndPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
}

UBRegularShapesPalette::UBRegularShapesPalette(Qt::Orientation orient, QWidget *parent )
    : UBAbstractSubPalette(parent, orient)
{
    hide();

    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionRegularTriangle;
    actions << UBApplication::mainWindow->actionRegularSquare;
    actions << UBApplication::mainWindow->actionRegularPentagone;
    actions << UBApplication::mainWindow->actionRegularHexagone;
    actions << UBApplication::mainWindow->actionRegularHeptagone;
    actions << UBApplication::mainWindow->actionRegularOctogone;

    setActions(actions);
    groupActions();

    layout()->setSpacing(0);
    mButtons.at(0)->setStyleSheet(styleSheetLeftGroupedButton);
    mButtons.at(1)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(2)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(3)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(4)->setStyleSheet(styleSheetCenterGroupedButton);
    mButtons.at(5)->setStyleSheet(styleSheetRightGroupedButton);

    adjustSizeAndPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
}

UBRegularShapesPalette::~UBRegularShapesPalette()
{

}

void UBRegularShapesPalette::buttonClicked()
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


void UBRegularShapesPalette::triggerAction(QAction *action)
{
    if (action)
    {
        if (action == UBApplication::mainWindow->actionRegularTriangle){
            UBApplication::boardController->shapeFactory().createRegularPolygon(Triangle);
        }
        else if (action == UBApplication::mainWindow->actionRegularSquare){
            UBApplication::boardController->shapeFactory().createSquare(true);
        }
        else if (action == UBApplication::mainWindow->actionRegularPentagone){
            UBApplication::boardController->shapeFactory().createRegularPolygon(Pentagone);
        }
        else if (action == UBApplication::mainWindow->actionRegularHexagone){
            UBApplication::boardController->shapeFactory().createRegularPolygon(Hexagone);
        }
        else if (action == UBApplication::mainWindow->actionRegularHeptagone){
            UBApplication::boardController->shapeFactory().createRegularPolygon(Heptagone);
        }
        else if (action == UBApplication::mainWindow->actionRegularOctogone){
            UBApplication::boardController->shapeFactory().createRegularPolygon(Octogone);
        }
    }
}
