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



#include "UBDrawingPalette.h"

#include <QtGui>

#include "UBDrawingPolygonPalette.h"
/*#include "UBAlignObjectPalette.h"*/
#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include <QActionGroup>
#include "domain/UBEditableGraphicsRegularShapeItem.h"
#include "UBDrawingStrokePropertiesPalette.h"
#include "UBDrawingFillPropertiesPalette.h"
#include "UBDrawingArrowsPropertiesPalette.h"
#include "gui/UBAlignObjectPalette.h"

UBDrawingPalette::UBDrawingPalette(QWidget *parent, Qt::Orientation orient)
    : UBActionPalette(Qt::TopLeftCorner, parent, orient)
{
    mCustomPosition = true;

    UBActionPaletteButton * btnSubPaletteShape = addButtonSubPalette(new UBShapesPalette(Qt::Horizontal, parentWidget()));
    UBActionPaletteButton * btnSubPaletteRegularShape = addButtonSubPalette(new UBRegularShapesPalette(Qt::Horizontal, parentWidget()));
    UBActionPaletteButton * btnSubPalettePolygon = addButtonSubPalette(new UBDrawingPolygonPalette(Qt::Horizontal, parentWidget()));
    addButtonSubPalette(new UBDrawingStrokePropertiesPalette(Qt::Horizontal, parentWidget()), UBApplication::mainWindow->actionStrokeProperties);
    addButtonSubPalette(new UBDrawingFillPropertiesPalette(Qt::Horizontal, parentWidget()), UBApplication::mainWindow->actionFillProperties);
    addButtonSubPalette(new UBDrawingArrowsPropertiesPalette(Qt::Horizontal, parentWidget()), UBApplication::mainWindow->actionArrowProperties);
    UBActionPaletteButton * btnPaintBucket = addActionButton(UBApplication::mainWindow->actionChangeFillingColor);

    //Sankore-1701 - NNE - 20140324
    addButtonSubPalette(new UBAlignObjectPalette(Qt::Horizontal, parentWidget()), UBApplication::mainWindow->actionAlignObject);

    // Some of those buttons are grouped :
    mButtonGroup = new QButtonGroup(this);
    mButtonGroup->addButton(btnSubPaletteShape);
    mButtonGroup->addButton(btnSubPaletteRegularShape);
    mButtonGroup->addButton(btnSubPalettePolygon);
    mButtonGroup->addButton(btnPaintBucket);    

    initSubPalettesPosition();

    adjustSizeAndPosition();
}



UBActionPaletteButton * UBDrawingPalette::addButtonSubPalette(UBAbstractSubPalette * subPalette, QAction* action)
{
    UBActionPaletteButton * button = 0;
    if (action == 0 && subPalette->actions().size()>0)
    {
        action = subPalette->actions()[0];
    }

    if (action != 0)
    {
        button = new UBActionPaletteButton(action, this);
        button->setToolButtonStyle(mToolButtonStyle);
        subPalette->setActionPaletteButtonParent(button);
        mSubPalettes[button] = subPalette;
        layout()->addWidget(button);
        layout()->setAlignment(button,Qt::AlignHCenter | Qt::AlignVCenter);
        connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
    return button;
}

UBActionPaletteButton * UBDrawingPalette::addActionButton(QAction * action)
{
    UBActionPaletteButton * actionButton = new UBActionPaletteButton(action);

    layout()->addWidget(actionButton);
    layout()->setAlignment(actionButton, Qt::AlignHCenter | Qt::AlignVCenter);

    connect(action, SIGNAL(triggered()), this, SLOT(buttonClicked()));

    return actionButton;
}

UBDrawingPalette::~UBDrawingPalette()
{

}

void UBDrawingPalette::buttonClicked()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>( sender() );
    if (button && mSubPalettes.contains(button))
    {
        UBAbstractSubPalette * subPalette = mSubPalettes.value(button);

        if (subPalette->isHidden())
        {
            initSubPalettesPosition();
            subPalette->togglePalette(); // Show palette
            subPalette->triggerAction(button->defaultAction());
        }
        else
        {
            subPalette->hide(); // Hide palette
        }
    }
    else if (sender() == UBApplication::mainWindow->actionChangeFillingColor)
    {
        UBApplication::boardController->shapeFactory().prepareChangeFill();
    }

    // On any click on this palette's buttons, ends currently drawing shape.
    UBApplication::boardController->shapeFactory().terminateShape();
}


