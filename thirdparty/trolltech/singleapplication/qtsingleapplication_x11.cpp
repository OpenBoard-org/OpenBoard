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

#include "qtsingleapplication.h"
#include <QtGui/QWidget>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtCore/QDebug>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <QtGui/QX11Info>


class QtSingletonListener : public QWidget
{
public:
    QtSingletonListener(Atom atom, QWidget* par = 0)
        : QWidget(par)
    {
        QByteArray yes = "YES";
        XChangeProperty(QX11Info::display(),
                        winId(),
                        atom, atom, 8,
                        PropModeReplace,
                        reinterpret_cast<unsigned char *>(yes.data()), 3);
    }
};

class QtSingletonSysPrivate
{
public:
    Atom selAtom;
    Atom typAtom;
    Atom listenAtom;
    QWidget *listener;
    QByteArray xpcs;
    bool owner;

    QString login() const;
    QByteArray toXPCS(const QString &str) const; // X Portable Character Set

    void sendMessageTo(Window wid, Atom sel, Atom typ,
                       const QString &message) const;
    bool getProperty(char** data, unsigned long* nitems,
                     Window win, Atom sel, Atom typ) const;
    void findWindows(QList<Window> &windows);
};

QByteArray QtSingletonSysPrivate::toXPCS(const QString &str) const
{
    QString strtmp(str);
    if (strtmp.isEmpty() || strtmp[0] != '_')
        strtmp.prepend('_');
    QByteArray tmp = strtmp.toLocal8Bit();
    for (int i = 0; i < tmp.size(); ++i) {
	if (!xpcs.contains(tmp[i]))
	    tmp[i] = '_';
    }
    return tmp;
}

bool QtSingletonSysPrivate::getProperty(char** data, unsigned long* nitems,
                                        Window win, Atom sel, Atom typ) const
{
    if (data == 0 || nitems == 0)
        return false;

    Atom actualType;
    int  actualFormat, ret;
    long length = 1024;
    unsigned long bafter;
    for (;;) {
        *data = 0;
        *nitems = 0;

        ret = XGetWindowProperty(QX11Info::display(),
                                 win,
                                 sel,
                                 0L, length,
                                 False,
                                 typ,
                                 &actualType,
                                 &actualFormat,
                                 nitems,
                                 &bafter,
                                 reinterpret_cast<unsigned char **>(data));
        if (ret == Success && actualType == typ && *nitems > 0) {
            if (bafter == 0)
                return TRUE;
            else {
                if (actualFormat == 8)
                    length += (bafter / 4) + 1;
                else if (actualFormat == 16)
                    length += (bafter / 2) + 1;
                else if (actualFormat == 32)
                    length += bafter;
            }
        } else {
            if (*nitems > 0 && *data != 0)
                XFree(*data);
            return FALSE;
        }
        if (*nitems > 0 && *data != 0)
            XFree(*data);
    }
    return FALSE;
}

void QtSingletonSysPrivate::findWindows(QList<Window> &windows)
{
    Window root, parent;
    Window* children = 0;
    unsigned int nchildren;
    QDesktopWidget* desktop = qApp->desktop();
    char* data;
    unsigned long nitems;

    for (int i = 0; i < desktop->numScreens(); ++i) {
        if (XQueryTree(QX11Info::display(),
                       desktop->screen(i)->winId(),
                       &root, &parent,
                       &children, &nchildren) == 0)
            continue;
        if (nchildren > 0 && children != 0) {
            for (unsigned int i = 0; i < nchildren; ++i) {
                if (getProperty(&data, &nitems, children[i],
                                listenAtom, listenAtom)) {
                    if (data != 0)
                        XFree(data);
                    windows.append(children[i]);
                }
            }
            XFree(children);
        }
    }
}

QString QtSingletonSysPrivate::login() const
{
    struct passwd *pwd = getpwuid(getuid());
    if (pwd) {
	return QString(pwd->pw_name);
    }
    return QString();
}

void QtSingletonSysPrivate::sendMessageTo(Window wid, Atom sel, Atom typ,
                                          const QString &message) const
{
    if (typ != None) {
        QByteArray umsg = message.toUtf8();
        XChangeProperty(QX11Info::display(),
                        wid,
                        sel, typ, 8,
                        PropModeReplace,
                        reinterpret_cast<unsigned char *>(umsg.data()),
                        umsg.size());
    }
}

