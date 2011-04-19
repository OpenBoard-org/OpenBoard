/*
 * UBExportPDF.h
 *
 *  Created on: Nov 3, 2008
 *      Author: luc
 */

#ifndef UBEXPORTPDF_H_
#define UBEXPORTPDF_H_

#include <QtCore>
#include "UBExportAdaptor.h"

class UBDocumentProxy;

class UBExportPDF : public UBExportAdaptor
{
    Q_OBJECT;

    public:
        UBExportPDF(QObject *parent = 0);
        virtual ~UBExportPDF();

        virtual QString exportName();
        virtual QString exportExtention();
        virtual void persist(UBDocumentProxy* pDocument);

        static void persistsDocument(UBDocumentProxy* pDocument, QString filename);
};

#endif /* UBEXPORTPDF_H_ */
