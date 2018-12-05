/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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
    foreach(const QPixmap* pm, mDocumentThumbs){
        delete pm;
        pm = NULL;
    }
}

void UBDocumentContainer::setDocument(UBDocumentProxy* document, bool forceReload)
{
    if (mCurrentDocument != document || forceReload)
    {
        mCurrentDocument = document;

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

bool UBDocumentContainer::movePageToIndex(int source, int target)
{
    //on document view
    UBPersistenceManager::persistenceManager()->moveSceneToIndex(mCurrentDocument, source, target);
    deleteThumbPage(source);
    insertThumbPage(target);
    emit documentThumbnailsUpdated(this);
    //on board thumbnails view
    emit moveThumbnailRequired(source, target);
    return true;
}

void UBDocumentContainer::deletePages(QList<int>& pageIndexes)
{
    UBPersistenceManager::persistenceManager()->deleteDocumentScenes(mCurrentDocument, pageIndexes);
    int offset = 0;
    foreach(int index, pageIndexes)
    {
        deleteThumbPage(index - offset);
        emit removeThumbnailRequired(index - offset);
        offset++;

    }
    emit documentThumbnailsUpdated(this);
}

void UBDocumentContainer::addPage(int index)
{
    UBPersistenceManager::persistenceManager()->createDocumentSceneAt(mCurrentDocument, index);
    insertThumbPage(index);

    emit documentThumbnailsUpdated(this);
    emit addThumbnailRequired(this, index);
}


void UBDocumentContainer::addPixmapAt(const QPixmap *pix, int index)
{
    mDocumentThumbs.insert(index, pix);
    emit documentThumbnailsUpdated(this);
}


void UBDocumentContainer::clearThumbPage()
{
    qDeleteAll(mDocumentThumbs);
    mDocumentThumbs.clear();
}

void UBDocumentContainer::initThumbPage()
{
    clearThumbPage();

    for (int i=0; i < selectedDocument()->pageCount(); i++)
        insertThumbPage(i);
}

void UBDocumentContainer::updatePage(int index)
{
    updateThumbPage(index);
    emit documentThumbnailsUpdated(this);
}

void UBDocumentContainer::deleteThumbPage(int index)
{
    mDocumentThumbs.removeAt(index);
}

void UBDocumentContainer::updateThumbPage(int index)
{
    if (mDocumentThumbs.size() > index)
    {
        mDocumentThumbs[index] = UBThumbnailAdaptor::get(mCurrentDocument, index);
        emit documentPageUpdated(index);
    }
    else
    {
        qDebug() << "error [updateThumbPage] : index > mDocumentThumbs' size.";
    }
}

void UBDocumentContainer::insertThumbPage(int index)
{
    mDocumentThumbs.insert(index, UBThumbnailAdaptor::get(mCurrentDocument, index));
}

void UBDocumentContainer::reloadThumbnails()
{
    if (mCurrentDocument)
    {
        UBThumbnailAdaptor::load(mCurrentDocument, mDocumentThumbs);
    }
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
    const QPixmap* pThumb = new QPixmap();
    mDocumentThumbs.append(pThumb);
}
