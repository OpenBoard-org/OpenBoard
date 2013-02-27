/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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
