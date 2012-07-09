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

#ifndef UBEXPORTWEB_H_
#define UBEXPORTWEB_H_

#include <QtCore>

#include "UBExportAdaptor.h"

class UBDocumentProxy;

class UBExportWeb : public UBExportAdaptor
{
    Q_OBJECT;

    public:
        UBExportWeb(QObject *parent = 0);
        virtual ~UBExportWeb();

        virtual QString exportName();

        virtual void persist(UBDocumentProxy* pDocument);

};

#endif /* UBEXPORTWEB_H_ */
