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

#ifndef UBIMPORTCFF_H
#define UBIMPORTCFF_H

#include <QtGui>
#include "UBImportAdaptor.h"

class UBDocumentProxy;

class UBImportCFF : public UBImportAdaptor
{
    Q_OBJECT;

    public:
        UBImportCFF(QObject *parent = 0);
        virtual ~UBImportCFF();

        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();

        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);

        //base class method override
        virtual UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);

    private:

        virtual QString expandFileToDir(const QFile& pZipFile, const QString& pDir);
};

#endif // UBIMPORTCFF_H
