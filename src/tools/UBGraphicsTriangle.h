
/*
 * UBGraphicsRuler.h
 *
 *  Created on: April 16, 2009
 *      Author: Jerome Marchaud
 */

#ifndef UBGRAPHICSTRIANGLE_H_
#define UBGRAPHICSTRIANGLE_H_

#include <QtGui>
#include <QtSvg>
#include <QGraphicsPolygonItem>

#include "core/UB.h"
#include "domain/UBItem.h"


class UBGraphicsScene;

class UBGraphicsTriangle : public QObject, public QGraphicsPolygonItem, public UBItem
{
    Q_OBJECT;

    public:
        UBGraphicsTriangle();
        virtual ~UBGraphicsTriangle();
};

#endif /* UBGRAPHICSTRIANGLE_H_ */
