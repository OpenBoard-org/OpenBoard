/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBDocumentContainer.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"
#include "adaptors/UBThumbnailAdaptor.h"
#include "core/UBPersistenceManager.h"
#include "core/memcheck.h"
#include "document/UBDocument.h"
#include "gui/UBThumbnailScene.h"


UBDocumentContainer::UBDocumentContainer(QObject * parent)
    :QObject(parent)
    ,mCurrentDocument(NULL)
{}

UBDocumentContainer::~UBDocumentContainer()
{

}

void UBDocumentContainer::setDocument(std::shared_ptr<UBDocumentProxy> document, bool forceReload)
{
    if (mCurrentDocument != document || forceReload)
    {
        pureSetDocument(document);
        emit documentSet(document);
    }
}

void UBDocumentContainer::pureSetDocument(std::shared_ptr<UBDocumentProxy> document)
{
    mCurrentDocument = document;
    mActiveDocument = UBDocument::getDocument(mCurrentDocument);
}

std::shared_ptr<UBDocument> UBDocumentContainer::activeDocument()
{
    return mActiveDocument;
}

void UBDocumentContainer::duplicatePage(int index)
{
    auto document = UBDocument::getDocument(selectedDocument());

    document->duplicatePage(index);
}


void UBDocumentContainer::moveSceneToIndex(std::shared_ptr<UBDocumentProxy> proxy, int source, int target)
{
    auto document = UBDocument::getDocument(proxy);

    if (document)
    {
        document->movePage(source, target);
        document->thumbnailScene()->hightlightItem(target);

        proxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBMetadataDcSubsetAdaptor::persist(proxy);
    }
}

void UBDocumentContainer::deletePages(QList<int>& pageIndexes)
{
    mActiveDocument->deletePages(pageIndexes);
}

void UBDocumentContainer::addPage(int index)
{
    mActiveDocument->createPage(index);
}


int UBDocumentContainer::pageFromSceneIndex(int sceneIndex)
{
    return sceneIndex+1;
}

int UBDocumentContainer::sceneIndexFromPage(int page)
{
    return page-1;
}
