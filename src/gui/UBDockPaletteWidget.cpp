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



#include "UBDockPaletteWidget.h"

#include "core/memcheck.h"

UBDockPaletteWidget::UBDockPaletteWidget(QWidget *parent, const char *name):QWidget(parent)
{
    setObjectName(name);
}

UBDockPaletteWidget::~UBDockPaletteWidget()
{

}

QPixmap UBDockPaletteWidget::iconToRight()
{
    return mIconToRight;
}

QPixmap UBDockPaletteWidget::iconToLeft()
{
    return mIconToLeft;
}

QString UBDockPaletteWidget::name()
{
    return mName;
}

/**
  * When a widget registers a mode it means that it would be displayed on that mode
  */
void UBDockPaletteWidget::registerMode(eUBDockPaletteWidgetMode mode)
{
    if(!mRegisteredModes.contains(mode))
        mRegisteredModes.append(mode);
}

void UBDockPaletteWidget::slot_changeMode(eUBDockPaletteWidgetMode newMode)
{
    this->setVisible(this->visibleInMode( newMode ));
}


