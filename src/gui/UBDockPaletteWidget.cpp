#include "UBDockPaletteWidget.h"

UBDockPaletteWidget::UBDockPaletteWidget(const char *name, QWidget *parent):QWidget(parent)
  , mpWidget(NULL)
{
    setObjectName(name);

}

UBDockPaletteWidget::~UBDockPaletteWidget()
{
    if(NULL != mpWidget)
    {
        delete mpWidget;
        mpWidget = NULL;
    }
}

QWidget* UBDockPaletteWidget::widget()
{
    return mpWidget;
}

QIcon UBDockPaletteWidget::icon()
{
    return mIcon;
}

QIcon UBDockPaletteWidget::collapsedIcon()
{
    return mCollapsedIcon;
}
 QString UBDockPaletteWidget::name()
 {
     return mName;
 }
