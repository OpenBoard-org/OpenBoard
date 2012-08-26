/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
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


