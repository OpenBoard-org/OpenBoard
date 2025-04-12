/*
 * Copyright (C) 2015-2025API of  Département de l'Instruction Publique (DIP-SEM)
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


static const QVersionNumber TOC_VERSION{1, 0};

class UBToc
{
public:
    UBToc(QString documentPath);

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
    int findUuid(const QUuid& sceneUuid);

    int pageId(int index) const;
    void setPageId(int index, int pageId);

    QStringList assets(int index) const;
    void setAssets(int index, const QStringList& assets);

    // load and save
    bool load();
    void save() const;

    // next available pageId
    int nextAvailablePageId();

private:
    void assureSize(int index);

private:
    QString mDocumentPath;
    QVersionNumber mVersion{TOC_VERSION};
    QVector<QVariantMap> mToc;
    int mNextAvailablePageId{0};
};
