/*
 * UBRubberBand.cpp
 *
 *  Created on: 20 mai 2009
 *      Author: Luc
 */

#include "UBRubberBand.h"

#include <QtGui>

#ifdef Q_WS_MAC
#include <QtGui/QMacStyle>
#endif

UBRubberBand::UBRubberBand(Shape s, QWidget * p)
    : QRubberBand(s, p)
{
    QStyle* rubberBandStyle = QRubberBand::style();

#ifdef Q_WS_WIN
    rubberBandStyle = new QWindowsXPStyle();
#elif defined(Q_WS_MAC)
    rubberBandStyle = new QMacStyle();
#endif
    QRubberBand::setStyle(rubberBandStyle);
}

UBRubberBand::~UBRubberBand()
{
    // NOOP
}
