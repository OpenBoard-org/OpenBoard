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
