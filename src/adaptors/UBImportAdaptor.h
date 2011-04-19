/*
 * UBImportAdaptor.h
 *
 *  Created on: Feb 11, 2009
 *      Author: julienbachmann
 */

#ifndef UBIMPORTADAPTOR_H_
#define UBIMPORTADAPTOR_H_

#include <QtGui>

class UBDocumentProxy;

class UBImportAdaptor : public QObject
{
    Q_OBJECT;

    protected:
        UBImportAdaptor(QObject *parent = 0);
        virtual ~UBImportAdaptor();

    public:

        virtual QStringList supportedExtentions() = 0;
        virtual QString importFileFilter() = 0;
        virtual UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);
        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile) = 0;
};

#endif /* UBIMPORTADAPTOR_H_ */
