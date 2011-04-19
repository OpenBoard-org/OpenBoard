#ifndef UBDESKTOPTOOLSPALETTE_H
#define UBDESKTOPTOOLSPALETTE_H

#include <QtGui>

#include "gui/UBActionPalette.h"

class UBDesktopToolsPalette : public UBActionPalette
{
    public:
        UBDesktopToolsPalette(QWidget *parent = 0);
        virtual ~UBDesktopToolsPalette(){}
};

#endif // UBDESKTOPTOOLSPALETTE_H
