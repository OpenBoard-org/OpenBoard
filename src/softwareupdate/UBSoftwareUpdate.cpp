
/*
 * UBSoftwareUpdate.h
 *
 *  Created on: May 29, 2009
 *      Author: Jerome Marchaud
 */

#include "frameworks/UBVersion.h"

#include "UBSoftwareUpdate.h"

UBSoftwareUpdate::UBSoftwareUpdate()
{
    // NOOP
}

UBSoftwareUpdate::UBSoftwareUpdate(UBVersion& version, const QString &downloadUrl)
    : mVersion(version)
    , mDownloadUrl(downloadUrl)
{
    // NOOP
}

UBSoftwareUpdate::~UBSoftwareUpdate()
{
    // NOOP
}

UBVersion UBSoftwareUpdate::version() const
{
    return mVersion;
}

QString UBSoftwareUpdate::downloadUrl() const
{
    return mDownloadUrl;
}

bool UBSoftwareUpdate::operator==(const UBSoftwareUpdate &other) const
{
    return version() == other.version();
}
