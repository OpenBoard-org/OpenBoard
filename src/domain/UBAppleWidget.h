/*
 * UBAppleWidget.h
 *
 *  Created on: 11 Feb. 2009
 *      Author: Luc
 */

#ifndef UBAPPLEWIDGET_H_
#define UBAPPLEWIDGET_H_

#include <QtGui>
#include <QtWebKit>

#include "UBAbstractWidget.h"

class UBItem;

class UBAppleWidget : public UBAbstractWidget
{
    Q_OBJECT;

    public:
        UBAppleWidget(const QUrl& pWidgetUrl, QWidget *parent = 0);
        virtual ~UBAppleWidget();

};

#endif /* UBAPPLEWIDGET_H_ */
