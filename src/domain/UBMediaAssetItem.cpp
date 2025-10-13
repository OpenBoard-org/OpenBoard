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

#include "UBMediaAssetItem.h"

#include <QDomDocument>
#include <QRegularExpression>

QUuid UBMediaAssetItem::mediaAssetUuid() const
{
    return uuidFromPath(mediaAssets().value(0, ""));
}

QUuid UBMediaAssetItem::createMediaAssetUuid(const QByteArray& data)
{
    static QUuid namespaceUuid{QUuid::createUuidV5(QUuid(), QString("OpenBoard"))};

    return QUuid::createUuidV5(namespaceUuid, data);
}

QUuid UBMediaAssetItem::createMediaAssetUuid(const QString& path)
{
    QFileInfo fileInfo{path};

    if (!fileInfo.exists())
    {
        return {};
    }

    if (fileInfo.isFile() && fileInfo.isReadable())
    {
        QFile file{path};

        if (file.open(QFile::ReadOnly))
        {
            return createMediaAssetUuid(file.readAll());
        }

        return {};
    }

    if (fileInfo.isDir())
    {
        // check for a config.xml and read the id
        QFile configFile(path + "/config.xml");

        if (configFile.open(QFile::ReadOnly))
        {
            QDomDocument doc;
            doc.setContent(configFile.readAll());
            QDomNodeList widgetDomList = doc.elementsByTagName("widget");

            if (widgetDomList.count() > 0)
            {
                QDomElement widgetElement = widgetDomList.item(0).toElement();
                const auto id = widgetElement.attribute("id", "");

                if (id.startsWith("urn:uuid:"))
                {
                    const auto uuid = QUuid{id.mid(9)};

                    if (uuid.version() == QUuid::Sha1)
                    {
                        return uuid;
                    }
                }
                else
                {
                    return createMediaAssetUuid(id.toUtf8());
                }
            }
        }
    }

    return {};
}

QUuid UBMediaAssetItem::uuidFromPath(const QString& path) const
{
    static QRegularExpression uuidPattern("[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}");

    // scan asset file name for UUID
    QRegularExpressionMatch match = uuidPattern.match(path);

    if (match.hasMatch())
    {
        return QUuid{match.captured()};
    }

    return {};
}
