/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
