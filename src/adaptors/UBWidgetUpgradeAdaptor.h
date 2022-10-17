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

#ifndef UBWIDGETUPGRADEADAPTOR_H
#define UBWIDGETUPGRADEADAPTOR_H

#include <QDir>
#include <QMap>
#include <QVector>

class UBDocumentProxy;

//forward
class UBDocumentProxy;

class UBWidgetUpgradeAdaptor
{
private:
    // content of widget's config.xml as far as relevant for upgrade
    class Widget
    {
    public:
        Widget() = default;
        explicit Widget(const QString& dir);
        bool operator==(const Widget& other) const;
        bool valid() const;
        QString path() const;
        QString id() const;
        QString version() const;
        bool hasUniqueId() const;

    private:
        QString m_path;
        QString m_id;
        QString m_version;
        bool m_hasUniqueId;
    };

    enum class ApiUsage
    {
        NO,
        COMPATIBLE,
        INCOMPATIBLE
    };

public:
    UBWidgetUpgradeAdaptor();
    void upgradeWidgets(UBDocumentProxy *proxy);

private:
    ApiUsage scanDir(const QDir &widget) const;
    ApiUsage scanFile(const QFileInfo& info) const;
    void copyDir(QDir target, QDir source);
    void fillLibraryWidgets();

private:
    QMap<QString, Widget> libraryWidgets;
};

#endif // UBWIDGETUPGRADEADAPTOR_H
