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
#include "board/UBBoardController.h"

#include "UBRightPalette.h"


UBRightPalette::UBRightPalette(QWidget *parent, const char *name):UBDockPalette(parent)
  , mpLibWidget(NULL)
{
    setObjectName(name);
    setOrientation(eUBDockOrientation_Right);
    mCollapseWidth = 180;
    mLastWidth = 300;
    resize(UBSettings::settings()->libPaletteWidth->get().toInt(), parentWidget()->height());
    mpLayout->setContentsMargins(2*border() + customMargin(), customMargin(), customMargin(), customMargin());

    // Create and register the widgets
    mpLibWidget = new UBLibWidget(this);
    mpCachePropWidget = new UBCachePropertiesWidget(this);
    registerWidget(mpLibWidget);
    registerWidget(mpCachePropWidget);

    // Add the visible widgets
    addTabWidget(mpLibWidget);

    connectSignals();
}

UBRightPalette::~UBRightPalette()
{
    if(NULL != mpLibWidget)
    {
        delete mpLibWidget;
        mpLibWidget = NULL;
    }
    if(NULL != mpCachePropWidget)
    {
        delete mpCachePropWidget;
        mpCachePropWidget = NULL;
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

void UBRightPalette::updateMaxWidth()
{
    setMaximumWidth((int)((parentWidget()->width() * 2)/3));
    setMaximumHeight(parentWidget()->height());
    setMinimumHeight(parentWidget()->height());
}

//void UBRightPalette::onCacheEnabled()
//{
//    if(mpCachePropWidget->isHidden())
//    {
//        mpCachePropWidget->setVisible(true);
//        // Add the cache tab
//        addTabWidget(mpCachePropWidget);
//    }

//    // Set the cache of the current page as the active one for the properties widget
//    mpCachePropWidget->updateCurrentCache();

//    // Show the cache properties widget
//    for(int i = 0; i < mTabWidgets.size(); i++)
//    {
//        if((NULL != mTabWidgets.at(i)) && ("CachePropWidget" == mTabWidgets.at(i)->name()))
//        {
//            showTabWidget(i);
//            break;
//        }
//    }

//}

//void UBRightPalette::onCacheDisabled()
//{
//    removeTab(mpCachePropWidget->name());
//    mpCachePropWidget->hide();
//}