void UBDrawingPalette::initPosition()
{
    // Rem : positions would be very different if drawingPalette were horizontal...

    int x = 0;
    int y = 0;

    // The drawingPalette appears near the button that open it.
    // Find the "Drawing" button :
    UBStylusPalette * stylusPalette = UBApplication::boardController->paletteManager()->stylusPalette();
    int indexDrawingButton = stylusPalette->actions().indexOf(UBApplication::mainWindow->actionDrawing);
    QAction * actionDrawing = stylusPalette->actions().at(indexDrawingButton);
    QList<QWidget *> buttonsDrawing = actionDrawing->associatedWidgets();
    if (buttonsDrawing.size()>0)
    {
        QWidget* buttonDrawing = buttonsDrawing.first();
        if (buttonDrawing)
        {
            if (stylusPalette->orientation() == Qt::Horizontal)
            {
                x = stylusPalette->pos().x() + buttonDrawing->pos().x() + buttonDrawing->width()/2 - this->width()/2;
                y = stylusPalette->pos().y() - this->height();
            }
            else // stylus Palette is vertical :
            {
                x = stylusPalette->pos().x() - this->width();
                y = stylusPalette->pos().y() + buttonDrawing->pos().y() + buttonDrawing->height()/2 - this->width()/2;
            }
        }
    }

    moveInsideParent(QPoint(x, y));

    initSubPalettesPosition(); // place the subPalettes next to the palette.
}

void UBDrawingPalette::initSubPalettesPosition()
{
    foreach(UBActionPaletteButton* button, mSubPalettes.keys())
    {
        UBAbstractSubPalette* subPalette = mSubPalettes.value(button);

        // Depending on position of palette,
        int x = this->x() + this->width(); // place subPalette on the right of the palette ...
        if (this->x() > parentWidget()->width()/2)
        {
            x = this->x() - subPalette->width(); // ... or on the left on the palette.
        }
        // Align vertically the center of subPalette to center of button :
        int y = this->y() + button->y() + button->height()/2 - subPalette->height()/2;

        subPalette->move(x, y);
    }
}

void UBDrawingPalette::hideSubPalettes()
{
    foreach(UBAbstractSubPalette* subPalette, mSubPalettes.values())
    {
        subPalette->hide();
    }
}

void UBDrawingPalette::setVisible(bool checked)
{
    UBActionPalette::setVisible(checked);

    if ( ! checked)
    {
        foreach(UBAbstractSubPalette* subPalette, mSubPalettes.values())
        {
            subPalette->hide();
        }
    }
}

void UBDrawingPalette::mouseMoveEvent(QMouseEvent *event)
{
    QPoint oldPos = this->pos();

    UBActionPalette::mouseMoveEvent(event);

    QPoint delta = this->pos() - oldPos;
    updateSubPalettesPosition(delta);
}

void UBDrawingPalette::updateSubPalettesPosition(const QPoint& delta)
{
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes.values()) {
        QPoint newPos = subPalette->pos() + delta;
        subPalette->move(newPos);
    }
}

void UBDrawingPalette::stackUnder(QWidget * w)
{
    UBActionPalette::stackUnder(w);

    // For all subpalettes :
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes.values()) {
        subPalette->stackUnder(w);
    }
}

void UBDrawingPalette::raise()
{
    UBActionPalette::raise();

    // For all subpalettes :
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes.values()) {
        subPalette->raise();
    }
}

