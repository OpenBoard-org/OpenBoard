/*
 * UBExportDocument.h
 *
 *  Created on: Feb 10, 2009
 *      Author: julienbachmann
 */

#ifndef UBEXPORTDOCUMENT_H_
#define UBEXPORTDOCUMENT_H_

#include <QtCore>

#include "UBExportAdaptor.h"

#include "frameworks/UBFileSystemUtils.h"

class UBDocumentProxy;


class UBExportDocument : public UBExportAdaptor, public UBProcessingProgressListener
{
    Q_OBJECT;

    public:
        UBExportDocument(QObject *parent = 0);
        virtual ~UBExportDocument();

        virtual QString exportName();
        virtual QString exportExtention();
        virtual void persist(UBDocumentProxy* pDocument);

        virtual void persistsDocument(UBDocumentProxy* pDocument, QString filename);

        virtual void processing(const QString& pObjectName, int pCurrent, int pTotal);
};

#endif /* UBEXPORTDOCUMENT_H_ */
