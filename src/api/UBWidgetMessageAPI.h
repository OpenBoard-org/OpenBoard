/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBWIDGETMESSAGEAPI_H_
#define UBWIDGETMESSAGEAPI_H_

#include <QtCore>

#include "domain/UBGraphicsWidgetItem.h"

class UBWidgetMessageAPI : public QObject
{
    Q_OBJECT;

    public:
        UBWidgetMessageAPI(UBGraphicsWidgetItem *graphicsWidgetItem, QObject *parent = 0);
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
        UBGraphicsWidgetItem *mGraphicsWidgetItem;
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
