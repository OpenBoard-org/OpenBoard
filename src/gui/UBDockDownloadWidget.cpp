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
#include "UBDockDownloadWidget.h"
#include "core/UBApplication.h"

#include "globals/UBGlobals.h"

UBDockDownloadWidget::UBDockDownloadWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent, name)
  , mpLayout(NULL)
  , mpDLWidget(NULL)
{
    mName = "DownloadWidget";
    mVisibleState = false;

    SET_STYLE_SHEET();

    mIconToLeft = QPixmap(":images/download_open.png");
    mIconToRight = QPixmap(":images/download_close.png");

    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpDLWidget = new UBDownloadWidget(this);
    mpLayout->addWidget(mpDLWidget);
}

UBDockDownloadWidget::~UBDockDownloadWidget()
{
    if(NULL != mpDLWidget)
    {
        delete mpDLWidget;
        mpDLWidget = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}
