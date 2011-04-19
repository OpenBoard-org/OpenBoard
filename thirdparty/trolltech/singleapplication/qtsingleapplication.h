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

#ifndef QTSINGLEAPPLICATION_H
#define QTSINGLEAPPLICATION_H

#include <QtGui/QApplication>
#ifdef Q_WS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

class QtSingletonPrivate;
class QtSingletonSysPrivate;
class QWidget;

#if defined(Q_WS_WIN)
#  if !defined(QT_QTSINGLEAPPLICATION_EXPORT) && !defined(QT_QTSINGLEAPPLICATION_IMPORT)
#    define QT_QTSINGLEAPPLICATION_EXPORT
#  elif defined(QT_QTSINGLEAPPLICATION_IMPORT)
#    if defined(QT_QTSINGLEAPPLICATION_EXPORT)
#      undef QT_QTSINGLEAPPLICATION_EXPORT
#    endif
#    define QT_QTSINGLEAPPLICATION_EXPORT __declspec(dllimport)
#  elif defined(QT_QTSINGLEAPPLICATION_EXPORT)
#    undef QT_QTSINGLEAPPLICATION_EXPORT
#    define QT_QTSINGLEAPPLICATION_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTSINGLEAPPLICATION_EXPORT
#endif

class QT_QTSINGLEAPPLICATION_EXPORT QtSingleApplication : public QApplication
{
    Q_OBJECT
public:
    QtSingleApplication(const QString &id, int &argc, char **argv, bool useGui = true);
//#ifdef Q_WS_X11
    //QtSingleApplication(Display* dpy, const QString &id, int argc, char **argv, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);
//#endif
    ~QtSingleApplication();

    bool isRunning() const;
    QString id() const;

    void initialize(bool activate = true);
    void setActivationWindow(QWidget* aw);
    QWidget* activationWindow() const;

protected:
#if defined(Q_WS_X11)
    bool x11EventFilter(XEvent *msg);
#endif

public Q_SLOTS:
    bool sendMessage(const QString &message, int timeout = 5000);
    virtual void activateWindow();

Q_SIGNALS:
    void messageReceived(const QString& message);

private:
    friend class QtSingletonSysPrivate;
#ifdef Q_WS_MAC
    friend CFDataRef MyCallBack(CFMessagePortRef, SInt32, CFDataRef, void *);
#endif
    QtSingletonPrivate *d;
    QtSingletonSysPrivate *sysd;
    QWidget* actWin;

    void sysInit();
    void sysCleanup();
};

#endif //QTSINGLEAPPLICATION_H
