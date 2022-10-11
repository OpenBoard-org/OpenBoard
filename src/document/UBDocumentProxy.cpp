/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBDocumentProxy.h"

#include "frameworks/UBStringUtils.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "core/UBDocumentManager.h"
#include "core/memcheck.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"

UBDocumentProxy::UBDocumentProxy()
    : mPageCount(0)
    , mPageDpi(0)
    , mPersistencePath("")
{
    init();
}

UBDocumentProxy::UBDocumentProxy(const UBDocumentProxy &rValue) :
    QObject()
{
    mPersistencePath = rValue.mPersistencePath;
    mMetaDatas = rValue.mMetaDatas;
    mIsModified = rValue.mIsModified;
    mPageCount = rValue.mPageCount;
}


UBDocumentProxy::UBDocumentProxy(const QString& pPersistancePath)
    : mPageCount(0)
    , mPageDpi(0)
{
    init();
    setPersistencePath(pPersistancePath);

    mMetaDatas = UBMetadataDcSubsetAdaptor::load(pPersistancePath);
}


void UBDocumentProxy::init()
{
    setMetaData(UBSettings::documentGroupName, "");

    QDateTime now = QDateTime::currentDateTime();
    setMetaData(UBSettings::documentName, QLocale::system().toString(now, QLocale::ShortFormat));

    setUuid(QUuid::createUuid());

    setDefaultDocumentSize(UBSettings::settings()->pageSize->get().toSize());
}

bool UBDocumentProxy::theSameDocument(UBDocumentProxy *proxy)
{
    return  proxy && mPersistencePath == proxy->mPersistencePath;
}

UBDocumentProxy::~UBDocumentProxy()
{
    // NOOP
}

UBDocumentProxy* UBDocumentProxy::deepCopy() const
{
    UBDocumentProxy* copy = new UBDocumentProxy();

    copy->mPersistencePath = QString(mPersistencePath);
    copy->mMetaDatas = QMap<QString, QVariant>(mMetaDatas);
    copy->mIsModified = mIsModified;
    copy->mPageCount = mPageCount;

    return copy;
}


int UBDocumentProxy::pageCount()
{
    return mPageCount;
}


void UBDocumentProxy::setPageCount(int pPageCount)
{
    mPageCount = pPageCount;
}

int UBDocumentProxy::pageDpi()
{
    return mPageDpi;
}

void UBDocumentProxy::setPageDpi(int dpi)
{
    mPageDpi = dpi;
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

QMap<QString, QVariant> UBDocumentProxy::metaDatas() const
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
    return QDateTime::currentDateTime();
}

bool UBDocumentProxy::isModified() const
{
    return mIsModified;
}





