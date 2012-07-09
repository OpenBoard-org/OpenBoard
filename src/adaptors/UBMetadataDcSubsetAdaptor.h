/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
