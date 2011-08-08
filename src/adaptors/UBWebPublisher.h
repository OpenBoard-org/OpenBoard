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

#ifndef UBWEBPUBLISHER_H_
#define UBWEBPUBLISHER_H_

#include <QtGui>

#include "UBExportAdaptor.h"

#include "frameworks/UBFileSystemUtils.h"
#include "ui_webPublishing.h"

class UBDocumentProxy;
class UBServerXMLHttpRequest;

class UBWebPublisher : public UBExportAdaptor
{
    Q_OBJECT;

    public:
        UBWebPublisher(QObject *parent = 0);
        virtual ~UBWebPublisher();

        virtual QString exportName();

        virtual void persist(UBDocumentProxy* pDocument);
};


#endif /* UBWEBPUBLISHER_H_ */
