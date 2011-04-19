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

class QtSingletonPrivate
{
public:
    QString id;
};

/*!
    \class QtSingleApplication qtsingleapplication.h
    \brief The QtSingleApplication class provides an API to detect and
    communicate with running instances of an application.

    This class allows you to create applications that cannot have
    multiple instances running on the same machine for the same user.

    To use the QtSingleApplication class you must provide an ID string
    that it unique on the system you run the application on. Typical
    IDs are the name of the application and the application vendor, or
    a string representation of a \link QUuid UUID\endlink.

    The application should create the QtSingleApplication object very
    early in the startup phase, and try to send a message or call
    isRunning() to find out if an instance of this application is
    already running.

    If an instance is already running, this application instance
    should terminate. Otherwise the application should call
    initialize() immediately, and continue with the initialization of
    the application user interface before entering the event loop with
    exec(). The messageReceived() signal will be emitted when the
    application receives messages from another instance of the same
    application.

    If a message is received it might be helpful to the user to raise
    the application so that it becomes visible. To facilitate this,
    QtSingleApplication provides the setActivationWindow() function
    and the activateWindow() slot.

    Here's an example that shows how to convert an existing
    application to us QtSingleApplication. It is very simple and does
    not make use of all QtSingleApplication's functionality (see the
    examples for that).

    \code
    // Original
    int main(int argc, char **argv)
    {
	QApplication app(argc, argv);

	MyMainWidget mmw;

	mmw.show();
	return app.exec();
    }

    // Single instance
    int main(int argc, char **argv)
    {
	QtSingleApplication app("MySingleInstance", argc, argv);

	if (app.sendMessage("Do I exist?"))
	    return 0;

        app.initialize();

	MyMainWidget mmw;

	app.setActivationWindow(&mmw);

	mmw.show();
	return app.exec();
    }
    \endcode

    Once this QtSingleApplication instance is destroyed(for example,
    when the user quits), when the user next attempts to run the
    application this instance will not, of course, be encountered.
*/

/*!
    Creates a QtSingleApplication object with the identifier \a id. \a
    argc, \a argv and \a type are passed on to the QAppliation
    constructor.

    There can only be one QtSingleApplication object(and since there
    can only be one QApplication object you do not need to create
    another QApplication object yourself).

    \warning On X11 type can not be QApplication::Tty.

*/
QtSingleApplication::QtSingleApplication(const QString &id, int &argc, char **argv, bool useGui)
    : QApplication(argc, argv, useGui)
{
    d = new QtSingletonPrivate;
    d->id = id;
    actWin = 0;

    sysInit();
}

#ifdef Q_WS_X11

/*!
    Creates a QtSingleApplication object, given an already open display
    \a dpy. Uses the identifier \a id. \a argc and \a argv are
    passed on to the QAppliation constructor. If \a visual and \a colormap
    are non-zero, the application will use those as the default Visual and
    Colormap contexts.

    There can only be one QtSingleApplication object(and since there
    can only be one QApplication object you do not need to create
    another QApplication object yourself).

    \warning Qt only supports TrueColor visuals at depths higher than 8
    bits-per-pixel.

    This is available only on X11.
*/
/*
QtSingleApplication::QtSingleApplication(Display* dpy, const QString &id, int argc, char **argv,
                                         Qt::HANDLE visual, Qt::HANDLE colormap)
    : QApplication(dpy, argc, argv, visual, colormap)
{
    d = new QtSingletonPrivate;
    d->id = id;
    actWin = 0;

    sysInit();
}
*/

#endif // Q_WS_X11


/*!
    Destroys the object, freeing all allocated resources.

    If the same application is started again it will not find this
    instance.
*/
QtSingleApplication::~QtSingleApplication()
{
    sysCleanup();

    delete d;
}

/*!
    Returns the identifier of this singleton object.
*/
QString QtSingleApplication::id() const
{
    return d->id;
}


/*!
  Sets the activation window of this application to \a aw. The
  activation window is the widget that will be activated by
  activateWindow(). This is typically the application's main window.

  \sa activateWindow(), messageReceived()
*/
void QtSingleApplication::setActivationWindow(QWidget* aw)
{
    actWin = aw;
}


/*!
    Returns the applications activation window if one has been set by
    calling setActivationWindow(), otherwise returns 0.

    \sa setActivationWindow()
*/
QWidget* QtSingleApplication::activationWindow() const
{
    return actWin;
}


/*!
  De-minimizes, raises, and activates this application's activation window.
  This function does nothing if no activation window has been set.

  This is a convenience function to show the user that this
  application instance has been activated when he has tried to start
  another instance.

  This function should typically be called in response to the
  messageReceived() signal. initialize() will connect that
  signal to this slot by default.

  \sa setActivationWindow(), messageReceived(), initialize()
*/

void QtSingleApplication::activateWindow()
{
    if (actWin) {
        actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
        actWin->raise();
        actWin->activateWindow();
    }
}


/*! \fn bool QtSingleApplication::isRunning() const

    Returns true if another instance of this application has called
    initialize(); otherwise returns false.

    This function does not find instances of this application that are
    being run by a different user.

    \sa initialize()
*/

/*!
    \fn void QtSingleApplication::initialize(bool activate)

    Once this function has been called, this application instance
    becomes "visible" to other instances. This means that if another
    instance is started(by the same user), and calls isRunning(), the
    isRunning() function will return true to that other instance,
    which should then quit, leaving this instance to continue.

    If \a activate is true (the default) the messageReceived() signal
    will be connected to the activateWindow() slot.
*/

/*!
    \fn bool QtSingleApplication::sendMessage(const QString& message, int timeout)

    Tries to send the text \a message to the currently running
    instance. The QtSingleApplication object in the running instance
    will emit the messageReceived() signal when it receives the
    message.

    This function returns true if the message has been sent to, and
    processed by, the current instance. If there is no instance
    currently running, or if the running instance fails to process the
    message within \a timeout milliseconds this function return false.

    Note that on X11 systems the \a timeout parameter is ignored.

    \sa messageReceived()
*/

/*!
    \fn void QtSingleApplication::messageReceived(const QString& message)

    This signal is emitted when the current instance receives a \a
    message from another instance of this application.

    This signal is typically connected to the activateWindow()
    slot.

    \sa activateWindow(), initialize()
*/

