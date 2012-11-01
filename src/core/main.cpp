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


#include <QtGui>
#include <QTextCodec>

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "UBApplication.h"
#include "UBSettings.h"

/* Uncomment this for memory leaks detection */
/*
#if defined(WIN32) && defined(_DEBUG)
     #define _CRTDBG_MAP_ALLOC
     #include <stdlib.h>
     #include <crtdbg.h>
     #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
     #define new DEBUG_NEW
#endif
*/

void ub_message_output(QtMsgType type, const char *msg) {
    // We must temporarily remove the handler to avoid the infinite recursion of
    // ub_message_output -> qt_message_output -> ub_message_output -> qt_message_output ...
    QtMsgHandler previousHandler = qInstallMsgHandler(0);

#if defined(QT_NO_DEBUG)
    // Suppress qDebug output in release builds
    if (type != QtDebugMsg)
    {
        qt_message_output(type, msg);
    }

#else
    // Default output in debug builds
    qt_message_output(type, msg);
#endif

    if (UBApplication::app() && UBApplication::app()->isVerbose()) {
        QString logFileNamePath = UBSettings::userDataDirectory() + "/log/uniboard.log";
        QFile logFile(logFileNamePath);

        if (logFile.exists() && logFile.size() > 10000000)
            logFile.remove();

        if (logFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&logFile);
            out << QDateTime::currentDateTime().toString(Qt::ISODate)
                << "      " << msg << "\n";
            logFile.close();
        }
    }

    qInstallMsgHandler(previousHandler);
}

int main(int argc, char *argv[]) 
{

    // Uncomment next section to have memory leaks information
    // tracing in VC++ debug mode under Windows
/*
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
*/

    Q_INIT_RESOURCE(sankore);

    qInstallMsgHandler(ub_message_output);

#if defined(Q_WS_X11)
    qDebug() << "Setting GraphicsSystem to raster";
    QApplication::setGraphicsSystem("raster");
#endif

    UBApplication app("Sankore", argc, argv);

    //BUGFIX:
    //when importing a sankore file that contains a non standard character
    //the codecForLocale or the codecForCString is used to convert the file path
    //into a const char*. This is why in french windows setup the codec name shouldn't be
    //set to UTF-8. For example, setting UTF-8, will convert "Haïti" into "HaÂ-ti.

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QStringList args = app.arguments();

    QString dumpPath = UBSettings::userDataDirectory() + "/log";
    QDir logDir(dumpPath);
    if (!logDir.exists())
        logDir.mkdir(dumpPath);

    QString fileToOpen;

    if (args.size() > 1) {
        // On Windows/Linux first argument is the file that has been double clicked.
        // On Mac OSX we use FileOpen QEvent to manage opening file in current instance. So we will never
        // have file to open as a parameter on OSX.

        QFile f(args[1]);

        if (f.exists()) {
            fileToOpen += args[1];

            if (app.sendMessage(UBSettings::appPingMessage, 20000)) {
                app.sendMessage(fileToOpen, 1000000);
                return 0;
            }
        }
    }

    app.initialize(false);

    QObject::connect(&app, SIGNAL(messageReceived(const QString&)), &app, SLOT(handleOpenMessage(const QString&)));

    qDebug() << "file name argument" << fileToOpen;
    int result = app.exec(fileToOpen);

    app.cleanup();

    qDebug() << "application is quitting";



    return result;

}
