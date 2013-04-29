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



#ifndef UBDOCKTEACHERGUIDEWIDGET_H
#define UBDOCKTEACHERGUIDEWIDGET_H

class QVBoxLayout;
class UBTeacherGuideWidget;

#include "UBDockPaletteWidget.h"


class UBDockTeacherGuideWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBDockTeacherGuideWidget(QWidget* parent=0, const char* name="UBDockTeacherGuideWidget");
    ~UBDockTeacherGuideWidget();

    bool visibleInMode(eUBDockPaletteWidgetMode mode){ return mode == eUBDockPaletteWidget_BOARD; }

    UBTeacherGuideWidget* teacherGuideWidget();

private:
    QVBoxLayout* mpLayout;
    UBTeacherGuideWidget* mpTeacherGuideWidget;
};

#endif // UBDOCKTEACHERGUIDEWIDGET_H
