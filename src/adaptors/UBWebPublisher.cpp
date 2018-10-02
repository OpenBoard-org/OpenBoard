/*
 * Copyright (C) 2015-2016 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBWebPublisher.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "adaptors/publishing/UBDocumentPublisher.h"

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

    //UniboardSankoreTransition document;
    QString documentPath(pDocumentProxy->persistencePath());
    //document.checkDocumentDirectory(documentPath);

    UBDocumentPublisher* publisher = new UBDocumentPublisher(pDocumentProxy, this); // the publisher will self delete when publication finishes
    publisher->publish();

}

bool UBWebPublisher::associatedActionactionAvailableFor(const QModelIndex &selectedIndex)
{
    const UBDocumentTreeModel *docModel = qobject_cast<const UBDocumentTreeModel*>(selectedIndex.model());
    if (!selectedIndex.isValid() || docModel->isCatalog(selectedIndex)) {
        return false;
    }

    return true;
}


