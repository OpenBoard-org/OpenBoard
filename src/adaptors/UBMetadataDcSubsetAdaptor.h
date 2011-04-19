/*
 * UBMetadataDcSubsetAdaptor.h
 *
 *  Created on: Nov 27, 2008
 *      Author: luc
 */

#ifndef UBMETADATADCSUBSETADAPTOR_H_
#define UBMETADATADCSUBSETADAPTOR_H_

#include <QtGui>

class UBDocumentProxy;

class UBMetadataDcSubsetAdaptor
{
    public:
        UBMetadataDcSubsetAdaptor();
        virtual ~UBMetadataDcSubsetAdaptor();

        static void persist(UBDocumentProxy* proxy);
        static QMap<QString, QVariant> load(QString pPath);

        static const QString nsRdf;
        static const QString nsDc;
        static const QString metadataFilename;

        static const QString rdfIdentifierDomain;

};

#endif /* UBMETADATADCSUBSETADAPTOR_H_ */
