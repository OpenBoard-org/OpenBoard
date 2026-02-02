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
#include <QVariantMap>
#include <QVector>
#include <QVersionNumber>


class UBTocSerializer
{
public:
    UBTocSerializer(QString path);
    virtual ~UBTocSerializer() = default;

    virtual bool load(QVersionNumber& version, QVector<QVariantMap>& toc) = 0;
    virtual bool save(const QVersionNumber& version, const QVector<QVariantMap>& toc) = 0;

protected:
    QString mPath;
};


class UBTocJsonSerializer : public UBTocSerializer
{
public:
    UBTocJsonSerializer(QString filename);
    virtual ~UBTocJsonSerializer() = default;

    virtual bool load(QVersionNumber& version, QVector<QVariantMap>& toc) override;
    virtual bool save(const QVersionNumber& version, const QVector<QVariantMap>& toc) override;
};
