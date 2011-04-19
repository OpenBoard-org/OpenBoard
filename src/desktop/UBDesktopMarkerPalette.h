#ifndef UBDESKTOPMARKERPALETTE_H
#define UBDESKTOPMARKERPALETTE_H

#include <QtGui>

#include "gui/UBPropertyPalette.h"

class UBDesktopMarkerPalette : public UBPropertyPalette
{
    public:
        UBDesktopMarkerPalette(QWidget *parent = 0);
        virtual ~UBDesktopMarkerPalette(){}
};

#endif // UBDESKTOPMARKERPALETTE_H
