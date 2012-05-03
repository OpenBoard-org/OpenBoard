/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "core/UBApplication.h"
#include "globals/UBGlobals.h"

#include "UBDockTeacherGuideWidget.h"
#include "UBTeacherGuideWidget.h"


UBDockTeacherGuideWidget::UBDockTeacherGuideWidget(QWidget* parent, const char* name):
    UBDockPaletteWidget(parent,name)
  , mpTeacherGuideWidget(NULL)
{
    mName = "TeacherGuide";
    mVisibleState = true;
    SET_STYLE_SHEET();

    mIconToLeft = QPixmap(":images/teacher_open.png");
    mIconToRight = QPixmap(":images/teacher_close.png");

    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);
    mpTeacherGuideWidget = new UBTeacherGuideWidget(this);
    mpLayout->addWidget(mpTeacherGuideWidget);
}

UBDockTeacherGuideWidget::~UBDockTeacherGuideWidget()
{
    DELETEPTR(mpTeacherGuideWidget);
    DELETEPTR(mpLayout);
}
