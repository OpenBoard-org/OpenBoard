/*
 * UBDocumentProxy.h
 *
 *  Created on: Oct 23, 2008
 *      Author: luc
 */

#ifndef UBDOCUMENTPROXY_H_
#define UBDOCUMENTPROXY_H_

#include <QtGui>

#include "frameworks/UBStringUtils.h"

#include "core/UBSettings.h"

class UBGraphicsScene;

class UBDocumentProxy : public QObject
{
    Q_OBJECT;

    public:

        UBDocumentProxy();
        UBDocumentProxy(const QString& pPersistencePath);

        virtual ~UBDocumentProxy();

        QString persistencePath() const;

        void setPersistencePath(const QString& pPersistencePath);

        void setMetaData(const QString& pKey , const QVariant& pValue);
        QVariant metaData(const QString& pKey) const;
        QHash<QString, QVariant> metaDatas() const;

        QString name() const;
        QString groupName() const;

        QSize defaultDocumentSize() const;
        void setDefaultDocumentSize(QSize pSize);
        void setDefaultDocumentSize(int pWidth, int pHeight);

        QUuid uuid() const;
        void setUuid(const QUuid& uuid);

        bool isModified() const;

        int pageCount();
        void setPageCount(int pPageCount);
        int incPageCount();
        int decPageCount();

    signals:
        void defaultDocumentSizeChanged();

    private:

        void init();

        QString mPersistencePath;

        QHash<QString, QVariant> mMetaDatas;

        bool mIsModified;

        int mPageCount;

};

inline bool operator==(const UBDocumentProxy &proxy1, const UBDocumentProxy &proxy2)
{
    return proxy1.persistencePath() == proxy2.persistencePath();
}

inline uint qHash(const UBDocumentProxy &key)
{
    return qHash(key.persistencePath());
}


#endif /* UBDOCUMENTPROXY_H_ */
