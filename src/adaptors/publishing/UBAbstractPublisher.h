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
