/*
 *  UBWebPublisher.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: Luc
 */

#include "UBWebPublisher.h"

#include "document/UBDocumentProxy.h"

#include "adaptors/publishing/UBDocumentPublisher.h"



UBWebPublisher::UBWebPublisher(QObject *parent)
    : UBExportAdaptor(parent)
{
    // NOOP
}


UBWebPublisher::~UBWebPublisher()
{
    // NOOP
}


QString UBWebPublisher::exportName()
{
    return tr("Publish Document on Uniboard Web");
}


void UBWebPublisher::persist(UBDocumentProxy* pDocumentProxy)
{
    if (!pDocumentProxy)
        return;

    UBDocumentPublisher* publisher = new UBDocumentPublisher(pDocumentProxy, this); // the publisher will self delete when publication finishes
    publisher->publish();

}


