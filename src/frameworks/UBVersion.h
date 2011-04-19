
/*
 * UBVersion.h
 *
 *  Created on: May 12, 2009
 *      Author: Jerome Marchaud
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
