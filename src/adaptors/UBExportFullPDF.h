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

        virtual void persistsDocument(UBDocumentProxy* pDocument, const QString& filename);

    protected:
        void saveOverlayPdf(UBDocumentProxy* pDocumentProxy, const QString& filename);

    private:
        float mScaleFactor;
        bool mHasPDFBackgrounds;
};

#endif /* UBExportFullPDF_H_ */
