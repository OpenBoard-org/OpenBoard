/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
