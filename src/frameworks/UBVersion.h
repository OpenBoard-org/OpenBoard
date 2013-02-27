/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBVERSION_H_
#define UBVERSION_H_

#include <QString>

enum ReleaseStage { Alpha = 10, Beta = 11, ReleaseCandidate = 15 };

class UBVersion
{
    public:
        UBVersion();
        UBVersion(const QString &string);
        virtual ~UBVersion();

        void setString(const QString &string);
        QString toString() const;

        bool              isValid() const;
        int        platformNumber() const;
        int           majorNumber() const;
        ReleaseStage releaseStage() const;
        int           minorNumber() const;

        bool operator < (const UBVersion &otherVersion) const;
        bool operator == (const UBVersion &otherVersion) const;
        bool operator > (const UBVersion &otherVersion) const;

    private:

        QString mString;
        bool mIsValid;
        int mPlatform;
        int mMajor;
        int mMinor;
        ReleaseStage mReleaseStage;
};

#endif // UBVERSION_H_
