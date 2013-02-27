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
