/*
 * UBFavoriteToolPalette.h
 *
 *  Created on: 30 nov. 2009
 *      Author: Luc
 */

#ifndef UBFAVORITETOOLPALETTE_H_
#define UBFAVORITETOOLPALETTE_H_

#include <QtGui>

#include "UBActionPalette.h"

class UBFavoriteToolPalette : public UBActionPalette
{
    Q_OBJECT;

    public:
        UBFavoriteToolPalette(QWidget* parent = 0);
        virtual ~UBFavoriteToolPalette();

    private slots:
        void addFavorite();
};

#endif /* UBFAVORITETOOLPALETTE_H_ */
