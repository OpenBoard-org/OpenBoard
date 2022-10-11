/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include <QtGui>

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

void ub_message_output(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    // We must temporarily remove the handler to avoid the infinite recursion of
    // ub_message_output -> qt_message_output -> ub_message_output -> qt_message_output ...
    QtMessageHandler previousHandler = qInstallMessageHandler(0);

#if defined(QT_NO_DEBUG)
    // Suppress qDebug output in release builds
    if (type != QtDebugMsg)
    {
        qt_message_output(type, context, msg);
    }

#else
    // Default output in debug builds
    qt_message_output(type, context, msg);
#endif

    if (UBApplication::app() && UBApplication::app()->isVerbose()) {
        QString logFileNamePath = UBSettings::userDataDirectory() + "/log/"+ qApp->applicationName() + ".log";
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

    qInstallMessageHandler(previousHandler);
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

    // QT_NO_GLIB=1 is set by default on Linux, and prevents media playback
    if (qEnvironmentVariableIsSet("QT_NO_GLIB"))
        qunsetenv("QT_NO_GLIB");

    Q_INIT_RESOURCE(OpenBoard);

    qInstallMessageHandler(ub_message_output);

    UBApplication app("OpenBoard", argc, argv);

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

            if (app.sendMessage(UBSettings::appPingMessage.toUtf8(), 20000)) {
                app.sendMessage(fileToOpen.toUtf8(), 1000000);
                return 0;
            }
        }
    }

    qDebug() << "file name argument" << fileToOpen;
    int result = app.exec(fileToOpen);

    app.cleanup();

    qDebug() << "application is quitting";



    return result;

}
