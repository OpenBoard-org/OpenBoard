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




#include "UBVersion.h"

#include <QtCore>
#include <QStringList>


#include "core/memcheck.h"

UBVersion::UBVersion(const QString &string)
{
    mString = string;
}

uint UBVersion::toUInt() const
{
    /* Based on semantic versioning, version numbers look like:
     * Major.Minor.Patch-Type.Build
     *
     * To compare version numbers, the string is split into each part, and they are multiplied
     * to give a number where the first two digits are the Major version, the next two are the
     * Minor version, and so on.
     *
     * i.e if Major, Minor etc. are named A, B, C, D, E, the number will look like:
     * AABBCCDDEE
    */

    uint result = 0;
    QStringList list = mString.split(QRegularExpression("[-\\.]"));
    switch (list.count()) {
    case 2:
        //short version  1.0
        result = (list.at(0).toUInt() * 100000000) + (list.at(1).toUInt() * 1000000) + (Release * 100);
        break;
    case 3:
        //release version 1.0.0
        result = (list.at(0).toUInt() * 100000000) + (list.at(1).toUInt() * 1000000) + list.at(2).toUInt()*10000 + (Release * 100);
        break;
    case 5:{
        //standard version  1.0.0.a/b/rc.0
        uint releaseStage = list.at(3).startsWith("a") ? Alpha :(list.at(3).startsWith("b") ? Beta : ReleaseCandidate);
        result = (list.at(0).toUInt() * 100000000) + (list.at(1).toUInt() * 1000000) + (list.at(2).toUInt() * 10000) + (releaseStage * 100) + list.at(4).toUInt();
        break;
    }
    default:
        qWarning() << "Unknown version format.";
        break;
    }
    return result;
}

UBVersion::~UBVersion()
{
    // NOOP
}

bool UBVersion::operator < (const UBVersion &otherVersion) const
{
    return toUInt() < otherVersion.toUInt();
}

bool UBVersion::operator == (const UBVersion &otherVersion) const
{
    return toUInt() == otherVersion.toUInt();
}

bool UBVersion::operator > (const UBVersion &otherVersion) const
{
    return toUInt() > otherVersion.toUInt();
}
