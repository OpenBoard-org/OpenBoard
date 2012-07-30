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
