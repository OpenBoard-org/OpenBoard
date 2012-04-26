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

#include "UBDocumentProxy.h"

#include "frameworks/UBStringUtils.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "core/UBDocumentManager.h"

#include "core/memcheck.h"

UBDocumentProxy::UBDocumentProxy()
    : mPageCount(0)
{
    init();
}


UBDocumentProxy::UBDocumentProxy(const QString& pPersistancePath)
    : mPageCount(0)
{
    init();
    setPersistencePath(pPersistancePath);
}


void UBDocumentProxy::init()
{
    setMetaData(UBSettings::documentGroupName, "");

    QDateTime now = QDateTime::currentDateTime();
    setMetaData(UBSettings::documentName, now.toString(Qt::SystemLocaleShortDate));

    setUuid(QUuid::createUuid());

    setDefaultDocumentSize(UBSettings::settings()->pageSize->get().toSize());

    setSessionTitle("");
    setSessionTarget("");
    setSessionLicence("");
    setSessionKeywords("");
    setSessionLevel("");
    setSessionTopic("");
    setSessionAuthor("");
}


UBDocumentProxy::~UBDocumentProxy()
{
    // NOOP
}


int UBDocumentProxy::pageCount()
{
    return mPageCount;
}


void UBDocumentProxy::setPageCount(int pPageCount)
{
    mPageCount = pPageCount;
}


int UBDocumentProxy::incPageCount()
{
    if (mPageCount <= 0)
    {
        mPageCount = 1;
    }
    else
    {
        mPageCount++;
    }

    return mPageCount;

}


int UBDocumentProxy::decPageCount()
{
    mPageCount --;

    if (mPageCount < 0)
    {
        mPageCount = 0;
    }

    return mPageCount;
}

QString UBDocumentProxy::persistencePath() const
{
    return mPersistencePath;
}

void UBDocumentProxy::setPersistencePath(const QString& pPersistencePath)
{
    if (pPersistencePath != mPersistencePath)
    {
        mIsModified = true;
        mPersistencePath = pPersistencePath;
    }
}

void UBDocumentProxy::setMetaData(const QString& pKey, const QVariant& pValue)
{
    if (mMetaDatas.contains(pKey) && mMetaDatas.value(pKey) == pValue)
        return;
    else
    {
        mIsModified = true;
        mMetaDatas.insert(pKey, pValue);
        if (pKey == UBSettings::documentUpdatedAt)
        {
            UBDocumentManager *documentManager = UBDocumentManager::documentManager();
            if (documentManager)
                documentManager->emitDocumentUpdated(this);
        }
    }
}

QVariant UBDocumentProxy::metaData(const QString& pKey) const
{
    if (mMetaDatas.contains(pKey))
    {
        return mMetaDatas.value(pKey);
    }
    else
    {
        qDebug() << "Unknown metadata key" << pKey;
        return QString(""); // failsafe
    }
}

QHash<QString, QVariant> UBDocumentProxy::metaDatas() const
{
    return mMetaDatas;
}

QString UBDocumentProxy::name() const
{
    return metaData(UBSettings::documentName).toString();
}

QString UBDocumentProxy::groupName() const
{
    return metaData(UBSettings::documentGroupName).toString();
}

QSize UBDocumentProxy::defaultDocumentSize() const
{
    if (mMetaDatas.contains(UBSettings::documentSize))
        return metaData(UBSettings::documentSize).toSize();
    else
        return UBSettings::settings()->pageSize->get().toSize();
}

void UBDocumentProxy::setDefaultDocumentSize(QSize pSize)
{
    if (defaultDocumentSize() != pSize)
    {
        setMetaData(UBSettings::documentSize, QVariant(pSize));
        emit defaultDocumentSizeChanged();

        mIsModified = true;
    }
}

void UBDocumentProxy::setDefaultDocumentSize(int pWidth, int pHeight)
{
    setDefaultDocumentSize(QSize(pWidth, pHeight));
}


