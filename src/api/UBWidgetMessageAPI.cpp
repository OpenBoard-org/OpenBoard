/*
 * UBWidgetMessageAPI.cpp
 *
 *  Created on: 28 aug 2009
 *      Author: Luc
 */

#include "UBWidgetMessageAPI.h"

#include "core/UBApplication.h"

#include "domain/UBAbstractWidget.h"

UBWidgetMessageAPI::UBWidgetMessageAPI(UBW3CWidget *widget)
    : QObject(widget)
    , mWebWidget(widget)
{
    connect(UBWidgetAPIMessageBroker::instance(), SIGNAL(newMessage(const QString&, const QString&))
            , this, SLOT(onNewMessage(const QString&, const QString&)), Qt::QueuedConnection);
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
        if (mWebWidget
                && mWebWidget->page()
                && mWebWidget->page()->mainFrame())
        {

            QString js;
            js += "if(widget && widget.messages && widget.messages.onmessage)";
            js += "{widget.messages.onmessage('";
            js += pMessage + "', '" + pTopicName + "')}";

            mWebWidget->page()->
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
