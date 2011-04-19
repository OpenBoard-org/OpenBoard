/*
 * UBRubberBand.h
 *
 *  Created on: 20 mai 2009
 *      Author: Luc
 */

#ifndef UBRUBBERBAND_H_
#define UBRUBBERBAND_H_

#include <QRubberBand>

class UBRubberBand : public QRubberBand
{
    Q_OBJECT;

    public:
        UBRubberBand(Shape s, QWidget * p = 0);
        virtual ~UBRubberBand();
};

#endif /* UBRUBBERBAND_H_ */
