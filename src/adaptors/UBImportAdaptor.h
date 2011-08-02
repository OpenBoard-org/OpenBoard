/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