QUuid UBDocumentProxy::uuid() const
{
    QString id = metaData(UBSettings::documentIdentifer).toString();
    QString sUuid = id.replace(UBSettings::uniboardDocumentNamespaceUri + "/", "");

    return QUuid(sUuid);
}

void UBDocumentProxy::setUuid(const QUuid& uuid)
{
    setMetaData(UBSettings::documentIdentifer,
            UBSettings::uniboardDocumentNamespaceUri + "/" + UBStringUtils::toCanonicalUuid(uuid));
}

QString UBDocumentProxy::sessionTitle() const
{
    if(mMetaDatas.contains(UBSettings::sessionTitle))
        return metaData(UBSettings::sessionTitle).toString();
    else
        return QString();
}

void UBDocumentProxy::setSessionTitle(const QString & sessionTitle)
{
    setMetaData(UBSettings::sessionTitle,QVariant(sessionTitle));
}

QString UBDocumentProxy::sessionTarget() const
{
    if(mMetaDatas.contains(UBSettings::sessionTarget))
        return metaData(UBSettings::sessionTarget).toString();
    else
        return QString();
}

void UBDocumentProxy::setSessionTarget(const QString & sessionTarget)
{
    setMetaData(UBSettings::sessionTarget,QVariant(sessionTarget));
}

QString UBDocumentProxy::sessionLicence() const
{
    if(mMetaDatas.contains(UBSettings::sessionLicence))
        return metaData(UBSettings::sessionLicence).toString();
    else
        return QString();
}

void UBDocumentProxy::setSessionLicence(const QString & sessionLicence)
{
    setMetaData(UBSettings::sessionLicence,QVariant(sessionLicence));
}

void UBDocumentProxy::setSessionKeywords(const QString &kw)
{
    setMetaData(UBSettings::sessionKeywords,QVariant(kw));
}

QString UBDocumentProxy::sessionKeywords()
{
    if(mMetaDatas.contains(UBSettings::sessionKeywords))
        return metaData(UBSettings::sessionKeywords).toString();
    else
        return QString();
}

void UBDocumentProxy::setSessionLevel(const QString &level)
{
    setMetaData(UBSettings::sessionLevel,QVariant(level));
}

QString UBDocumentProxy::sessionLevel()
{
    if(mMetaDatas.contains(UBSettings::sessionLevel))
        return metaData(UBSettings::sessionLevel).toString();
    else
        return QString();
}

void UBDocumentProxy::setSessionTopic(const QString &topic)
{
    setMetaData(UBSettings::sessionTopic,QVariant(topic));
}

QString UBDocumentProxy::sessionTopic()
{
    if(mMetaDatas.contains(UBSettings::sessionTopic))
        return metaData(UBSettings::sessionTopic).toString();
    else
        return QString();
}

void UBDocumentProxy::setSessionAuthor(const QString &authors)
{
    setMetaData(UBSettings::sessionAuthors,QVariant(authors));
}

QString UBDocumentProxy::sessionAuthors()
{
    if(mMetaDatas.contains(UBSettings::sessionAuthors))
        return metaData(UBSettings::sessionAuthors).toString();
    else
        return QString();
}

QDateTime UBDocumentProxy::documentDate()
{
    if(mMetaDatas.contains(UBSettings::documentDate))
        return UBStringUtils::fromUtcIsoDate(metaData(UBSettings::documentDate).toString());
    return QDateTime::currentDateTime();
}

QDateTime UBDocumentProxy::lastUpdate()
{
    if(mMetaDatas.contains(UBSettings::documentUpdatedAt))
        return UBStringUtils::fromUtcIsoDate(metaData(UBSettings::documentUpdatedAt).toString());
    return QDateTime().currentDateTime();
}

bool UBDocumentProxy::isModified() const
{
    return mIsModified;
}





