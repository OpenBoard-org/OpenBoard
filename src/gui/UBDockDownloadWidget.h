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



#ifndef UBDOCKDOWNLOADWIDGET_H
#define UBDOCKDOWNLOADWIDGET_H

#include <QWidget>
#include <QVBoxLayout>

#include "UBDockPaletteWidget.h"
#include "UBDownloadWidget.h"

class UBDockDownloadWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBDockDownloadWidget(QWidget* parent=0, const char* name="UBDockDownloadWidget");
    ~UBDockDownloadWidget();

    bool visibleInMode(eUBDockPaletteWidgetMode mode)
    {
        return mode == eUBDockPaletteWidget_BOARD;
    }


private:
    QVBoxLayout* mpLayout;
    UBDownloadWidget* mpDLWidget;
};

#endif // UBDOCKDOWNLOADWIDGET_H
