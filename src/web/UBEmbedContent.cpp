/*
 * Copyright (C) 2021 Département de l'Instruction Publique (DIP-SEM)
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

#include "UBEmbedContent.h"

#include "core/memcheck.h"

UBEmbedContent::UBEmbedContent()
    : mType(UBEmbedType::UNKNOWN), mThumbWidth(0), mThumbHeight(0), mWidth(0), mHeight(0)
{

}

UBEmbedType UBEmbedContent::type() const
{
    return mType;
}

QString UBEmbedContent::typeString() const
{
    switch (mType) {
    case UBEmbedType::PHOTO:  return "PHOTO";
    case UBEmbedType::VIDEO:  return "VIDEO";
    case UBEmbedType::LINK:   return "LINK";
    case UBEmbedType::RICH:   return "RICH";
    case UBEmbedType::IFRAME: return "IFRAME";
    default: return "UNKNOWN";
    }
}

QString UBEmbedContent::title() const
{
    return mTitle;
}

QString UBEmbedContent::authorName() const
{
    return mAuthorName;
}

QUrl UBEmbedContent::authorUrl() const
{
    return mAuthorUrl;
}

QString UBEmbedContent::providerName() const
{
    return mProviderName;
}

QUrl UBEmbedContent::providerUrl() const
{
    return mProviderUrl;
}

QUrl UBEmbedContent::thumbUrl() const
{
    return mThumbUrl;
}

int UBEmbedContent::thumbWidth() const
{
    return mThumbWidth;
}

int UBEmbedContent::thumbHeight() const
{
    return mThumbHeight;
}

int UBEmbedContent::width() const
{
    return mWidth;
}

int UBEmbedContent::height() const
{
    return mHeight;
}

QString UBEmbedContent::html() const
{
    return mHtml;
}

QUrl UBEmbedContent::url() const
{
    return mUrl;
}

UBEmbedType UBEmbedContent::typeFromString(QString &type)
{
    if (type == "photo")
    {
        return UBEmbedType::PHOTO;
    }
    else if (type == "video")
    {
        return UBEmbedType::VIDEO;
    }
    else if (type == "link")
    {
        return UBEmbedType::LINK;
    }
    else if (type == "rich")
    {
        return UBEmbedType::RICH;
    }
    else if (type == "iframe")
    {
        return UBEmbedType::IFRAME;
    }

    return UBEmbedType::UNKNOWN;
}
