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


#include "UBTocSerializer.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static const QString VERSION{"version"};
static const QString PAGES("pages");

UBTocSerializer::UBTocSerializer(QString path)
    : mPath{path}
{
}

UBTocJsonSerializer::UBTocJsonSerializer(QString filename)
    : UBTocSerializer{filename}
{
}

bool UBTocJsonSerializer::load(QVersionNumber& version, QVector<QVariantMap>& toc)
{
    if (!toc.isEmpty())
    {
        qWarning() << "Refused to load non-empty TOC";
        return false;
    }

    QFile file{mPath + "/toc.json"};

    if (!file.open(QFile::ReadOnly))
    {
        qDebug() << "Cannot load TOC: Cannot open file" << file.fileName();
        return false;
    }

    const auto json = file.readAll();
    file.close();
    QJsonParseError error;
    const auto jsonDoc = QJsonDocument::fromJson(json, &error);

    if (error.error != QJsonParseError::NoError)
    {
        qWarning() << "Parse error reading TOC:" << error.errorString() << "at" << error.offset;
        return false;
    }

    if (!jsonDoc.isObject())
    {
        qWarning() << "Error reading TOC: JSON document is not an object";
        return false;
    }

    version = QVersionNumber::fromString(jsonDoc.object().value(VERSION).toString());

    const auto pages = jsonDoc.object().value(PAGES);

    if (!pages.isArray())
    {
        qWarning() << "Error reading TOC: pages is not an array";
        return false;
    }

    int index = 0;

    toc.resize(pages.toArray().size());

    for (const auto entry : pages.toArray())
    {
        if (!entry.isObject())
        {
            qWarning() << "Error reading TOC: JSON entry is not an object";
            return false;
        }

        toc[index++] = entry.toObject().toVariantMap();
    }

    return true;
}

bool UBTocJsonSerializer::save(const QVersionNumber& version, const QVector<QVariantMap>& toc)
{
    QJsonArray list;

    for (const auto entry : toc)
    {
        list.append(QJsonObject::fromVariantMap(entry));
    }

    QJsonObject document;
    document[VERSION] = version.toString();
    document[PAGES] = list;

    QJsonDocument doc{document};
    auto json = doc.toJson(QJsonDocument::Indented);

    // reduce indentation to one space
    json.replace("    ", " ");

    QFile file{mPath + "/toc.json"};

    if (!file.open(QFile::WriteOnly))
    {
        qWarning() << "Cannot save TOC: Cannot open file" << file.fileName();
        return false;
    }

    file.write(json);
    file.close();
    return true;
}
