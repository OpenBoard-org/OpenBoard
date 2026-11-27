/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBStorageIndicator.h"

#include "core/UBDocumentStorage.h"
#include "core/UBPersistenceManager.h"

UBStorageIndicator::UBStorageIndicator(QWidget* parent, QWidget* anchor, QWidget* leftNeighbour)
    : UBNotification(parent, anchor, leftNeighbour)
{
    UBDocumentStorage* storage = UBPersistenceManager::persistenceManager()->documentStorage();
    connect(storage, &UBDocumentStorage::availabilityChanged, this, &UBStorageIndicator::setStorageAvailable);
    setStorageAvailable(storage->isAvailable());
}

void UBStorageIndicator::setStorageAvailable(bool available)
{
    if (!available)
    {
        mUnavailable = true;
        showWarning(tr("Document location unavailable"),
                    tr("OpenBoard tries again automatically. Your changes are saved on this computer."),
                    tr("They are copied to the document location as soon as it is available again. "
                       "You can continue working normally.\n\n"
                       "Click to close this message."));
    }
    else if (mUnavailable)
    {
        mUnavailable = false;
        showInformation(tr("Document location available again"), tr("All your changes are saved."));
    }
    else
    {
        hide();
    }
}