void QtSingleApplication::sysInit()
{
    sysd = new QtSingletonSysPrivate;
    sysd->selAtom = None;
    sysd->typAtom = None;
    sysd->listenAtom = None;
    sysd->listener = 0;
    sysd->xpcs.resize(95);
    sysd->xpcs = QByteArray("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
    sysd->owner = false;
}

void QtSingleApplication::sysCleanup()
{
    if (sysd) {
        // make the old owner the current owner
        if (sysd->owner) {
            Window newowner = None;

            // We don't want any of the windows to be destroyed
            // while we're looping over them. So we grab the server.
            XGrabServer(QX11Info::display());

            QList<Window> windows;
            sysd->findWindows(windows);
            QList<Window>::ConstIterator it = windows.begin();
            while (it != windows.end()) {
                if (*it != sysd->listener->winId())
                    newowner = *it;
                ++it;
            }
            if (newowner != None)
                sysd->sendMessageTo(newowner,
                                    sysd->selAtom, sysd->typAtom,
                                    "a");

            XUngrabServer(QX11Info::display());

            // Make sure that the server releases the grab as soon
            // as possible.
            XFlush(QX11Info::display());
        }
        delete sysd->listener;
        delete sysd;
    }
}

void QtSingleApplication::initialize(bool activate)
{
    if (sysd->selAtom != None)
	return;

    QByteArray login = sysd->toXPCS(id()+sysd->login());
    sysd->selAtom = XInternAtom(QX11Info::display(),
				login,
				False);
    sysd->typAtom = XInternAtom(QX11Info::display(),
				"_QTSINGLEAPPLICATION",
				False);
    sysd->listenAtom = XInternAtom(QX11Info::display(),
                                   login+"_LISTENER",
                                   False);

    if (sysd->selAtom != None) {
	sysd->listener = new QtSingletonListener(sysd->listenAtom);
        Window lid = sysd->listener->winId();
	XSetSelectionOwner(QX11Info::display(),
			   sysd->selAtom,
			   lid,
			   CurrentTime);
        if (XGetSelectionOwner(QX11Info::display(), sysd->selAtom) == lid)
            sysd->owner = true;
    }

    if (activate)
	connect(this, SIGNAL(messageReceived(const QString&)),
		this, SLOT(activateWindow()));
}

bool QtSingleApplication::isRunning() const
{
    QByteArray login = sysd->toXPCS(id()+sysd->login());
    Atom tmp = XInternAtom(QX11Info::display(),
			   login,
			   True);

    if (tmp != None) {
	WId wid = XGetSelectionOwner(QX11Info::display(),
				     tmp);
	return (wid != None);
    }
    return FALSE;
}

bool QtSingleApplication::sendMessage(const QString &message, int)
{
    QByteArray login = sysd->toXPCS(id()+sysd->login());
    Atom sel = XInternAtom(QX11Info::display(),
                           login,
                           True);
    if (sel == None)
        return FALSE;
    WId wid = XGetSelectionOwner(QX11Info::display(), sel);
    if (wid != None) {
	Atom typ = XInternAtom(QX11Info::display(),
				"_QTSINGLEAPPLICATION",
				True);
        if (typ != None) {
            sysd->sendMessageTo(wid, sel, typ, message);
            return TRUE;
        }
    }
    return FALSE;
}

/*!
    \internal
*/

bool QtSingleApplication::x11EventFilter(XEvent *msg)
{
    if (sysd->listener != 0) {
        if (msg->type == PropertyNotify) {
            XPropertyEvent pev = msg->xproperty;
            if (pev.window == sysd->listener->winId()) {
                if (!sysd->owner) {
                    // We aren't the current owner, but some program changed our
                    // property. This will happen if more than one
                    // QtSingleApplication instance of the same type is running
                    // and one of them closes and we were the previous owner
                    // of the selection. Try to make ourselves the new owner
                    Window lid = sysd->listener->winId();
                    XSetSelectionOwner(QX11Info::display(),
                                       sysd->selAtom,
                                       lid,
                                       CurrentTime);
                    if (XGetSelectionOwner(QX11Info::display(), sysd->selAtom) == lid)
                        sysd->owner = true;
                    return TRUE;
                }
                char* data = 0;
                unsigned long nitems = 0;
                if (sysd->getProperty(&data, &nitems, pev.window,
                                      sysd->selAtom, sysd->typAtom)) {
                    QString str = QString::fromUtf8(reinterpret_cast<char *>(data), nitems);
                    if (data)
                        XFree(data);
                    emit messageReceived(str);

                    return TRUE;
                }
                if (data)
                    XFree(data);
            }
        } else if (msg->type == SelectionClear) {
            if (msg->xselectionclear.selection == sysd->selAtom)
                sysd->owner = false;
        }
    }
    return QApplication::x11EventFilter(msg);
}
