/*
 * UBWebPublisher.h
 *
 *  Created on: Feb 19, 2010
 *      Author: Luc
 */

#ifndef UBWEBPUBLISHER_H_
#define UBWEBPUBLISHER_H_

#include <QtGui>

#include "UBExportAdaptor.h"

#include "frameworks/UBFileSystemUtils.h"
#include "ui_webPublishing.h"

class UBDocumentProxy;
class UBServerXMLHttpRequest;

class UBWebPublisher : public UBExportAdaptor
{
    Q_OBJECT;

    public:
        UBWebPublisher(QObject *parent = 0);
        virtual ~UBWebPublisher();

        virtual QString exportName();

        virtual void persist(UBDocumentProxy* pDocument);
};


#endif /* UBWEBPUBLISHER_H_ */
