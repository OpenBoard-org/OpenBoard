
/*
 * UBRssHandler.h
 *
 *  Created on: May 11, 2009
 *      Author: Jerome Marchaud
 */

#ifndef UBRSSHANDLER_H_
#define UBRSSHANDLER_H_

#include <QtXml>
#include <QList>

#include "frameworks/UBVersion.h"
class UBSoftwareUpdate;

class UBRssHandler: public QXmlDefaultHandler
{
    public:
        UBRssHandler();
        virtual ~UBRssHandler(void);

        // QXmlDefaultHandler
        virtual bool   startElement(const QString &namespaceURI,
                                    const QString &localName, const QString &qualifiedName,
                                    const QXmlAttributes &attributes);
        virtual bool     endElement(const QString &namespaceURI,
                                    const QString &localName, const QString &qualifiedName);
        virtual bool     characters(const QString &str);
        virtual bool     fatalError(const QXmlParseException &exception);

        // UBRssHandler
        QString                             error() const;
        QList<UBSoftwareUpdate *> softwareUpdates() const;

    private:
        QList<UBSoftwareUpdate *> mSoftwareUpdates;
        bool mRssTagParsed;
        bool mInItem;
        bool mInVersion;
        QString mError;
        QString mCurrentText;
        UBVersion mVersion;
        QString mDownloadUrl;

        static const QString sRssItemElementName;
        static const QString sRssLinkElementName;
        static const QString sUniboardVersionElementName;
};

#endif /* UBRSSHANDLER_H_ */
