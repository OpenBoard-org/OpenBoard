#ifndef UBABSTRACTPUBLISHER_H
#define UBABSTRACTPUBLISHER_H

#include <QtCore>

class UBServerXMLHttpRequest;


class UBAbstractPublisher : public QObject
{
    Q_OBJECT
    public:
        explicit UBAbstractPublisher(QObject *parent = 0);
        virtual ~UBAbstractPublisher(){}

        void authenticate();

    signals:

        void authenticated(const QUuid&, const QString&);
        void authenticationFailure();

    private:

        void sendAuthenticationTokenRequest();

        QByteArray encrypt(const QString& token);

        UBServerXMLHttpRequest* mTokenRequest;

    private slots:

        void processAuthenticationTokenResponse(bool, const QByteArray&);

};

#endif // UBABSTRACTPUBLISHER_H
