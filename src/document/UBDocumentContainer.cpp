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
#include "adaptors/UBThumbnailAdaptor.h"
#include "core/UBPersistenceManager.h"
#include "core/memcheck.h"


UBDocumentContainer::UBDocumentContainer(QObject * parent)
    :QObject(parent)
    ,mCurrentDocument(NULL)
{}

UBDocumentContainer::~UBDocumentContainer()
{

}

void UBDocumentContainer::setDocument(UBDocumentProxy* document, bool forceReload)
{
    if (mCurrentDocument != document || forceReload)
    {
        mCurrentDocument = document;
        emit initThumbnailsRequired(document); //for board mode
        clearThumbPage(); //for document mode
        reloadThumbnails();
        emit documentSet(mCurrentDocument);
    }
}

void UBDocumentContainer::duplicatePages(QList<int>& pageIndexes)
{
    int offset = 0;
    foreach(int sceneIndex, pageIndexes)
    {
        UBPersistenceManager::persistenceManager()->duplicateDocumentScene(mCurrentDocument, sceneIndex + offset);
        offset++;
    }
}

void UBDocumentContainer::duplicatePage(int index)
{
    UBPersistenceManager::persistenceManager()->duplicateDocumentScene(mCurrentDocument, index);
}

void UBDocumentContainer::moveThumbPage(int source, int target)
{
    mDocumentThumbs.move(source, target);

    //on board thumbnails view (UBDocumentNavigator)
    emit documentPageMoved(source, target);

    //on board thumbnails view (UBoardThumbnailsView)
    emit moveThumbnailRequired(source, target);
}

void UBDocumentContainer::deletePages(QList<int>& pageIndexes)
{
    UBPersistenceManager::persistenceManager()->deleteDocumentScenes(mCurrentDocument, pageIndexes);
    int offset = 0;
    foreach(int index, pageIndexes)
    {
        deleteThumbPage(index - offset);
        offset++;
    }
}

void UBDocumentContainer::addPage(int index)
{
    UBPersistenceManager::persistenceManager()->createDocumentSceneAt(mCurrentDocument, index);
    insertThumbPage(index);
}


void UBDocumentContainer::addPixmapAt(std::shared_ptr<QPixmap> pix, int index)
{
    mDocumentThumbs.insert(index, pix);
}


void UBDocumentContainer::clearThumbPage()
{
    mDocumentThumbs.clear();
}

void UBDocumentContainer::initThumbPage()
{
    clearThumbPage();

    for (int i=0; i < selectedDocument()->pageCount(); i++)
        insertThumbPage(i);
}

void UBDocumentContainer::deleteThumbPage(int index)
{
    mDocumentThumbs.removeAt(index);

    //on board thumbnails view (UBDocumentNavigator)
    emit documentPageRemoved(index);

    //on board thumbnails view (UBoardThumbnailsView)
    emit removeThumbnailRequired(index);
}

void UBDocumentContainer::updateThumbPage(int index)
{
    if (mDocumentThumbs.size() > index)
    {
        QPixmap pixmap = UBThumbnailAdaptor::get(mCurrentDocument, index);
        mDocumentThumbs[index] = std::make_shared<QPixmap>(pixmap);

        emit documentPageUpdated(index);
    }
}

void UBDocumentContainer::insertThumbPage(int index)
{
    QPixmap newPixmap = UBThumbnailAdaptor::get(mCurrentDocument, index);
    mDocumentThumbs.insert(index, std::make_shared<QPixmap>(newPixmap));

    emit documentPageInserted(index);
    emit addThumbnailRequired(selectedDocument(), index);
}

void UBDocumentContainer::insertExistingThumbPage(int index, std::shared_ptr<QPixmap> thumbnailPixmap)
{
    mDocumentThumbs.insert(index, thumbnailPixmap);

    emit documentPageInserted(index);
    emit addThumbnailRequired(selectedDocument(), index);
}

void UBDocumentContainer::reloadThumbnails()
{
    emit documentThumbnailsUpdated(this);
}

int UBDocumentContainer::pageFromSceneIndex(int sceneIndex)
{
    return sceneIndex+1;
}

int UBDocumentContainer::sceneIndexFromPage(int page)
{
    return page-1;
}

void UBDocumentContainer::addEmptyThumbPage()
{
    mDocumentThumbs.append(std::shared_ptr<QPixmap>());
}
