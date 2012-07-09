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

#include "UBWebPublisher.h"

#include "document/UBDocumentProxy.h"

#include "adaptors/publishing/UBDocumentPublisher.h"

#include "transition/UniboardSankoreTransition.h"

#include "core/memcheck.h"

UBWebPublisher::UBWebPublisher(QObject *parent)
    : UBExportAdaptor(parent)
{
    // NOOP
}


UBWebPublisher::~UBWebPublisher()
{
    // NOOP
}


QString UBWebPublisher::exportName()
{
    return tr("Publish Document on Sankore Web");
}


void UBWebPublisher::persist(UBDocumentProxy* pDocumentProxy)
{
    if (!pDocumentProxy)
        return;

    UniboardSankoreTransition document;
    QString documentPath(pDocumentProxy->persistencePath());
    document.checkDocumentDirectory(documentPath);

    UBDocumentPublisher* publisher = new UBDocumentPublisher(pDocumentProxy, this); // the publisher will self delete when publication finishes
    publisher->publish();

}


