/*
 * UBStroke.h
 *
 *  Created on: 28 sept. 2009
 *      Author: Luc
 */

#ifndef UBGRAPHICSSTROKE_H_
#define UBGRAPHICSSTROKE_H_

#include <QtGui>

#include "core/UB.h"


class UBGraphicsPolygonItem;

class UBGraphicsStroke
{
    public:
        UBGraphicsStroke();
        virtual ~UBGraphicsStroke();

        bool hasPressure();

        void addPolygon(UBGraphicsPolygonItem* pol);

        QList<UBGraphicsPolygonItem*> polygons() const;

        UBGraphicsStroke *deepCopy();

        bool hasAlpha() const;

    private:

        QList<UBGraphicsPolygonItem*> mPolygons;

};

#endif /* UBGRAPHICSSTROKE_H_ */
