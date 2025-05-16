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

#include <QFileInfo>
#include <QHash>
#include <QTemporaryDir>

#include <memory>

// forward
class UBToc;

/**
 * @brief The UBPageMapper maps a file name which should be saved in a ZIP
 * file for export to an input file and an output file name.
 *
 * This function is used to sequentially renumber pages on export. Page file
 * names are created according to the page sequence in the TOC. Additionally
 * an updated TOC is created which reflects the renamed page files.
 */
class UBPageMapper
{
public:
    struct MapResult
    {
        QFileInfo input; ///< file for reading
        QString output;  ///< file name for the output file in the ZIP archive
    };

public:
    UBPageMapper(const QString& documentPath, UBToc* toc);

    MapResult map(const QString& filename) const;

private:
    const QString mDocumentPath;
    const UBToc* mSourceToc{nullptr};
    std::unique_ptr<UBToc> mMappedToc;
    QHash<QString, QString> mFileMap;
    QTemporaryDir mTempDir;
};
