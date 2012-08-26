/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include "UBWidgetMessageAPI.h"

#include "core/UBApplication.h"

#include "domain/UBGraphicsWebView.h"

#include "core/memcheck.h"

UBWidgetMessageAPI::UBWidgetMessageAPI(UBGraphicsWidgetItem *graphicsWidgetItem, QObject *parent)
    : QObject(parent)
    , mGraphicsWidgetItem(graphicsWidgetItem)
{
    connect(UBWidgetAPIMessageBroker::instance(), SIGNAL(newMessage(const QString&, const QString&)), this, SLOT(onNewMessage(const QString&, const QString&)), Qt::QueuedConnection);
}

UBWidgetMessageAPI::~UBWidgetMessageAPI()
{
    // NOOP
}


void UBWidgetMessageAPI::sendMessage(const QString& pTopicName, const QString& pMessage)
{
    UBWidgetAPIMessageBroker::instance()->sendMessage(pTopicName, pMessage);
}


void UBWidgetMessageAPI::onNewMessage(const QString& pTopicName, const QString& pMessage)
{
    if (mSubscribedTopics.contains(pTopicName))
    {
        if (mGraphicsWidgetItem && mGraphicsWidgetItem->page() && mGraphicsWidgetItem->page()->mainFrame())
        {

            QString js;
            js += "if(widget && widget.messages && widget.messages.onmessage)";
            js += "{widget.messages.onmessage('";
            js += pMessage + "', '" + pTopicName + "')}";

            mGraphicsWidgetItem->page()->
                mainFrame()->evaluateJavaScript(js);

        }
    }
}



UBWidgetAPIMessageBroker* UBWidgetAPIMessageBroker::sInstance = 0;


UBWidgetAPIMessageBroker::UBWidgetAPIMessageBroker(QObject *parent)
    : QObject(parent)
{
    // NOOP
}


UBWidgetAPIMessageBroker::~UBWidgetAPIMessageBroker()
{
    // NOOP
}


UBWidgetAPIMessageBroker* UBWidgetAPIMessageBroker::instance()
{
    if (!sInstance)
        sInstance = new UBWidgetAPIMessageBroker(UBApplication::staticMemoryCleaner);

    return sInstance;

}


void UBWidgetAPIMessageBroker::sendMessage(const QString& pTopicName, const QString& pMessage)
{
    emit newMessage(pTopicName, pMessage);
}
