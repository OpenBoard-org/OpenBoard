/*
 * Copyright (C) 2012 Webdoc SA
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



#include "UBPropertyPalette.h"

#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBPropertyPalette::UBPropertyPalette(QWidget *parent, const char *name):UBActionPalette(parent)
{
    setObjectName(name);
    mbGrip = false;
}

/**
 * \brief Constructor
 * @param orientation as the palette orientation
 * @param parent as the parent widget
 */
UBPropertyPalette::UBPropertyPalette(Qt::Orientation orientation, QWidget *parent):UBActionPalette(orientation, parent)
{
    mbGrip = false;
}

/**
 * \brief Destructor
 */
UBPropertyPalette::~UBPropertyPalette()
{

}

/**
 * \brief Handles the mouse release event
 */
void UBPropertyPalette::onMouseRelease()
{
//    qDebug() << "UBPropertyPalette::onMouseRelease() called (" << mMousePos.x() << "," << mMousePos.y();
//    QWidget* pW = NULL;
//    pW = childAt(mMousePos);

//    if(NULL != pW)
//    {
//        // A widget has been found under the mouse!
//        UBActionPaletteButton* pButton = dynamic_cast<UBActionPaletteButton*>(pW);
//        if(NULL != pButton)
//        {
//            pButton->click();
//        }
//    }

//    // Close the palette
//    close();
}
