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

#ifndef UBNETWORKACCESSMANAGER_H
#define UBNETWORKACCESSMANAGER_H

#include <QtNetwork>

class UBNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT;

    public:

        static UBNetworkAccessManager *defaultAccessManager();

        UBNetworkAccessManager(QObject *parent = 0);

		virtual QNetworkReply *get(const QNetworkRequest &request);

    protected:
        virtual QNetworkReply * createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0 );

    private:

        QList<QString> sslTrustedHostList;

        static UBNetworkAccessManager *sNetworkAccessManager;

        volatile int mProxyAuthenticationCount;

    private slots:
        void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
        void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth);
        void sslErrors(QNetworkReply *reply, const QList<QSslError> &error);
};



#endif // UBNETWORKACCESSMANAGER_H
