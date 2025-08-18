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


#pragma once

#include <QString>
#include <QVariant>
#include <QVector>
#include <QVersionNumber>


static const QVersionNumber DOCUMENT_TOC_VERSION{1, 0};

class UBDocumentToc
{
public:
    UBDocumentToc(const QString& documentPath);
    UBDocumentToc(const UBDocumentToc& other, const QString& documentPath);

    QVersionNumber version() const;

    // size
    int pageCount() const;

    // insert, move and remove
    int insert(int index);
    void move(int fromIndex, int toIndex);
    void remove(int index);

    // access values
    QUuid uuid(int index) const;
    void setUuid(int index, const QUuid& uuid);
    int findUuid(const QUuid& sceneUuid) const;

    int pageId(int index) const;
    void setPageId(int index, int pageId);

    QStringList assets(int index) const;
    void setAssets(int index, const QStringList& assets);
    void unsetAssets(int index);
    bool hasAssetsEntry(int index) const;

    // load and save
    bool load();
    void save();

    // next available pageId
    int nextAvailablePageId();

private:
    void assureSize(int index);

private:
    const QString mDocumentPath;
    QVersionNumber mVersion{DOCUMENT_TOC_VERSION};
    QVector<QVariantMap> mToc;
    int mNextAvailablePageId{0};
    bool mModified{false};
};
