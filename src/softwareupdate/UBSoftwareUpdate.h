
/*
 * UBSoftwareUpdate.h
 *
 *  Created on: May 29, 2009
 *      Author: Jerome Marchaud
 */

#ifndef UBSOFTWAREUPDATE_H_
#define UBSOFTWAREUPDATE_H_

#include <QtCore>

#include "frameworks/UBVersion.h"

class UBSoftwareUpdate
{
    public:
        UBSoftwareUpdate();
        UBSoftwareUpdate(UBVersion& version, const QString &downloadUrl);
        virtual ~UBSoftwareUpdate();

        UBVersion     version() const;
        QString   downloadUrl() const;

        bool operator==(const UBSoftwareUpdate &other) const;

    private:
        UBVersion mVersion;
        QString mDownloadUrl;
};

#endif // UBSOFTWAREUPDATE_H_
