
#include "UBDocumentToolsPalette.h"

#include <QtGui>

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"


#include "core/memcheck.h"

UBDocumentToolsPalette::UBDocumentToolsPalette(QWidget *parent)
    : UBActionPalette(Qt::TopRightCorner, parent)
{
    QList<QAction*> actions;

    setActions(actions);
    setButtonIconSize(QSize(42, 42));

    adjustSizeAndPosition();
}


UBDocumentToolsPalette::~UBDocumentToolsPalette()
{
    // NOOP
}

