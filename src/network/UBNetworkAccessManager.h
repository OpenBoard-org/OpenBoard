
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
