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

#ifndef UBEMBEDCONTENT_H
#define UBEMBEDCONTENT_H

#include <QString>
#include <QUrl>

enum class UBEmbedType {
    UNKNOWN,
    PHOTO,
    VIDEO,
    LINK,
    RICH,
    IFRAME
};

class UBEmbedContent
{
public:
    UBEmbedContent();

    UBEmbedType type() const;
    QString typeString() const;
    QString title() const;
    QString authorName() const;
    QUrl authorUrl() const;
    QString providerName() const;
    QUrl providerUrl() const;
    QUrl thumbUrl() const;
    int thumbWidth() const;
    int thumbHeight() const;
    int width() const;
    int height() const;
    QString html() const;
    QUrl url() const;

    static UBEmbedType typeFromString(QString& type);

private:
    friend class UBEmbedParser;
    UBEmbedType mType;
    QString mTitle;
    QString mAuthorName;
    QUrl mAuthorUrl;
    QString mProviderName;
    QUrl mProviderUrl;
    QUrl mThumbUrl;
    int mThumbWidth;
    int mThumbHeight;
    int mWidth;
    int mHeight;
    QString mHtml;
    QUrl mUrl;
};

#endif // UBEMBEDCONTENT_H
