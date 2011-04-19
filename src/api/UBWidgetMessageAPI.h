/*
 * UBWidgetMessageAPI.h
 *
 *  Created on: 28 aug 2009
 *      Author: Luc
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
