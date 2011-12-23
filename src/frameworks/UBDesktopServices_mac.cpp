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

#include "UBDesktopServices.h"

#include <QtCore>

#import <Carbon/Carbon.h>

/*
    Translates a QDesktopServices::StandardLocation into the mac equivalent.
*/
OSType translateLocation(QDesktopServices::StandardLocation type)
{
    switch (type) {
    case QDesktopServices::DesktopLocation:
        return kDesktopFolderType; break;

    case QDesktopServices::DocumentsLocation:
        return kDocumentsFolderType; break;

    case QDesktopServices::FontsLocation:
        // There are at least two different font directories on the mac: /Library/Fonts and ~/Library/Fonts.
        // To select a specific one we have to specify a different first parameter when calling FSFindFolder.
        return kFontsFolderType; break;

    case QDesktopServices::ApplicationsLocation:
        return kApplicationsFolderType; break;

    case QDesktopServices::MusicLocation:
        return kMusicDocumentsFolderType; break;

    case QDesktopServices::MoviesLocation:
        return kMovieDocumentsFolderType; break;

    case QDesktopServices::PicturesLocation:
        return kPictureDocumentsFolderType; break;

    case QDesktopServices::TempLocation:
        return kTemporaryFolderType; break;

    case QDesktopServices::DataLocation:
        return kApplicationSupportFolderType; break;

    case QDesktopServices::CacheLocation:
        return kCachedDataFolderType; break;

    default:
        return kDesktopFolderType; break;
    }
}

/*
    Constructs a full unicode path from a FSRef.
*/
static QString getFullPath(const FSRef &ref)
{
    QByteArray ba(2048, 0);
    if (FSRefMakePath(&ref, reinterpret_cast<UInt8 *>(ba.data()), ba.size()) == noErr)
        return QString::fromUtf8(ba).normalized(QString::NormalizationForm_C);
    return QString();
}

// Mac OS X implementation of QDesktopServices is bugged in Qt 4.4,
// we use the implementation inspired from Qt 4.5 snapshot
QString UBDesktopServices::storageLocation(StandardLocation type)
{
    if (QDesktopServices::HomeLocation == type)
        return QDir::homePath();
    else if (QDesktopServices::TempLocation == type)
        return QDir::tempPath();

    short domain = kOnAppropriateDisk;

    if (QDesktopServices::DataLocation == type || QDesktopServices::CacheLocation == type)
        domain = kUserDomain;

     // http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html
     FSRef ref;
     OSErr err = FSFindFolder(domain, translateLocation(type), false, &ref);
     if (err)
        return QString();

    QString path = getFullPath(ref);

    if (QDesktopServices::DataLocation == type || QDesktopServices::CacheLocation == type)
        path += "/Sankore/Open-Sankore";

    return path;
}

