/*
 * Copyright (C) 2015-2025 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBPageMapper.h"

#include "core/UBPersistenceManager.h"
#include "document/UBDocumentToc.h"

UBPageMapper::UBPageMapper(const QString& documentPath, UBDocumentToc* toc)
    : mDocumentPath{documentPath}
    , mSourceToc{toc}
{
    if (!mTempDir.isValid())
    {
        qWarning() << "UBPageMapper: unable to create temporary directory";
        return;
    }

    auto pm = UBPersistenceManager::persistenceManager();
    mMappedToc = std::unique_ptr<UBDocumentToc>{new UBDocumentToc(*mSourceToc, mTempDir.path())};

    // create page map and target TOC
    for (int index = 0; index < mSourceToc->pageCount(); ++index)
    {
        const auto id = mSourceToc->pageId(index);

        // remove the leading "/" from the scene and thumbnail file names
        mFileMap[pm->sceneFilenameForId(id).mid(1)] = pm->sceneFilenameForId(index).mid(1);
        mFileMap[pm->thumbnailFilenameForId(id).mid(1)] = pm->thumbnailFilenameForId(index).mid(1);

        // set id to be same as index in the mapped TOC
        mMappedToc->setPageId(index, index);
    }

    mMappedToc->save();
}

UBPageMapper::MapResult UBPageMapper::map(const QString& filename) const
{
    if (!mMappedToc)
    {
        return {QFileInfo{mDocumentPath + "/" + filename}, filename};
    }

    if (filename.startsWith("toc."))
    {
        // return the mapped TOC
        return {QFileInfo(mTempDir.path() + "/" + filename), filename};
    }

    const auto mappedFilename = mFileMap.value(filename);

    if (!mappedFilename.isEmpty())
    {
        // return the mapped file
        return {QFileInfo{mDocumentPath + "/" + filename}, mappedFilename};
    }

    // return unmapped file
    return {QFileInfo{mDocumentPath + "/" + filename}, filename};
}
