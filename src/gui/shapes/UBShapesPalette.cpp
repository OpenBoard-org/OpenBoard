/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#include "UBShapesPalette.h"

#include <QtGui>

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "domain/shapes/UBShapeFactory.h"

#include "core/memcheck.h"

UBShapesPalette::UBShapesPalette(Qt::Orientation orient, QWidget* parent)
    : UBAbstractSubPalette(parent, orient)
{
    hide();

    auto shapeActions = UBApplication::boardController->shapeFactory().shapeActions();

    QList<QAction*> actions;

    actions << shapeActions->actionSmartLine;
    actions << shapeActions->actionPolygon;
    actions << shapeActions->actionEllipse;
    actions << shapeActions->actionCircle;
    actions << shapeActions->actionRectangle;
    actions << shapeActions->actionSquare;
    actions << shapeActions->actionRegularPolygon;

    // assign shape types
    shapeActions->actionSmartLine->setProperty("ShapeType", QVariant::fromValue(ShapeType::Line));
    shapeActions->actionPolygon->setProperty("ShapeType", QVariant::fromValue(ShapeType::Polygon));
    shapeActions->actionEllipse->setProperty("ShapeType", QVariant::fromValue(ShapeType::Ellipse));
    shapeActions->actionCircle->setProperty("ShapeType", QVariant::fromValue(ShapeType::Circle));
    shapeActions->actionRectangle->setProperty("ShapeType", QVariant::fromValue(ShapeType::Rectangle));
    shapeActions->actionSquare->setProperty("ShapeType", QVariant::fromValue(ShapeType::Square));
    shapeActions->actionRegularPolygon->setProperty("ShapeType", QVariant::fromValue(ShapeType::RegularPolygon));

    setActions(actions);

    layout()->setSpacing(0);

    adjustSizeAndPosition();

    for (const auto action : actions)
    {
        connect(action, &QAction::triggered, this, [this, action]() { actionActivated(action); });
    }

    // detect deactivation of polygon action to terminate shape
    const auto polygonButton = getButtonFromAction(shapeActions->actionPolygon);
    connect(polygonButton, &QAbstractButton::toggled, this, [this](bool checked){
        if (!checked)
        {
            UBApplication::boardController->shapeFactory().terminateShape();
        }
    });
}

UBShapesPalette::~UBShapesPalette()
{
}

void UBShapesPalette::actionActivated(QAction* action)
{
    auto& shapeFactory = UBApplication::boardController->shapeFactory();
    const auto shapeType = action->property("ShapeType").value<ShapeType>();

    switch (shapeType)
    {
    case ShapeType::Ellipse:
        shapeFactory.createEllipse(true);
        break;

    case ShapeType::Circle:
        shapeFactory.createCircle(true);
        break;

    case ShapeType::Rectangle:
        shapeFactory.createRectangle(true);
        break;

    case ShapeType::Square:
        shapeFactory.createSquare(true);
        break;

    case ShapeType::Line:
        shapeFactory.createLine(true);
        break;

    case ShapeType::RegularPolygon:
        shapeFactory.createRegularPolygon(5);
        break;

    case ShapeType::Polygon:
        shapeFactory.createPolygon(true);
        break;

    default:
        break;
    }

    if (actionPaletteButtonParent()->defaultAction() != action)
    {
        if (!actionPaletteButtonParent()->actions().isEmpty())
        {
            // Change the action shown in the stylus palette :
            for (const auto a : actionPaletteButtonParent()->actions())
            {
                // Remove all previois actions
                actionPaletteButtonParent()->removeAction(a);
            }
        }

        // Associate the new action to the button.
        actionPaletteButtonParent()->setDefaultAction(action);
    }
}
