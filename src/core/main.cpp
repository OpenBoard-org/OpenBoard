#include <QtGui>
#include <QTextCodec>

#if (defined(Q_WS_WIN) && defined(_DEBUG))
//#include <vld.h>
#endif

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "UBApplication.h"
#include "UBSettings.h"

#ifdef Q_WS_MAC
#include "google_breakpad/client/mac/handler/exception_handler.h"
#elif defined(Q_WS_WIN)
#include "google_breakpad/client/windows/handler/exception_handler.h"
#else
// TODO UB 4.x - Linux
#endif

#ifdef Q_WS_MAC
bool MacCallback(const char *dump_dir, const char *file_name,
		void *context, bool success) {

	Q_UNUSED(context);
	Q_UNUSED(success);
	std::string path(dump_dir);
	path.append(file_name);
	path.append(".dmp");
	qDebug() << "minidump has been created " << QString::fromStdString(path);
	// Currently we let OS alert the user about the crash. We will return true when
	// message to the user will be implemented. But dump is available in {data directory}/log
	return false;
}
#elif defined(Q_WS_WIN)
bool WinCallBack(const wchar_t* dump_path,
		const wchar_t* minidump_id,
		void* context,
		EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion,
		bool succeeded) {
	Q_UNUSED(context);
	Q_UNUSED(exinfo);
	Q_UNUSED(assertion);
	Q_UNUSED(succeeded);
	std::wstring path(dump_path);
	path.append(L"/");
	path.append(minidump_id);
	path.append(L".dmp");
	qDebug() << "minidump has been created " << QString::fromStdWString(path);
	// Currently we let OS alert the user about the crash. We will return true when
	// message to the user will be implemented. But dump is available in {data directory}/log
	return false;
}
#endif

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
		QString logFileNamePath = UBSettings::uniboardDataDirectory()
				+ "/log/uniboard.log";
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

int main(int argc, char *argv[]) {
	Q_INIT_RESOURCE(sankore);

	qInstallMsgHandler(ub_message_output);

#if defined(Q_WS_X11)
	qDebug() << "Setting GraphicsSystem to raster";
	QApplication::setGraphicsSystem("raster");
#endif

	UBApplication app("Sankore 3.1", argc, argv);

        QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	QStringList args = app.arguments();

	QString dumpPath = UBSettings::uniboardDataDirectory() + "/log";
	QDir logDir(dumpPath);
	if (!logDir.exists())
		logDir.mkdir(dumpPath);

#if defined(Q_WS_MAC) && defined(QT_NO_DEBUG)
	google_breakpad::ExceptionHandler breakpadHandler(dumpPath.toStdString(), 0, MacCallback, 0, true);
#elif defined(Q_WS_WIN)
	google_breakpad::ExceptionHandler breakpadHandler(dumpPath.toStdWString(), 0, WinCallBack, 0, google_breakpad::ExceptionHandler::HANDLER_ALL);
#endif

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

	QObject::connect(&app, SIGNAL(messageReceived(const QString&)), &app,
			SLOT(handleOpenMessage(const QString&)));

	int result = app.exec(fileToOpen);

	qDebug() << "application is quitting";

	return result;

}
