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


#include "UBDocumentToc.h"

#include <QUuid>

#include "document/UBTocSerializer.h"

static const QString UUID{"uuid"};
static const QString PAGE_ID{"id"};
static const QString ASSETS("assets");

UBDocumentToc::UBDocumentToc(const QString& documentPath)
    : mDocumentPath{documentPath}
{
}

UBDocumentToc::UBDocumentToc(const UBDocumentToc& other, const QString& documentPath)
    : mDocumentPath{documentPath}
    , mVersion{other.mVersion}
    , mToc{other.mToc}
{
}

QVersionNumber UBDocumentToc::version() const
{
    return mVersion;
}

int UBDocumentToc::pageCount() const
{
    return mToc.count();
}

int UBDocumentToc::insert(int index)
{
    if (index >= 0 && index <= mToc.count())
    {
        int pageId = nextAvailablePageId();
        mToc.insert(index, {{PAGE_ID, pageId}});
        mModified = true;
        return pageId;
    }

    return -1;
}

void UBDocumentToc::move(int fromIndex, int toIndex)
{
    if (isValidIndex(fromIndex) && isValidIndex(toIndex))
    {
        auto entry = mToc.at(fromIndex);
        mToc.removeAt(fromIndex);
        mToc.insert(toIndex, entry);
        mModified = true;
    }
}

void UBDocumentToc::remove(int index)
{
    if (isValidIndex(index))
    {
        mToc.remove(index);
        mModified = true;
    }
}

QUuid UBDocumentToc::uuid(int index) const
{
    if (isValidIndex(index))
    {
        return mToc.at(index).value(UUID).toUuid();
    }

    return {};
}

void UBDocumentToc::setUuid(int index, const QUuid& uuid)
{
    if (index < 0)
    {
        return;
    }

    assureSize(index);
    mToc[index][UUID] = uuid;
    mModified = true;
}

int UBDocumentToc::findUuid(const QUuid& sceneUuid) const
{
    for (int i = 0; i < mToc.size(); ++i)
    {
        if (uuid(i) == sceneUuid)
        {
            return i;
        }
    }

    return -1;
}

int UBDocumentToc::pageId(int index) const
{
    if (isValidIndex(index))
    {
        return mToc.at(index).value(PAGE_ID).toInt();
    }

    return -1;
}

void UBDocumentToc::setPageId(int index, int pageId)
{
    if (index < 0)
    {
        return;
    }

    assureSize(index);
    mToc[index][PAGE_ID] = pageId;
    mModified = true;
}

QStringList UBDocumentToc::assets(int index) const
{
    if (isValidIndex(index))
    {
        return mToc.at(index).value(ASSETS).toStringList();
    }

    return {};
}

void UBDocumentToc::setAssets(int index, const QStringList& assets)
{
    if (index < 0)
    {
        return;
    }

    assureSize(index);
    mToc[index][ASSETS] = assets;
    mModified = true;
}

void UBDocumentToc::unsetAssets(int index)
{
    if (isValidIndex(index))
    {
        mToc[index].remove(ASSETS);
    }
}

bool UBDocumentToc::hasAssetsEntry(int index) const
{
    return isValidIndex(index) && mToc.at(index).contains(ASSETS);
}

bool UBDocumentToc::load()
{
    UBTocJsonSerializer serializer(mDocumentPath);
    const auto ok = serializer.load(mVersion, mToc);

    for (const auto& entry : mToc)
    {
        mNextAvailablePageId = std::max(mNextAvailablePageId, entry[PAGE_ID].toInt() + 1);
    }

    mModified = false;
    return ok;
}

void UBDocumentToc::save()
{
    if (mModified)
    {
        UBTocJsonSerializer serializer(mDocumentPath);
        serializer.save(mVersion, mToc);
        mModified = false;
    }
}

int UBDocumentToc::nextAvailablePageId()
{
    return mNextAvailablePageId++;
}

bool UBDocumentToc::isValidIndex(int index) const
{
    return index >= 0 && index < mToc.count();
}

void UBDocumentToc::assureSize(int index)
{
    if (index >= mToc.count())
    {
        mToc.resize(index + 1);
    }
}
