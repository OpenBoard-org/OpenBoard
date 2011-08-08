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

#include "frameworks/UBVersion.h"

#include "UBRssHandler.h"
#include "UBSoftwareUpdate.h"

const QString UBRssHandler::sRssItemElementName = "item";
const QString UBRssHandler::sRssLinkElementName = "link";
const QString UBRssHandler::sUniboardVersionElementName = "ub:version";  // TODO UB 4.x map properly ub namespace

UBRssHandler::UBRssHandler()
    : mRssTagParsed(false)
    , mInItem(false)
    , mInVersion(false)
{
    // NOOP
}

UBRssHandler::~UBRssHandler(void)
{
    while (!mSoftwareUpdates.isEmpty())
    {
        delete mSoftwareUpdates.takeFirst();
    }
}

bool UBRssHandler::startElement(
    const QString & /* namespaceURI */,
    const QString & /* localName */,
    const QString &qualifiedName,
    const QXmlAttributes &attributes)
{
    bool ok = true;
    if (!mRssTagParsed && qualifiedName != "rss")
    {
        mError = "This file is not a RSS source.";
        ok = false;
    }
    else if (qualifiedName == "rss")
    {
        QString version = attributes.value("version");
        if (!version.isEmpty() && version != "2.0")
        {
            mError = "Can only handle RSS 2.0.";
            ok = false;
        }
        else
        {
            mRssTagParsed = true;
        }
    }
    else if (qualifiedName == sRssItemElementName)
    {
        mInItem = true;
    }
    else if (qualifiedName == sUniboardVersionElementName)
    {
        mInVersion = true;
    }
    mCurrentText = "";
    return ok;
}

bool UBRssHandler::characters(const QString &str)
{
    mCurrentText += str;
    return true;
}

bool UBRssHandler::endElement(
    const QString & /* namespaceURI */,
    const QString & /* localName */,
    const QString &qualifiedName)
{
    bool ok = true;
    if (qualifiedName == sRssItemElementName)
    {
        mInItem = false;
        if (mVersion.isValid() && !mDownloadUrl.isEmpty())
        {
            UBSoftwareUpdate *softwareUpdate = new UBSoftwareUpdate(mVersion, mDownloadUrl);
            mSoftwareUpdates.append(softwareUpdate);
        }
        mVersion = UBVersion();
        mDownloadUrl = "";
    }
    else if (qualifiedName == sRssLinkElementName)
    {
        if (mInItem)
        {
            QUrl url(mCurrentText);
            if (url.isValid())
                mDownloadUrl = mCurrentText;
            else
                ok = false;
        }
    }
    else if (qualifiedName == sUniboardVersionElementName)
    {
        if (mInItem)
        {
            mVersion.setString(mCurrentText);
            ok = mVersion.isValid();
            mInVersion = false;
        }
    }
    return ok;
}

bool UBRssHandler::fatalError(const QXmlParseException &exception)
{
    qWarning() << "Fatal error at line " << exception.lineNumber()
               << ", column " << exception.columnNumber() << ": "
               << exception.message() << mError;
    return false;
}

QString UBRssHandler::error() const
{
    return mError;
}

QList<UBSoftwareUpdate *> UBRssHandler::softwareUpdates() const
{
    return mSoftwareUpdates;
}
