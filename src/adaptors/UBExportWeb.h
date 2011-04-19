/*
 * UBExportWeb.h
 *
 *  Created on: Feb 19, 2010
 *      Author: Luc
 */

#ifndef UBEXPORTWEB_H_
#define UBEXPORTWEB_H_

#include <QtCore>

#include "UBExportAdaptor.h"

class UBDocumentProxy;

class UBExportWeb : public UBExportAdaptor
{
    Q_OBJECT;

    public:
        UBExportWeb(QObject *parent = 0);
        virtual ~UBExportWeb();

        virtual QString exportName();

        virtual void persist(UBDocumentProxy* pDocument);

};

#endif /* UBEXPORTWEB_H_ */
