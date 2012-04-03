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
#ifndef UBExportCFF_H_
#define UBExportCFF_H_

#include <QtCore>

#include "UBExportAdaptor.h"

#include "frameworks/UBFileSystemUtils.h"

class UBDocumentProxy;

class UBExportCFF : public UBExportAdaptor
{
    Q_OBJECT;

public:
    UBExportCFF(QObject *parent = 0);
    virtual ~UBExportCFF();

    virtual QString exportName();
    virtual QString exportExtention();
    virtual void persist(UBDocumentProxy* pDocument);
};

#endif /* UBExportCFF_H_ */