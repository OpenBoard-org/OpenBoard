/*
 * UBDocumentProxy.cpp
 *
 *  Created on: Oct 23, 2008
 *      Author: luc
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

    setDefaultDocumentSize(UBSettings::settings()->defaultDocumentSize);
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
        return UBSettings::settings()->defaultDocumentSize;
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

bool UBDocumentProxy::isModified() const
{
    return mIsModified;
}





