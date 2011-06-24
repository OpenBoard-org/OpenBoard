/*
 * UBImportDocument.h
 *
 *  Created on: Feb 11, 2009
 *      Author: julienbachmann
 */

#ifndef UBIMPORTDOCUMENT_H_
#define UBIMPORTDOCUMENT_H_

#include <QtGui>
#include "UBImportAdaptor.h"

class UBDocumentProxy;

class UBImportDocument : public UBImportAdaptor
{
    Q_OBJECT;

    public:
        UBImportDocument(QObject *parent = 0);
        virtual ~UBImportDocument();


        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();

        virtual UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);
        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);

    private:
        virtual QString expandFileToDir(const QFile& pZipFile, const QString& pDir);
};

#endif /* UBIMPORTDOCUMENT_H_ */
