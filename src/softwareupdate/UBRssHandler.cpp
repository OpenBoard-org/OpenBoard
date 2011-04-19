
/*
 * UBRssHandler.cpp
 *
 *  Created on: May 11, 2009
 *      Author: Jérôme Marchaud
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
