/*
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

//UBVersion::UBVersion()
//    : mIsValid(false)
//    , mPlatform(-1)
//    , mMajor(-1)
//    , mMinor(-1)
//{
//    // NOOP
//}


UBVersion::UBVersion(const QString &string)
{
    mString = string;
}

uint UBVersion::toUInt() const
{
    uint result = 0;
    QStringList list = mString.split(".");
    switch (list.count()) {
    case 2:
        //short version  1.0
        result = (list.at(0).toUInt() * 1000000) + (list.at(1).toUInt() * 10000);
        break;
    case 3:
        //release version 1.0.0
        result = (list.at(0).toUInt() * 1000000) + (list.at(1).toUInt() * 10000) + list.at(2).toUInt();
        break;
    case 4:{
        //standard version  1.0.a/b/r.0
        uint releaseStage = list.at(2).startsWith("a") ? Alpha :(list.at(2).startsWith("b") ? Beta : ReleaseCandidate);
        result = (list.at(0).toUInt() * 1000000) + (list.at(1).toUInt() * 10000) + (releaseStage * 100) + list.at(3).toUInt();
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


//QString UBVersion::toString() const
//{
//    return isValid() ? mString : "INVALID";
//}

//void UBVersion::setString(const QString &string)
//{
//    mIsValid = true;
//    mString = string;
//    QStringList versionParts = string.split(".");
//    int count = versionParts.count();

//    if (count <  2)
//    {
//        mIsValid = false;
//        return;
//    }

//    mPlatform = versionParts.at(0).toInt(&mIsValid);
//    if (!isValid()) return;
//    mMajor = versionParts.at(1).toInt(&mIsValid);
//    if (!isValid()) return;

//    if (count == 2)
//    {
//        mMinor = 0;
//        mReleaseStage = ReleaseCandidate;
//    }
//    else if (count == 3)
//    {
//        // Format 4.1.2 (implicitly release)
//        mMinor = versionParts.at(2).toInt(&mIsValid);
//        if (!isValid()) return;
//        mReleaseStage = ReleaseCandidate;
//    }
//    else if (count >= 4)
//    {
//        // Format 4.1.x.2 (where x = a|b|r|<integer>)
//        if ("a" == versionParts.at(2))
//        {
//            mReleaseStage = Alpha;
//        }
//        else if ("b" == versionParts.at(2))
//        {
//            mReleaseStage = Beta;
//        }
//        else if ("r" == versionParts.at(2))
//        {
//            mReleaseStage = ReleaseCandidate;
//        }
//        else
//        {
//            mMinor = versionParts.at(2).toInt(&mIsValid);
//            if (!isValid()) return;
//            mReleaseStage = ReleaseCandidate;
//            return;
//        }

//        QStringList lastParts = versionParts.at(3).split(" ");
//        mMinor = lastParts.at(0).toInt();
//    }
//}

//bool UBVersion::isValid() const
//{
//    return mIsValid;
//}

//int UBVersion::platformNumber() const
//{
//    Q_ASSERT(isValid());
//    return mPlatform;
//}

//int UBVersion::majorNumber() const
//{
//    Q_ASSERT(isValid());
//    return mMajor;
//}

//ReleaseStage UBVersion::releaseStage() const
//{
//    Q_ASSERT(isValid());
//    return mReleaseStage;
//}

//int UBVersion::minorNumber() const
//{
//    Q_ASSERT(isValid());
//    return mMinor;
//}

//bool UBVersion::operator < (const UBVersion &otherVersion) const
//{
//    Q_ASSERT(isValid());
//    Q_ASSERT(otherVersion.isValid());

//    if (platformNumber() != otherVersion.platformNumber())
//        return platformNumber() < otherVersion.platformNumber();
//    if (majorNumber() != otherVersion.majorNumber())
//        return majorNumber() < otherVersion.majorNumber();
//    if (releaseStage() != otherVersion.releaseStage())
//        return releaseStage() < otherVersion.releaseStage();
//    if (minorNumber() != otherVersion.minorNumber())
//        return minorNumber() < otherVersion.minorNumber();
//    return false;
//}

//bool UBVersion::operator == (const UBVersion &otherVersion) const
//{
//    Q_ASSERT(isValid());
//    Q_ASSERT(otherVersion.isValid());

//    return (platformNumber() == otherVersion.platformNumber() &&
//            majorNumber() == otherVersion.majorNumber() &&
//            releaseStage() == otherVersion.releaseStage() &&
//            minorNumber() == otherVersion.minorNumber());
//}

//bool UBVersion::operator > (const UBVersion &otherVersion) const
//{
//    Q_ASSERT(isValid());
//    Q_ASSERT(otherVersion.isValid());

//    if (platformNumber() != otherVersion.platformNumber())
//        return platformNumber() > otherVersion.platformNumber();
//    if (majorNumber() != otherVersion.majorNumber())
//        return majorNumber() > otherVersion.majorNumber();
//    if (releaseStage() != otherVersion.releaseStage())
//        return releaseStage() > otherVersion.releaseStage();
//    if (minorNumber() != otherVersion.minorNumber())
//        return minorNumber() > otherVersion.minorNumber();
//    return false;
//}


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
