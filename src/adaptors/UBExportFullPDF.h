/*
 * UBExportFullPDF.h
 *
 *  Created on: Aug 21, 2009
 *      Author: Patrick
 */

#ifndef UBExportFullPDF_H_
#define UBExportFullPDF_H_

#include <QtCore>
#include "UBExportAdaptor.h"

class UBDocumentProxy;

class UBExportFullPDF : public UBExportAdaptor
{
    Q_OBJECT;

    public:
        UBExportFullPDF(QObject *parent = 0);
        virtual ~UBExportFullPDF();

        virtual QString exportName();
        virtual QString exportExtention();
        virtual void persist(UBDocumentProxy* pDocument);

        virtual void persistsDocument(UBDocumentProxy* pDocument, QString filename);

    protected:
        void saveOverlayPdf(UBDocumentProxy* pDocumentProxy, QString filename);

    private:
        QRect mDefaultPageRect;
        int mMargin;
        bool mHasPDFBackgrounds;
};

#endif /* UBExportFullPDF_H_ */
