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
#include "UBRightPalette.h"

UBRightPalette::UBRightPalette(QWidget *parent, const char *name):UBDockPalette(parent)
  , mpLibWidget(NULL)
{
    setObjectName(name);
    setOrientation(eUBDockOrientation_Right);
    setOrientation(eUBDockOrientation_Right);
    mCollapseWidth = 180;
    mLastWidth = 300;
    resize(UBSettings::settings()->libPaletteWidth->get().toInt(), parentWidget()->height());
    mpLayout->setContentsMargins(2*border() + customMargin(), customMargin(), customMargin(), customMargin());

    mpLibWidget = new UBLibWidget(this);
    addTabWidget(mpLibWidget);
}

UBRightPalette::~UBRightPalette()
{
    if(NULL != mpLibWidget)
    {
        delete mpLibWidget;
        mpLibWidget = NULL;
    }
}

UBLibWidget* UBRightPalette::libWidget()
{
    return mpLibWidget;
}

void UBRightPalette::mouseMoveEvent(QMouseEvent *event)
{
    if(mCanResize)
    {
        UBDockPalette::mouseMoveEvent(event);
    }
    else
    {
        //qDebug() << "Mouse move event detected!" ;
    }
}

void UBRightPalette::resizeEvent(QResizeEvent *event)
{
    UBDockPalette::resizeEvent(event);
    UBSettings::settings()->libPaletteWidth->set(width());
    emit resized();
}

/**
 * \brief Update the maximum width
 */
void UBRightPalette::updateMaxWidth()
{
    setMaximumWidth((int)((parentWidget()->width() * 2)/3));
    setMaximumHeight(parentWidget()->height());
    setMinimumHeight(parentWidget()->height());
}
