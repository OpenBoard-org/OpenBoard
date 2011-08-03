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
