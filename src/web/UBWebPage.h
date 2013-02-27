/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBWEBPAGE_H_
#define UBWEBPAGE_H_

/*
 * Use UBWebPage instead of QWebPage to log JavaScript console messages with qDebug
 */

#include <QtCore>
#include <QWebPage>

class UBWebPage : public QWebPage
{
    Q_OBJECT;

    public:
        UBWebPage(QObject *parent = 0);
        virtual ~UBWebPage();

        virtual void javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID);

        virtual QString userAgentForUrl(const QUrl& url) const;

    private:
        QString mCachedUserAgentString;
        QWebPluginFactory *mPluginFactory;

};

#endif /* UBWEBPAGE_H_ */
