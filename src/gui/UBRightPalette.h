/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBRIGHTPALETTE_H
#define UBRIGHTPALETTE_H

#include "UBDockPalette.h"

class UBRightPalette : public UBDockPalette
{
    Q_OBJECT
public:
    UBRightPalette(QWidget* parent=0, const char* name="UBRightPalette");
    ~UBRightPalette();
    bool switchMode(eUBDockPaletteWidgetMode mode);

signals:
    void resized();

protected:
    void updateMaxWidth();
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);

};

#endif // UBRIGHTPALETTE_H
