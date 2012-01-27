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
        QString sessionTitle() const;
        void setSessionTitle(const QString& sessionTitle);
        QString sessionTarget() const;
        void setSessionTarget(const QString& sessionTarget);
        QString sessionLicence() const;
        void setSessionLicence(const QString& sessionLicence);
        void setSessionKeywords(const QString& kw);
        QString sessionKeywords();
        void setSessionLevel(const QString& level);
        QString sessionLevel();
        void setSessionTopic(const QString& topic);
        QString sessionTopic();
        void setSessionAuthor(const QString& authors);
        QString sessionAuthors();


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
