/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info.com)
**
** This file is part of a Qt Solutions component.
**
** Commercial Usage
** Licensees holding valid Qt Solutions licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales.com.
**
****************************************************************************/

#include <QtCore/QString>
#include <Carbon/Carbon.h>

#include "qtsingleapplication.h"

CFStringRef qstring2cfstring(const QString &str)
{
    return CFStringCreateWithCharacters(0, (UniChar *)str.unicode(), str.length());
}

QString cfstring2qstring(CFStringRef str)
{
    if(!str)
	return QString();

    CFIndex length = CFStringGetLength(str);
    if(const UniChar *chars = CFStringGetCharactersPtr(str))
	return QString((QChar *)chars, length);
    UniChar *buffer = (UniChar*)malloc(length * sizeof(UniChar));
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer);
    QString ret((QChar *)buffer, length);
    free(buffer);
    return ret;
}

class QtSingletonSysPrivate
{
public:
    inline QtSingletonSysPrivate() { port = 0; }
    CFMessagePortRef port;
};

class QtSingletonPrivate
{
};

bool QtSingleApplication::isRunning() const
{
    CFStringRef cfstr = qstring2cfstring(id());
    CFMessagePortRef myport = CFMessagePortCreateRemote(kCFAllocatorDefault, cfstr);
    CFRelease(cfstr);
    return (myport != 0);
}

void QtSingleApplication::sysInit()
{
    sysd = new QtSingletonSysPrivate;
}

void QtSingleApplication::sysCleanup()
{
    delete sysd;
}

CFDataRef MyCallBack(CFMessagePortRef, SInt32, CFDataRef data, void *info)
{
    CFIndex index = CFDataGetLength(data);
    const UInt8 *p = CFDataGetBytePtr(data);
    QByteArray ba(index, 0);
    for (int i = 0; i < index; ++i)
        ba[i] = p[i];
    QString message(ba);
    emit static_cast<QtSingleApplication *>(info)->messageReceived(message);
    return 0;
}

void QtSingleApplication::initialize(bool activate)
{
    CFStringRef cfstr = qstring2cfstring(id());
    CFMessagePortContext context;
    context.version = 0;
    context.info = this;
    context.retain = 0;
    context.release = 0;
    context.copyDescription = 0;
    sysd->port = CFMessagePortCreateLocal(kCFAllocatorDefault, cfstr, MyCallBack, &context, 0);
    CFRunLoopRef runloop = CFRunLoopGetCurrent();
    if (sysd->port && runloop) {
        CFRunLoopSourceRef source = CFMessagePortCreateRunLoopSource(0, sysd->port, 0);
        if (source)
            CFRunLoopAddSource(runloop, source, kCFRunLoopCommonModes);
        CFRelease(source);
    }
    CFRelease(cfstr);
    if (activate) {
	connect(this, SIGNAL(messageReceived(const QString&)),
		this, SLOT(activateWindow()));
    }
}

bool QtSingleApplication::sendMessage(const QString &message, int timeout)
{
    CFStringRef cfstr = qstring2cfstring(id());
    CFMessagePortRef myport = CFMessagePortCreateRemote(kCFAllocatorDefault, cfstr);
    CFRelease(cfstr);
    if (!myport)
        return false;
    static SInt32 msgid = 0;
    const QByteArray latin1Message = message.toLatin1();
    CFDataRef data = CFDataCreate(0, (UInt8*)latin1Message.constData(), latin1Message.length());
    CFDataRef reply = 0;
    SInt32 result = CFMessagePortSendRequest(myport, ++msgid, data, timeout / 1000,
                                             timeout / 1000, 0, &reply);
    CFRelease(data);
    if (reply)
        CFRelease(reply);
    return result == kCFMessagePortSuccess;

}
