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

#ifndef UBWIDGETMESSAGEAPI_H_
#define UBWIDGETMESSAGEAPI_H_

#include <QtCore>

#include "domain/UBGraphicsWidgetItem.h"

class UBWidgetMessageAPI : public QObject
{
    Q_OBJECT;

    public:
        UBWidgetMessageAPI(UBW3CWidget *widget);
        virtual ~UBWidgetMessageAPI();

    public slots:

        void sendMessage(const QString& pTopicName, const QString& pMessage);

        void subscribeToTopic(const QString& pTopicName)
        {
            mSubscribedTopics << pTopicName;
        }

        void unsubscribeFromTopic(const QString& pTopicName)
        {
            mSubscribedTopics.remove(pTopicName);
        }

    private slots:

         void onNewMessage(const QString& pTopicName, const QString& pMessage);

    private:

        QSet<QString> mSubscribedTopics;
        UBW3CWidget *mWebWidget;
};


class UBWidgetAPIMessageBroker : public QObject
{
    Q_OBJECT;

    private:

        UBWidgetAPIMessageBroker(QObject *parent = 0);
        ~UBWidgetAPIMessageBroker();

    public:
        static UBWidgetAPIMessageBroker* instance();

    public slots:

        void sendMessage(const QString& pTopicName, const QString& pMessage);

    signals:

        void newMessage(const QString& pTopicName, const QString& pMessage);

    private:

      static UBWidgetAPIMessageBroker* sInstance;

};

#endif /* UBWIDGETMESSAGEAPI_H_ */
