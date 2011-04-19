
#include "UBDocumentToolsPalette.h"

#include <QtGui>

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"


UBDocumentToolsPalette::UBDocumentToolsPalette(QWidget *parent)
    : UBActionPalette(Qt::TopRightCorner, parent)
{
    QList<QAction*> actions;

     if (UBPlatformUtils::hasVirtualKeyboard())
     {
         actions << UBApplication::mainWindow->actionVirtualKeyboard;
     }

     setActions(actions);
     setButtonIconSize(QSize(42, 42));

     adjustSizeAndPosition();
}


UBDocumentToolsPalette::~UBDocumentToolsPalette()
{
    // NOOP
}

