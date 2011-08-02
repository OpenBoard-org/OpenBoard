/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
